#ifndef TOKEN_H
#define TOKEN_H

#include "string_buffer.h"

typedef enum TokenKind {
    TokenKind_Eof,

    TokenKind_Symbol,
    TokenKind_NumberLiteral,
    TokenKind_StringLiteral,

    TokenKind_LBrace,
    TokenKind_RBrace,

    TokenKind_LParen,
    TokenKind_RParen,
    
    TokenKind_Colon,
    TokenKind_Semicolon,
    TokenKind_Comma,
    TokenKind_Ampersand,
    TokenKind_Arrow,
    TokenKind_Star,
    TokenKind_Slash,
    TokenKind_Tilde,
    TokenKind_Bang,

    TokenKind_Equals,
    TokenKind_Plus,
    TokenKind_Dash,

    TokenKind_KeywordLet,
    TokenKind_KeywordFn,
    TokenKind_KeywordReturn,
    TokenKind_KeywordExtern,
    TokenKind_KeywordIf,
    TokenKind_KeywordElse,
    TokenKind_KeywordTrue,
    TokenKind_KeywordFalse,
    TokenKind_KeywordStruct,
} TokenKind;

typedef struct TextPosition {
    unsigned int line;
    unsigned int column;
} TextPosition;

typedef struct Token {
    TokenKind kind;
    unsigned int start;
    unsigned int end;
    TextPosition position;
    String text;
} Token;

char* token_string(TokenKind type);
int token_symbol_compare(String source, Token* token, char* symbol);

#endif
