#include <stdio.h>
#include <stdlib.h>

#include "ast/ast.h"
#include "eval/eval.h"
#include "parse/parse.h"

#ifndef _USE_CLIMBING
# define _USE_CLIMBING 0
#endif

int main(void)
{
    char *line = NULL;
    size_t size = 0;
    ssize_t ret = 0;

    while ((getline(&line, &size, stdin)) > 0)
    {
#if _USE_CLIMBING
        struct ast_node *ast = climbing_parse(line);
#else
        struct ast_node *ast = recursive_parse(line);
#endif

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
