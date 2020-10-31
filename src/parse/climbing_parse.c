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

static size_t parse_binop(enum op_kind *op, const char **input)
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
            *op = ops[i].kind;
        }
    }

    // Return how many characters should be skipped
    return best_len;
}

static size_t parse_prefix(enum op_kind *op, const char **input)
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
            *op = ops[i].kind;
        }
    }

    // Return how many characters should be skipped
    return best_len;
}

static size_t parse_postfix(enum op_kind *op, const char **input)
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
            *op = ops[i].kind;
        }
    }

    // Return how many characters should be skipped
    return best_len;
}

static int right_prec(enum op_kind op)
{
    for (size_t i = 0; i < ARR_SIZE(ops); ++i)
        if (op == ops[i].kind)
        {
            if (ops[i].assoc == ASSOC_RIGHT)
                return ops[i].prio;
            return ops[i].prio + 1;
        }

    return INT_MIN;
}

static int next_prec(enum op_kind op)
{
    for (size_t i = 0; i < ARR_SIZE(ops); ++i)
        if (op == ops[i].kind)
        {
            if (ops[i].assoc != ASSOC_LEFT)
                return ops[i].prio - 1;
            return ops[i].prio;
        }

    return INT_MIN;
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

static size_t update_op(enum op_kind *op, bool *is_binop, const char **input)
{
    skip_whitespace(input); // Unnecessary given that both methods skip it...

    const char *save_input = *input;

    enum op_kind op_bin;
    size_t bin_size = parse_binop(&op_bin, input);

    // Reset the parsing
    *input = save_input;

    enum op_kind op_post;
    size_t post_size = parse_postfix(&op_post, input);

    // Reset the parsing
    *input = save_input;

    if (bin_size > post_size)
    {
        *op = op_bin;
        *is_binop = true;
        return bin_size;
    }
    else if (post_size > bin_size)
    {
        *op = op_post;
        *is_binop = false;
        return post_size;
    }

    // NOTE: assume that there were no matching operators at all, instead
    return 0;
}

static bool prec_between(enum op_kind op, int min, int max)
{
    for (size_t i = 0; i < ARR_SIZE(ops); ++i)
        if (op == ops[i].kind)
            return min <= ops[i].prio && ops[i].prio <= max;

    return false;
}

static struct ast_node *climbing_parse_internal(const char **input, int prec)
{
    prec = prec;
    struct ast_node *ast = parse_operand(input);

    int r = INT_MAX;
    size_t len = 0;
    enum op_kind op; // Used in the next loop
    bool is_binop; // Used in the next loop
    while ((len = update_op(&op, &is_binop, input)) // Initialise the operator
            && prec_between(op, prec, r) // Use newly initialized operator
            && ast)
    {
        *input += len; // Skip the parsed operator
        if (is_binop) // Given to us by `update_op`
        {
            struct ast_node *rhs =
                climbing_parse_internal(input, right_prec(op));
            if (!rhs)
            {
                destroy_ast(ast);
                return NULL;
            }
            struct ast_node *tree = make_binop(op, ast, rhs);

            if (!tree)
                destroy_ast(ast); // Error case
            ast = tree;
        }
        else
        {
            struct ast_node *tree = make_unop(op, ast);
            if (!tree)
                destroy_ast(ast); // Error case
            ast = tree;
        }
        r = next_prec(op);
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
    enum op_kind op;
    if ((skip = parse_prefix(&op, input))) // Removes whitespace as side-effect
    {
        *input += skip; // Skip the parsed operator
        ast = climbing_parse_internal(input, next_prec(op));

        if (!ast)
            return NULL;
        struct ast_node *tree = make_unop(op, ast);
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
