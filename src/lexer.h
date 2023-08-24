#ifndef LEXER_H
#define LEXER_H

#include "string_buffer.h"
#include "list.h"

List lexer_lex(String source);

#endif // !LEXER_H
