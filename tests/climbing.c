#include <criterion/criterion.h>

#include "ast/ast.h"
#include "eval/eval.h"
#include "parse/parse.h"

static void do_success(const char *input, int expected)
{
    struct ast_node *ast = climbing_parse(input);

    cr_assert_not_null(ast);
    cr_expect_eq(eval_ast(ast), expected);

    destroy_ast(ast);
}

static void do_failure(const char *input)
{
    struct ast_node *ast = climbing_parse(input);

    cr_expect_null(ast);

    destroy_ast(ast); // Do not leak if it exists
}

TestSuite(climbing);

Test(climbing, empty)
{
    do_failure("");
}

Test(climbing, trailing_operator)
{
    do_failure("1 +");
}

Test(climbing, trailing_expression)
{
    do_failure("1 1");
}

Test(climbing, double_operator)
{
    do_failure("1 * * 1");
}

Test(climbing, one)
{
    do_success("1", 1);
}

Test(climbing, the_answer)
{
    do_success("42", 42);
}

Test(climbing, int_max)
{
    do_success("2147483647", 2147483647);
}

Test(climbing, whitespace)
{
    do_success("   1   ", 1);
}

Test(climbing, more_whitespace)
{
    do_success("   1   + 2     ", 3);
}

Test(climbing, one_plus_one)
{
    do_success("1+1", 2);
}

Test(climbing, one_minus_one)
{
    do_success("1-1", 0);
}

Test(climbing, additions)
{
    do_success("1+1+1+1+1", 5);
}

Test(climbing, substractions)
{
    do_success("1-1-1-1-1", -3);
}

Test(climbing, multiplication)
{
    do_success("2 * 3", 6);
}

Test(climbing, multiplications)
{
    do_success("1 * 2 * 3 * 4", 24);
}

Test(climbing, division)
{
    do_success("12 / 3", 4);
}

Test(climbing, divisions)
{
    do_success("24 / 4 / 3 / 2", 1);
}

Test(climbing, simple_priority)
{
    do_success("1 + 2 * 3", 7);
}

Test(climbing, more_priority)
{
    do_success("1 + 6 / 3 + 4 * 6 + 14 / 7", 29);
}

Test(climbing, fail_parenthesis)
{
    do_failure("(1 + 2))");
}

Test(climbing, simple_parenthesis)
{
    do_success("(1 + 2) * 3", 9);
}

Test(climbing, more_parentheses)
{
    do_success("(1 + 2) * (3 - 4)", -3);
}

Test(climbing, unary_minus)
{
    do_success("-1", -1);
}

Test(climbing, unary_plus)
{
    do_success("+1", 1);
}

Test(climbing, unary_torture)
{
    do_success("--+++--+-+-+-1", -1);
}

Test(climbing, factorial)
{
    do_success("3!", 6);
}

Test(climbing, fail_factorial)
{
    do_failure("3!!");
}

Test(climbing, power)
{
    do_success("4^3", 64);
}

Test(climbing, powers)
{
    do_success("4^3^2", 262144);
}

Test(climbing, fact_and_power)
{
    do_success("2^3!", 64);
}

Test(climbing, altogether)
{
    do_success("  -   3 ^ 2 + - 4 * 8 / 2 + + 3! -- 2 + ((-1) + 1) * 2 ", -17);
}
