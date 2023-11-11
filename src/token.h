#ifndef TOKEN_H
#define TOKEN_H

#include <chnlib/str.h>


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
    TokenKind_FatArrow,
    TokenKind_Star,
    TokenKind_Slash,
    TokenKind_Tilde,
    TokenKind_Bang,
    TokenKind_Dot,
    TokenKind_Range,
    TokenKind_RangeInclusive,

    TokenKind_Equals,
    TokenKind_Equality,
    TokenKind_Inequality,
    TokenKind_Plus,
    TokenKind_Dash,
    TokenKind_Percent,

    TokenKind_KeywordLet,
    TokenKind_KeywordConst,
    TokenKind_KeywordFn,
    TokenKind_KeywordReturn,
    TokenKind_KeywordExtern,
    TokenKind_KeywordIf,
    TokenKind_KeywordMatch,
    TokenKind_KeywordElse,
    TokenKind_KeywordTrue,
    TokenKind_KeywordFalse,
    TokenKind_KeywordType,
    TokenKind_KeywordPub,
} TokenKind;

typedef struct TextPosition {
    unsigned int line;
    unsigned int column;
} TextPosition;

typedef struct Token {
    TokenKind kind;
    TextPosition position;
    String span;
} Token;

char* token_string(TokenKind type);
int token_compare_literal(Token* token, char* symbol);
void token_print(Token* token);

#endif
