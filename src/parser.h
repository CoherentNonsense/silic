#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "ast.h"

AstRoot* parser_parse(String source, List token_list);

#endif // PARSER_H
