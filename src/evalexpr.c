#include <stdio.h>
#include <stdlib.h>

#include "ast/ast.h"
#include "eval/eval.h"
#include "parse/parse.h"

int main(void)
{
    char *line = NULL;
    size_t size = 0;
    ssize_t ret = 0;

    while ((getline(&line, &size, stdin)) > 0)
    {
        struct ast_node *ast = recursive_parse(line);

        if (ast == NULL)
        {
            fputs("Could not parse input\n", stderr);
            ret = 1;
            continue;
        }

        printf("%d\n", eval_ast(ast));

        destroy_ast(ast);
    }

    free(line);

    return ret;
}
