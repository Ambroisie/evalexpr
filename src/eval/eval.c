#include "eval.h"

#define UNREACHABLE() __builtin_unreachable()

static int my_pow(int lhs, int rhs)
{
    if (!rhs)
        return 1;
    int rec = my_pow(lhs * lhs, rhs / 2);
    if (rhs & 1)
        rec *= lhs;
    return rec;
}

static int my_fact(int num)
{
    int ret = 1;
    while (num > 1)
        ret *= num--;
    return ret;
}

static int eval_unop(const struct unop_node *un_op)
{
    switch (un_op->op)
    {
    case UNOP_IDENTITY:
        return eval_ast(un_op->tree);
    case UNOP_NEGATE:
        return -eval_ast(un_op->tree);
    case UNOP_FACT:
        return my_fact(eval_ast(un_op->tree));
    }
    UNREACHABLE();
}

static int eval_binop(const struct binop_node *bin_op)
{
#define EVAL_OP(OP, TREE) (eval_ast((TREE)->lhs) OP eval_ast((TREE)->rhs))
    switch (bin_op->op)
    {
    case BINOP_PLUS:
        return EVAL_OP(+, bin_op);
    case BINOP_MINUS:
        return EVAL_OP(-, bin_op);
    case BINOP_TIMES:
        return EVAL_OP(*, bin_op);
    case BINOP_DIVIDES:
        return EVAL_OP(/, bin_op);
    case BINOP_POW:
        return my_pow(eval_ast(bin_op->lhs), eval_ast(bin_op->rhs));
    }
#undef EVAL_OP
    UNREACHABLE();
}

int eval_ast(const struct ast_node *ast)
{
    switch (ast->kind)
    {
    case NODE_NUM:
        return ast->val.num;
    case NODE_UNOP:
        return eval_unop(&ast->val.un_op);
    case NODE_BINOP:
        return eval_binop(&ast->val.bin_op);
    }
    UNREACHABLE();
}
