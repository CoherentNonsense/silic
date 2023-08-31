#include "lexer.h"

#include "token.h"
#include "util.h"

#define WHITESPACE \
    ' ': \
    case '\t': \
    case '\n': \
    case '\f': \
    case '\r': \
    case 0xb

#define DIGIT \
    '0': case '1': case '2': case '3': case '4': \
    case '5': case '6': case '7': case '8': case '9'

#define ALPHA \
    'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': \
    case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': \
    case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': \
    case 'v': case 'w': case 'x': case 'y': case 'z': case 'A': case 'B': \
    case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': \
    case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': \
    case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': \
    case 'X': case 'Y': case 'Z'

typedef enum LexerState {
    LexerState_Start,
    LexerState_Symbol,
    LexerState_Number,
    LexerState_String,
    LexerState_Dash,
    LexerState_Slash,
    LexerState_Comment,
    LexerState_MultilineComment,
} LexerState;

typedef struct LexerContext {
    String source;
    unsigned int offset;
    LexerState state;
    TextPosition position;
    Token* current_token;
    TokenList token_list;
} LexerContext;

static LexerContext init_context(String source) {
    LexerContext context;
    context.source = source;
    context.offset = 0;
    context.state = LexerState_Start;
    context.position = (TextPosition){ 1, 1 };
    context.current_token = 0;
    
    list_init(context.token_list);

    return context;
}

static char get_char(LexerContext* context, unsigned int offset) {
    return *(context->source.data + offset);
}

static void begin_token(LexerContext* context, TokenKind kind) {
    list_reserve(context->token_list, 1);
    context->current_token = list_get_ref(context->token_list, context->token_list.length - 1);

    Token* token = context->current_token;
    token->kind = kind;
    token->position = context->position;
    token->start = context->offset;
}

static void end_token(LexerContext* context) {
    Token* token = context->current_token;
    token->end = context->offset + 1;

    if (token->kind == TokenKind_Symbol) {
        if (token_symbol_compare(context->source, token, "fn")) {
            token->kind = TokenKind_KeywordFn;
        } else if (token_symbol_compare(context->source, token, "return")) {
            token->kind = TokenKind_KeywordReturn;
        } else if (token_symbol_compare(context->source, token, "let")) {
            token->kind = TokenKind_KeywordLet;
        } else if (token_symbol_compare(context->source, token, "extern")) {
            token->kind = TokenKind_KeywordExtern;
        } else if (token_symbol_compare(context->source, token, "if")) {
            token->kind = TokenKind_KeywordIf;
        } else if (token_symbol_compare(context->source, token, "else")) {
            token->kind = TokenKind_KeywordElse;
        } else if (token_symbol_compare(context->source, token, "true")) {
            token->kind = TokenKind_KeywordTrue;
        } else if (token_symbol_compare(context->source, token, "false")) {
            token->kind = TokenKind_KeywordFalse;
        } else if (token_symbol_compare(context->source, token, "struct")) {
            token->kind = TokenKind_KeywordStruct;
        }
    }
}

TokenList lexer_lex(String source) {
    LexerContext context = init_context(source);

    for (context.offset = 0; context.offset < source.length; context.offset++) {
        char current_char = get_char(&context, context.offset);

        switch (context.state) {

            case LexerState_Start:
                switch (current_char) {
                    case WHITESPACE: break;
                    case ALPHA:
                    case '_':
                        begin_token(&context, TokenKind_Symbol);
                        context.state = LexerState_Symbol;
                        break;
                    case DIGIT:
                        begin_token(&context, TokenKind_NumberLiteral);
                        context.state = LexerState_Number;
                        break;
                    case '"':
                        begin_token(&context, TokenKind_StringLiteral);
                        context.state = LexerState_String;
                        break;
                    case ';':
                        begin_token(&context, TokenKind_Semicolon);
                        end_token(&context);
                        break;
                    case ':':
                        begin_token(&context, TokenKind_Colon);
                        end_token(&context);
                        break;
                    case ',':
                        begin_token(&context, TokenKind_Comma);
                        end_token(&context);
                        break;
                    case '&':
                        begin_token(&context, TokenKind_Ampersand);
                        end_token(&context);
                        break;
                    case '*':
                        begin_token(&context, TokenKind_Star);
                        end_token(&context);
                        break;
                    case '{':
                        begin_token(&context, TokenKind_LBrace);
                        end_token(&context);
                        break;
                    case '}':
                        begin_token(&context, TokenKind_RBrace);
                        end_token(&context);
                        break;
                    case '(':
                        begin_token(&context, TokenKind_LParen);
                        end_token(&context);
                        break;
                    case ')':
                        begin_token(&context, TokenKind_RParen);
                        end_token(&context);
                        break;
                    case '~':
                        begin_token(&context, TokenKind_Tilde);
                        end_token(&context);
                        break;
                    case '!':
                        begin_token(&context, TokenKind_Bang);
                        end_token(&context);
                        break;
                    case '=':
                        begin_token(&context, TokenKind_Equals);
                        end_token(&context);
                        break;
                    case '+':
                        begin_token(&context, TokenKind_Plus);
                        end_token(&context);
                        break;
                    case '-':
                        begin_token(&context, TokenKind_Dash);
                        context.state = LexerState_Dash;
                        break;
                    case '/':
                        context.state = LexerState_Slash;
                        break;
                    default:
                        sil_panic("Unknown character: %c", current_char);
                }
                break;

            case LexerState_Symbol:
                switch (current_char) {
                    case ALPHA:
                    case DIGIT:
                    case '_':
                        break;
                    default:
                        context.offset -= 1;
                        context.position.column -= 1;
                        end_token(&context);
                        context.state = LexerState_Start;
                        break;
                }
                break;

            case LexerState_Number:
                switch (current_char) {
                    case DIGIT:
                        break;
                    default:
                        context.offset -= 1;
                        context.position.column -= 1;
                        end_token(&context);
                        context.state = LexerState_Start;
                        break;
                }
                break;

            case LexerState_String:
                switch (current_char) {
                    case '"':
                        end_token(&context);
                        context.state = LexerState_Start;
                        break;
                    default: break;
                }
                break;

            case LexerState_Dash:
                switch (current_char) {
                    case '>':
                        context.current_token->kind = TokenKind_Arrow;
                        end_token(&context);
                        context.state = LexerState_Start;
                        break;
                    default:
                        context.offset -= 1;
                        context.position.column -= 1;
                        end_token(&context);
                        context.state = LexerState_Start;
                        break;
                }
                break;

            case LexerState_Slash:
                if (current_char == '*') {
                    context.state = LexerState_MultilineComment;
                } else if (current_char == '/') {
                    context.state = LexerState_Comment;
                } else {
                    context.offset -= 1;
                    context.position.column -= 1;
                    begin_token(&context, TokenKind_Slash);
                    end_token(&context);
                    context.state = LexerState_Start;
                }

                break;

            case LexerState_Comment:
                if (current_char == '\n') {
                    context.state = LexerState_Start;
                }
                break;

            case LexerState_MultilineComment:
                if (current_char == '*' && get_char(&context, context.offset + 1) == '/') {
                    context.offset += 1;
                    context.position.column += 1;
                    context.state = LexerState_Start;
                }
                break;

            default:
                sil_panic("Unknown tokenizer state");
        }

        if (current_char == '\n') {
            context.position.line += 1;
            context.position.column = 1;
        } else {
            context.position.column += 1;
        }
    }

    // end of file
    begin_token(&context, TokenKind_Eof);
    end_token(&context);

    return context.token_list;
}
