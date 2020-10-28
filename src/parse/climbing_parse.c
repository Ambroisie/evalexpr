#include "parse.h"

#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

#include "ast/ast.h"

#define UNREACHABLE() __builtin_unreachable()
#define ARR_SIZE(Arr) (sizeof(Arr) / sizeof(*Arr))

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

static enum binop_kind char_to_binop(char c)
{
    static const struct { const char *op; const enum binop_kind kind; } ops[] = {
#define BINOP(Op, Kind, _, __, ___) { #Op, Kind },
#include "operators.inc"
    };

    for (size_t i = 0; i < ARR_SIZE(ops); ++i)
        if (c == ops[i].op[0])
            return ops[i].kind;

    UNREACHABLE();
}

static enum unop_kind char_to_unop(char c)
{
    static const struct { const char *op; const enum unop_kind kind; } ops[] = {
#define PREOP(Op, Kind, _, __, ___) { #Op, Kind },
#define POSTOP(Op, Kind, _, __, ___) { #Op, Kind },
#include "operators.inc"
    };

    for (size_t i = 0; i < ARR_SIZE(ops); ++i)
        if (c == ops[i].op[0])
            return ops[i].kind;

    UNREACHABLE();
}

static bool is_binop(char c)
{
    static const char *ops[] = {
#define BINOP(Op, _, __, ___, ____) #Op,
#include "operators.inc"
    };
    for (size_t i = 0; i < ARR_SIZE(ops); ++i)
        if (c == ops[i][0])
            return true;

    return false;
}

static bool prec_between(const char **input, int min, int max)
{
    static const struct { const char *op; const int prio; } ops[] = {
#define BINOP(Op, _, Priority, __, ___) { #Op, Priority },
#define PREOP(Op, _, Priority, __, ___) { #Op, Priority },
#define POSTOP(Op, _, Priority, __, ___) { #Op, Priority },
#include "operators.inc"
    };

    skip_whitespace(input);

    for (size_t i = 0; i < ARR_SIZE(ops); ++i)
        if (*input[0] == ops[i].op[0])
            return min <= ops[i].prio && ops[i].prio <= max;

    return false;
}

static const struct {
    const char *op;
    const int kind; // Because we store different kinds of enum...
    const int prio;
    const enum { ASSOC_LEFT, ASSOC_RIGHT, ASSOC_NONE } assoc;
    const enum { OP_INFIX, OP_PREFIX, OP_POSTFIX } fix;
} ops[] = {
# define BINOP(Op, Kind, Prio, Assoc, Fix) { #Op, Kind, Prio, Assoc, Fix },
#include "operators.inc"
};

static int right_prec(char c)
{
    for (size_t i = 0; i < ARR_SIZE(ops); ++i)
        if (c == ops[i].op[0])
        {
            if (ops[i].assoc == ASSOC_RIGHT)
                return ops[i].prio;
            return ops[i].prio + 1;
        }

    return INT_MIN;
}

static int next_prec(char c)
{
    for (size_t i = 0; i < ARR_SIZE(ops); ++i)
        if (c == ops[i].op[0])
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

static struct ast_node *climbing_parse_internal(const char **input, int prec)
{
    prec = prec;
    struct ast_node *ast = parse_operand(input);

    int r = INT_MAX;
    while (/* infix_or_postfix(input) && */ prec_between(input, prec, r) && ast)
    {
        const char c = *input[0];
        eat_char(input);
        if (is_binop(c))
        {
            enum binop_kind op = char_to_binop(c);
            struct ast_node *rhs = climbing_parse_internal(input, right_prec(c));
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
            struct ast_node *tree = make_unop(char_to_unop(c), ast);
            if (!tree)
                destroy_ast(ast); // Error case
            ast = tree;
        }
        r = next_prec(c);
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
    skip_whitespace(input); // Whitespace is not significant
    struct ast_node *ast = NULL;

    int val = 0;
    if (*input[0] == '+' || *input[0] == '-')
    {
        enum unop_kind op = char_to_unop(*input[0]);
        // Remove the parenthesis
        eat_char(input);

        ast = climbing_parse_internal(input, 2); // FIXME: should be from table

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
