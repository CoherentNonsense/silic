#include "token.h"

#include "string.h"

int token_compare_literal(Token* token, char* literal) {
    return !strncmp(token->span.start, literal, token->span.length);
}

char* token_string(TokenKind kind) {
    switch (kind) {
        case TokenKind_Eof: return "EOF"; break;
        case TokenKind_Symbol: return "Symbol"; break;
        case TokenKind_NumberLiteral: return "Number Literal"; break;
        case TokenKind_StringLiteral: return "String Literal"; break;
        case TokenKind_LBrace: return "Left Brace"; break;
        case TokenKind_RBrace: return "Right Brace"; break;
        case TokenKind_LParen: return "Left Parenthesis"; break;
        case TokenKind_RParen: return "Right Parenthesis"; break;
        case TokenKind_Colon: return "Colon"; break;
        case TokenKind_Semicolon: return "SemiColon"; break;
        case TokenKind_Comma: return "Comma"; break;
        case TokenKind_Ampersand: return "Ampersand"; break;
        case TokenKind_Arrow: return "Arrow"; break;
        case TokenKind_Equals: return "Equals"; break;
        case TokenKind_Plus: return "Plus"; break;
        case TokenKind_Dash: return "Dash"; break;
        case TokenKind_KeywordLet: return "Keyword(let)"; break;
        case TokenKind_KeywordFn: return "Keyword(fn)"; break;
        case TokenKind_KeywordReturn: return "Keyword(return)"; break;
        case TokenKind_KeywordExtern: return "Keyword(extern)"; break;
        default: return "Unknown Token"; break;
    }
}

void token_print(Token* token) {
    printf("%.*s", (int)token->span.length, token->span.start);
}
