#include "token.h"

#include "string.h"

int token_compare_literal(Token* token, char* literal) {
    return !strncmp(token->span.start, literal, token->span.length);
}

char* token_string(TokenKind kind) {
    switch (kind) {
        case TokenKind_Eof: return "EOF";
        case TokenKind_Symbol: return "Symbol";
        case TokenKind_NumberLiteral: return "Number Literal";
        case TokenKind_StringLiteral: return "String Literal";
        case TokenKind_LBrace: return "Left Brace";
        case TokenKind_RBrace: return "Right Brace";
        case TokenKind_LParen: return "Left Parenthesis";
        case TokenKind_RParen: return "Right Parenthesis";
        case TokenKind_Colon: return "Colon";
        case TokenKind_Semicolon: return "SemiColon";
        case TokenKind_Comma: return "Comma";
        case TokenKind_Ampersand: return "Ampersand";
        case TokenKind_Arrow: return "Arrow";
	case TokenKind_FatArrow: return "Fat Arrow";
	case TokenKind_Range: return "Range";
	case TokenKind_RangeInclusive: return "Range Inclusive";
        case TokenKind_Equals: return "Equals";
	case TokenKind_Equality: return "Equality";
        case TokenKind_Plus: return "Plus";
        case TokenKind_Dash: return "Dash";
	case TokenKind_Percent: return "Percent";
        case TokenKind_KeywordLet: return "Keyword(let)";
	case TokenKind_KeywordConst: return "Keyword(const)";
        case TokenKind_KeywordFn: return "Keyword(fn)";
	case TokenKind_KeywordIf: return "Keyword(if)";
	case TokenKind_KeywordMatch: return "Keyword(match)";
        case TokenKind_KeywordReturn: return "Keyword(return)";
        case TokenKind_KeywordExtern: return "Keyword(extern)";
	case TokenKind_KeywordStruct: return "Keyword(struct)";
	case TokenKind_KeywordPub: return "Keyword(pub)";
        default: return "Unknown Token";
    }
}

void token_print(Token* token) {
    printf("%.*s", (int)token->span.length, token->span.start);
}
