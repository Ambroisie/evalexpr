#include "parse.h"

#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "ast/ast.h"

#define UNREACHABLE() __builtin_unreachable()
#define ARR_SIZE(Arr) (sizeof(Arr) / sizeof(*Arr))
#define OP_STRING(...) (const char[]){__VA_ARGS__}
#define OP_SIZE(...) (sizeof(OP_STRING(__VA_ARGS__)) - 1)

static const struct {
    const char *op;
    const size_t op_len;
    const enum op_kind kind;
    const int prio;
    const enum { ASSOC_LEFT, ASSOC_RIGHT, ASSOC_NONE } assoc;
    const enum { OP_INFIX, OP_PREFIX, OP_POSTFIX } fix;
} ops[] = {
# define OP(Kind, Prio, Assoc, Fix, /* Operator string */ ...) \
    { OP_STRING(__VA_ARGS__), OP_SIZE(__VA_ARGS__), Kind, Prio, Assoc, Fix, },
#include "operators.inc"
};

static struct ast_node *climbing_parse_internal(const char **input, int prec);
static struct ast_node *parse_operand(const char **input);

static void eat_char(const char **input)
{
    *input += 1; // Skip this character
}

static void skip_whitespace(const char **input)
{
    while (*input[0] && isspace(*input[0]))
        eat_char(input);
}

static size_t parse_binop(size_t *op_ind, const char **input)
{
    skip_whitespace(input);

    size_t best_len = 0;
    for (size_t i = 0; i < ARR_SIZE(ops); ++i)
    {
        if (ops[i].fix != OP_INFIX) // Only look at infix operators
            continue;
        if (ops[i].op_len <= best_len) // Only look at longer operators
            continue;
        if (strncmp(*input, ops[i].op, ops[i].op_len) == 0)
        {
            best_len = ops[i].op_len;
            *op_ind = i;
        }
    }

    // Return how many characters should be skipped
    return best_len;
}

static size_t parse_prefix(size_t *op_ind, const char **input)
{
    skip_whitespace(input);

    size_t best_len = 0;
    for (size_t i = 0; i < ARR_SIZE(ops); ++i)
    {
        if (ops[i].fix != OP_PREFIX) // Only look at prefix operators
            continue;
        if (ops[i].op_len <= best_len) // Only look at longer operators
            continue;
        if (strncmp(*input, ops[i].op, ops[i].op_len) == 0)
        {
            best_len = ops[i].op_len;
            *op_ind = i;
        }
    }

    // Return how many characters should be skipped
    return best_len;
}

static size_t parse_postfix(size_t *op_ind, const char **input)
{
    skip_whitespace(input);

    size_t best_len = 0;
    for (size_t i = 0; i < ARR_SIZE(ops); ++i)
    {
        if (ops[i].fix != OP_POSTFIX) // Only look at postfix operators
            continue;
        if (ops[i].op_len <= best_len) // Only look at longer operators
            continue;
        if (strncmp(*input, ops[i].op, ops[i].op_len) == 0)
        {
            best_len = ops[i].op_len;
            *op_ind = i;
        }
    }

    // Return how many characters should be skipped
    return best_len;
}

static int right_prec(size_t op_ind)
{
    if (op_ind >= ARR_SIZE(ops))
        return INT_MIN; // Defensive programming

    if (ops[op_ind].assoc == ASSOC_RIGHT)
        return ops[op_ind].prio;
    return ops[op_ind].prio + 1;
}

static int next_prec(size_t op_ind)
{
    if (op_ind >= ARR_SIZE(ops))
        return INT_MIN; // Defensive programming

    if (ops[op_ind].assoc != ASSOC_LEFT)
        return ops[op_ind].prio - 1;
    return ops[op_ind].prio;
}

/*
 * Simple climbing parser, see `operators.inc` for more details.
 *
 * Whitespace is ignored in the input string, only serving to delimit numbers.
 *
 * The input shall consist of a single expression, having a trailing
 * expression in the input results in an error.
 */
struct ast_node *climbing_parse(const char *input)
{
    if (input == NULL)
        return NULL;

    struct ast_node *ast = climbing_parse_internal(&input, 0);

    if (ast == NULL)
        return NULL;

    // Make sure there is no trailing character, except whitespace
    skip_whitespace(&input);
    if (input[0] != '\0')
    {
        destroy_ast(ast);
        return NULL;
    }

    return ast;
}

static size_t update_op(size_t *op_ind, bool *is_binop, const char **input)
{
    skip_whitespace(input); // Unnecessary given that both methods skip it...

    const char *save_input = *input;

    size_t op_bin;
    size_t bin_size = parse_binop(&op_bin, input);

    // Reset the parsing
    *input = save_input;

    size_t op_post;
    size_t post_size = parse_postfix(&op_post, input);

    // Reset the parsing
    *input = save_input;

    if (bin_size > post_size)
    {
        *op_ind = op_bin;
        *is_binop = true;
        return bin_size;
    }
    else if (post_size > bin_size)
    {
        *op_ind = op_post;
        *is_binop = false;
        return post_size;
    }

    // NOTE: assume that there were no matching operators at all, instead
    return 0;
}

static bool prec_between(size_t op_ind, int min, int max)
{
    if (op_ind >= ARR_SIZE(ops))
        return false; // Defensive programming

    return min <= ops[op_ind].prio && ops[op_ind].prio <= max;
}

static struct ast_node *climbing_parse_internal(const char **input, int prec)
{
    prec = prec;
    struct ast_node *ast = parse_operand(input);

    int r = INT_MAX;
    size_t len = 0;
    size_t op_ind; // Used in the next loop
    bool is_binop; // Used in the next loop
    while ((len = update_op(&op_ind, &is_binop, input)) // Initialise the operator
            && prec_between(op_ind, prec, r) // Use newly initialized operator
            && ast)
    {
        *input += len; // Skip the parsed operator
        if (is_binop) // Given to us by `update_op`
        {
            struct ast_node *rhs =
                climbing_parse_internal(input, right_prec(op_ind));
            if (!rhs)
            {
                destroy_ast(ast);
                return NULL;
            }
            struct ast_node *tree = make_binop(ops[op_ind].kind, ast, rhs);

            if (!tree)
                destroy_ast(ast); // Error case
            ast = tree;
        }
        else
        {
            struct ast_node *tree = make_unop(ops[op_ind].kind, ast);
            if (!tree)
                destroy_ast(ast); // Error case
            ast = tree;
        }
        r = next_prec(op_ind);
    }

    return ast;
}

static bool my_atoi(const char **input, int *val)
{
    if (!isdigit(*input[0]))
        return false;

    *val = 0; // Initialize its value
    do
    {
        *val *= 10;
        *val += *input[0] - '0';
        *input += 1;
    } while (isdigit(*input[0]));

    return true;
}

static struct ast_node *parse_operand(const char **input)
{
    struct ast_node *ast = NULL;

    int val = 0;
    size_t skip = 0;
    size_t op_ind;
    if ((skip = parse_prefix(&op_ind, input))) // Removes whitespace as side-effect
    {
        *input += skip; // Skip the parsed operator
        ast = climbing_parse_internal(input, next_prec(op_ind));

        if (!ast)
            return NULL;
        struct ast_node *tree = make_unop(ops[op_ind].kind, ast);
        if (!tree)
            destroy_ast(ast);
        ast = tree;
    }
    else if (my_atoi(input, &val))
        ast = make_num(val);
    else if (*input[0] == '(')
    {
        // Remove the parenthesis
        eat_char(input);
        ast = climbing_parse_internal(input, 0);
        // Check that we have our closing parenthesis
        skip_whitespace(input);
        if (*input[0] != ')')
        {
            destroy_ast(ast);
            return NULL;
        }
        // Remove the parenthesis
        eat_char(input);
        return ast;
    }

    return ast;
}
