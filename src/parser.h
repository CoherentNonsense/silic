#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "token.h"

AstRoot* parser_parse(Span source, TokenList token_list);

#endif // PARSER_H
