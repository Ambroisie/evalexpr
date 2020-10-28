#include "parse.h"

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>

#include "ast/ast.h"

#define UNREACHABLE() __builtin_unreachable()

static struct ast_node *parse_expression(const char **input);
static struct ast_node *parse_term(const char **input);
static struct ast_node *parse_factor(const char **input);
static struct ast_node *parse_power(const char **input);
static struct ast_node *parse_group(const char **input);

static void eat_char(const char **input)
{
    *input += 1; // Skip this character
}

static void skip_whitespace(const char **input)
{
    while (*input[0] && isspace(*input[0]))
        eat_char(input);
}

/*
 * Simple recursive descent using the following grammar, using E as start:
 *
 *      E : T [ ('+'|'-') T ]*
 *      T : F [ ('*'|'/') F ]*
 *      F : [ ('-'|'+') ]* P
 *      P : G [ ('^') F ]*
 *      G : '(' E ')' | CONSTANT [ '!' ]
 *
 * Whitespace is ignored in the input string, only serving to delimit numbers.
 *
 * The input shall consist of a single expression, having a trailing
 * expression in the input results in an error.
 */

struct ast_node *recursive_parse(const char *input)
{
    if (input == NULL)
        return NULL;

    struct ast_node *ast = parse_expression(&input);

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

static enum binop_kind char_to_binop(char c)
{
    switch (c)
    {
    case '+':
        return BINOP_PLUS;
    case '-':
        return BINOP_MINUS;
    case '*':
        return BINOP_TIMES;
    case '/':
        return BINOP_DIVIDES;
    case '^':
        return BINOP_POW;
    }
    UNREACHABLE();
}

static struct ast_node *parse_expression(const char **input)
{
    struct ast_node *lhs = parse_term(input);
    if (lhs == NULL) // Error occured, abort
        return NULL;

    do
    {
        skip_whitespace(input); // Whitespace is not significant

        if (*input[0] == '\0') // End of input, return parsed expression
            return lhs;

        if (*input[0] == '+' || *input[0] == '-')
        {
            const enum binop_kind op = char_to_binop(*input[0]);

            eat_char(input);

            struct ast_node *rhs = parse_term(input);

            if (rhs == NULL) // Error occured
            {
                destroy_ast(lhs);
                return NULL;
            }

            lhs = make_binop(op, lhs, rhs);
        }
        else
            break; // Unexpected character, end of loop
    } while (true);

    return lhs;
}

static struct ast_node *parse_term(const char **input)
{
    struct ast_node *lhs = parse_factor(input);
    if (lhs == NULL) // Error occured, abort
        return NULL;

    do
    {
        skip_whitespace(input); // Whitespace is not significant

        if (*input[0] == '\0') // End of input, return parsed expression
            return lhs;

        if (*input[0] == '*' || *input[0] == '/')
        {
            const enum binop_kind op = char_to_binop(*input[0]);

            eat_char(input);

            struct ast_node *rhs = parse_factor(input);

            if (rhs == NULL) // Error occured
            {
                destroy_ast(lhs);
                return NULL;
            }

            lhs = make_binop(op, lhs, rhs);
        }
        else
            break; // Unexpected character, end of loop
    } while (true);

    return lhs;
}

static enum unop_kind char_to_unop(char c)
{
    switch (c)
    {
    case '+': // Assume non-faulty input, return identity as default
        return UNOP_IDENTITY;
    case '-':
        return UNOP_NEGATE;
    case '!':
        return UNOP_FACT;
    }
    UNREACHABLE();
}

static struct ast_node *parse_factor(const char **input)
{
    skip_whitespace(input); // Whitespace is not significant
    while (*input[0] == '+' || *input[0] == '-')
    {
        const enum unop_kind op = char_to_unop(*input[0]);

        eat_char(input);

        struct ast_node *rhs = parse_factor(input); // Loop by recursion

        if (rhs == NULL)
            return NULL;

        return make_unop(op, rhs);
    }
    return parse_power(input);
}

static struct ast_node *parse_power(const char **input)
{
    struct ast_node *lhs = parse_group(input);
    if (lhs == NULL) // Error occured, abort
        return NULL;

    skip_whitespace(input); // Whitespace is not significant

    if (*input[0] == '\0') // End of input, return parsed expression
        return lhs;

    if (*input[0] == '^')
    {
        const enum binop_kind op = char_to_binop(*input[0]);

        eat_char(input);

        struct ast_node *rhs = parse_factor(input);

        if (rhs == NULL) // Error occured
        {
            destroy_ast(lhs);
            return NULL;
        }

        lhs = make_binop(op, lhs, rhs);
    }

    return lhs;
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

static struct ast_node *parse_group(const char **input)
{
    skip_whitespace(input); // Whitespace is not significant
    struct ast_node *ast = NULL;

    int val = 0;
    if (my_atoi(input, &val))
        ast = make_num(val);
    else if (*input[0] == '(')
    {
        // Remove the parenthesis
        eat_char(input);
        ast = parse_expression(input);
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

    skip_whitespace(input);
    if (*input[0] == '!')
    {
        eat_char(input);
        return make_unop(UNOP_FACT, ast);
    }

    return ast;
}
