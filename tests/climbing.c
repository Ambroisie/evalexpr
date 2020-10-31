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

#define SUCCESS(Name, Input, Expected) \
    Test(climbing, Name) { do_success(Input, Expected); }
#define FAILURE(Name, Input) \
    Test(climbing, Name) { do_failure(Input); }
#include "tests.inc"
