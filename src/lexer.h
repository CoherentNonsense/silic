#ifndef LEXER_H
#define LEXER_H

#include "string_buffer.h"
#include "token.h"

TokenList lexer_lex(String source);

#endif // !LEXER_H
