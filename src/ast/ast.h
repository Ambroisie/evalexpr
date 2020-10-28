#ifndef AST_H
#define AST_H

// Forward declaration
struct ast_node;

enum unop_kind
{
    UNOP_IDENTITY,
    UNOP_NEGATE,
    UNOP_FACT,
};

enum binop_kind
{
    BINOP_PLUS,
    BINOP_MINUS,
    BINOP_TIMES,
    BINOP_DIVIDES,
    BINOP_POW,
};

struct unop_node
{
    enum unop_kind op;
    struct ast_node *tree;
};

struct binop_node
{
    enum binop_kind op;
    struct ast_node *lhs;
    struct ast_node *rhs;
};

struct ast_node
{
    enum node_kind
    {
        NODE_UNOP,
        NODE_BINOP,
        NODE_NUM,
    } kind;
    union ast_val
    {
        struct unop_node un_op;
        struct binop_node bin_op;
        int num;
    } val;
};

struct ast_node *make_num(int val);

struct ast_node *make_unop(enum unop_kind op, struct ast_node *tree);

struct ast_node *make_binop(enum binop_kind op, struct ast_node *lhs,
                            struct ast_node *rhs);

void destroy_ast(struct ast_node *ast);

#endif /* !AST_H */
