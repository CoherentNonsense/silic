#include "token.h"

#include "util.h"
#include "string.h"
#include <stdio.h>

int token_compare_literal(Token* token, char* literal) {
    size_t const literal_length = strlen(literal);
    size_t const max_length = token->span.len > literal_length ?
	token->span.len : literal_length;
    return !strncmp(token->span.data, literal, max_length);
}

char* token_string(TokenKind kind) {
    switch (kind) {
        case TokenKind_Eof: return "end of file";
        case TokenKind_Symbol: return "symbol";
        case TokenKind_NumberLiteral: return "number literal";
        case TokenKind_StringLiteral: return "string literal";
        case TokenKind_LBrace: return "'}'";
        case TokenKind_RBrace: return "'{'";
        case TokenKind_LParen: return "'('";
        case TokenKind_RParen: return "')'";
        case TokenKind_Colon: return "':'";
        case TokenKind_Semicolon: return "';'";
        case TokenKind_Comma: return "','";
        case TokenKind_Ampersand: return "'&'";
        case TokenKind_Arrow: return "'->'";
	case TokenKind_FatArrow: return "'=>'";
	case TokenKind_Range: return "'..'";
	case TokenKind_RangeInclusive: return "'..='";
        case TokenKind_Equals: return "'='";
	case TokenKind_Equality: return "'=='";
        case TokenKind_Inequality: return "'!='";
        case TokenKind_Plus: return "'+'";
        case TokenKind_Dash: return "'-'";
        case TokenKind_Star: return "'*'";
        case TokenKind_Slash: return "'/'";
        case TokenKind_Tilde: return "'~'";
        case TokenKind_Bang: return "'!'";
        case TokenKind_Dot: return "'.'";
	case TokenKind_Percent: return "'%'";
        case TokenKind_KeywordLet: return "keyword 'let'";
	case TokenKind_KeywordConst: return "keyword 'const'";
        case TokenKind_KeywordFn: return "keyword 'fn'";
	case TokenKind_KeywordIf: return "keyword 'if'";
        case TokenKind_KeywordElse: return "keyword 'else'";
	case TokenKind_KeywordMatch: return "keyword 'match'";
        case TokenKind_KeywordReturn: return "keyword 'return'";
        case TokenKind_KeywordExtern: return "keyword 'extern'";
	case TokenKind_KeywordType: return "keyword 'type'";
	case TokenKind_KeywordPub: return "keyword 'pub'";
        case TokenKind_KeywordTrue: return "keyword 'true'";
        case TokenKind_KeywordFalse: return "keyword 'false'";
    }

    sil_panic("unhandled token");
}

void token_print(Token* token) {
    printf("%.*s", str_format(token->span));
}
