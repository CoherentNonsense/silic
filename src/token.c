#include "token.h"

#include "string.h"

int token_symbol_compare(String source, Token* token, char* symbol) {
    return !strncmp(source.data + token->start, symbol, token->end - token->start);
}

char* token_string(TokenType type) {
    switch (type) {
        case TokenType_Eof: return "EOF"; break;
        case TokenType_Symbol: return "Symbol"; break;
        case TokenType_NumberLiteral: return "Number Literal"; break;
        case TokenType_StringLiteral: return "String Literal"; break;
        case TokenType_LBrace: return "Left Brace"; break;
        case TokenType_RBrace: return "Right Brace"; break;
        case TokenType_LParen: return "Left Parenthesis"; break;
        case TokenType_RParen: return "Right Parenthesis"; break;
        case TokenType_Colon: return "Colon"; break;
        case TokenType_Semicolon: return "SemiColon"; break;
        case TokenType_Comma: return "Comma"; break;
        case TokenType_Ampersand: return "Ampersand"; break;
        case TokenType_Arrow: return "Arrow"; break;
        case TokenType_Equals: return "Equals"; break;
        case TokenType_Plus: return "Plus"; break;
        case TokenType_Dash: return "Dash"; break;
        case TokenType_KeywordLet: return "Keyword(let)"; break;
        case TokenType_KeywordFn: return "Keyword(fn)"; break;
        case TokenType_KeywordReturn: return "Keyword(return)"; break;
        case TokenType_KeywordExtern: return "Keyword(extern)"; break;
        default: return "Unknown Token"; break;
    }
}
