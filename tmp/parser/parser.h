#ifndef PARSER_H
#define PARSER_H

#include "token.h"
#include "ast.h"
#include "lexer/lexer.h"
#include "list.h"
#include "string_buffer.h"

AstNode* parse(String source, List token_list);

#endif
