#ifndef PARSE_H
#define PARSE_H

#include "ast/ast.h"

struct ast_node *recursive_parse(const char *input);

#endif /* !PARSE_H */
