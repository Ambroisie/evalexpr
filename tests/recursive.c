#include <criterion/criterion.h>

#include "ast/ast.h"
#include "eval/eval.h"
#include "parse/parse.h"

static void do_success(const char *input, int expected)
{
    struct ast_node *ast = recursive_parse(input);

    cr_assert_not_null(ast);
    cr_expect_eq(eval_ast(ast), expected);

    destroy_ast(ast);
}

static void do_failure(const char *input)
{
    struct ast_node *ast = recursive_parse(input);

    cr_expect_null(ast);

    destroy_ast(ast); // Do not leak if it exists
}

TestSuite(recursive);

Test(recursive, empty)
{
    do_failure("");
}

Test(recursive, trailing_operator)
{
    do_failure("1 +");
}

Test(recursive, trailing_expression)
{
    do_failure("1 1");
}

Test(recursive, double_operator)
{
    do_failure("1 * * 1");
}

Test(recursive, one)
{
    do_success("1", 1);
}

Test(recursive, the_answer)
{
    do_success("42", 42);
}

Test(recursive, int_max)
{
    do_success("2147483647", 2147483647);
}

Test(recursive, whitespace)
{
    do_success("   1   ", 1);
}

Test(recursive, more_whitespace)
{
    do_success("   1   + 2     ", 3);
}

Test(recursive, one_plus_one)
{
    do_success("1+1", 2);
}

Test(recursive, one_minus_one)
{
    do_success("1-1", 0);
}

Test(recursive, additions)
{
    do_success("1+1+1+1+1", 5);
}

Test(recursive, substractions)
{
    do_success("1-1-1-1-1", -3);
}

Test(recursive, multiplication)
{
    do_success("2 * 3", 6);
}

Test(recursive, multiplications)
{
    do_success("1 * 2 * 3 * 4", 24);
}

Test(recursive, division)
{
    do_success("12 / 3", 4);
}

Test(recursive, divisions)
{
    do_success("24 / 4 / 3 / 2", 1);
}

Test(recursive, simple_priority)
{
    do_success("1 + 2 * 3", 7);
}

Test(recursive, more_priority)
{
    do_success("1 + 6 / 3 + 4 * 6 + 14 / 7", 29);
}

Test(recursive, fail_parenthesis)
{
    do_failure("(1 + 2))");
}

Test(recursive, simple_parenthesis)
{
    do_success("(1 + 2) * 3", 9);
}

Test(recursive, more_parentheses)
{
    do_success("(1 + 2) * (3 - 4)", -3);
}

Test(recursive, unary_minus)
{
    do_success("-1", -1);
}

Test(recursive, unary_plus)
{
    do_success("+1", 1);
}

Test(recursive, unary_torture)
{
    do_success("--+++--+-+-+-1", -1);
}

Test(recursive, factorial)
{
    do_success("3!", 6);
}

Test(recursive, fail_factorial)
{
    do_failure("3!!");
}

Test(recursive, power)
{
    do_success("4^3", 64);
}

Test(recursive, powers)
{
    do_success("4^3^2", 262144);
}

Test(recursive, fact_and_power)
{
    do_success("2^3!", 64);
}

Test(recursive, altogether)
{
    do_success("  -   3 ^ 2 + - 4 * 8 / 2 + + 3! -- 2 + ((-1) + 1) * 2 ", -17);
}
