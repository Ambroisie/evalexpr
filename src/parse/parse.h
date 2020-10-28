#ifndef PARSE_H
#define PARSE_H

#include "ast/ast.h"

struct ast_node *climbing_parse(const char *input);
struct ast_node *recursive_parse(const char *input);

#endif /* !PARSE_H */
