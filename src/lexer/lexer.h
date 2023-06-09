#ifndef LEXER_H
#define LEXER_H

#include "string_buffer.h"
#include "list.h"
#include "util.h"

typedef enum TokenType {
    TokenType_Eof,

    TokenType_Symbol,
    TokenType_NumberLiteral,
    TokenType_StringLiteral,

    TokenType_LBrace,
    TokenType_RBrace,

    TokenType_LParen,
    TokenType_RParen,
    
    TokenType_Colon,
    TokenType_Semicolon,
    TokenType_Comma,
    TokenType_Ampersand,
    TokenType_Arrow,
    TokenType_Star,
    TokenType_Slash,
    TokenType_Tilde,
    TokenType_Bang,

    TokenType_Equals,
    TokenType_Plus,
    TokenType_Dash,

    TokenType_KeywordLet,
    TokenType_KeywordFn,
    TokenType_KeywordReturn,
    TokenType_KeywordExtern,
    TokenType_KeywordIf,
    TokenType_KeywordElse,
    TokenType_KeywordTrue,
    TokenType_KeywordFalse,
} TokenType;

typedef struct TextPosition {
    unsigned int line;
    unsigned int column;
} TextPosition;

typedef struct Token {
    TokenType type;
    unsigned int start;
    unsigned int end;
    TextPosition position;
    String text;
} Token;

List* tokenize(String source);

Token lexer_next();

char* token_string(TokenType type);

int token_symbol_compare(String source, Token* token, char* symbol);

#endif // !LEXER_H
