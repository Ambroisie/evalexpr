#include "ast.h"

#include <stdlib.h>

struct ast_node *make_num(int val)
{
    struct ast_node *ret = malloc(sizeof(*ret));

    if (ret == NULL)
        return ret;

    ret->kind = NODE_NUM;
    ret->val.num = val;

    return ret;
}

struct ast_node *make_unop(enum unop_kind op, struct ast_node *tree)
{
    struct ast_node *ret = malloc(sizeof(*ret));

    if (ret == NULL)
        return ret;

    ret->kind = NODE_UNOP;
    ret->val.un_op.op = op;
    ret->val.un_op.tree = tree;

    return ret;
}

struct ast_node *make_binop(enum binop_kind op, struct ast_node *lhs,
                            struct ast_node *rhs)
{
    struct ast_node *ret = malloc(sizeof(*ret));

    if (ret == NULL)
        return ret;

    ret->kind = NODE_BINOP;
    ret->val.bin_op.op = op;
    ret->val.bin_op.lhs = lhs;
    ret->val.bin_op.rhs = rhs;

    return ret;
}

void destroy_ast(struct ast_node *ast)
{
    if (!ast)
        return;

    switch (ast->kind)
    {
    case NODE_BINOP:
        destroy_ast(ast->val.bin_op.lhs);
        destroy_ast(ast->val.bin_op.rhs);
        break;
    case NODE_UNOP:
        destroy_ast(ast->val.un_op.tree);
        /* fallthrough */
    case NODE_NUM:
        break;
    }

    free(ast);
}
