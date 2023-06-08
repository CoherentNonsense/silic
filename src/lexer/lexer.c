#include "lexer.h"

#include "list.h"
#include "string_buffer.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum LexerState {
    TokenizerState_Start,
    TokenizerState_Symbol,
    TokenizerState_Number,
    TokenizerState_String,
    TokenizerState_Dash,
    TokenizerState_Slash,
    TokenizerState_Comment,
} TokenizerState;

typedef struct LexerContext{
    String source;
    unsigned int offset;
    TokenizerState state;
    TextPosition position;
    Token* current_token;
    List* token_list;
} TokenizerContext;


#define WHITESPACE \
    ' ': \
    case '\t': \
    case '\n': \
    case '\f': \
    case '\r': \
    case 0xb

#define DIGIT \
    '0': \
    case '1': \
    case '2': \
    case '3': \
    case '4': \
    case '5': \
    case '6': \
    case '7': \
    case '8': \
    case '9'

#define ALPHA \
    'a': \
    case 'b': \
    case 'c': \
    case 'd': \
    case 'e': \
    case 'f': \
    case 'g': \
    case 'h': \
    case 'i': \
    case 'j': \
    case 'k': \
    case 'l': \
    case 'm': \
    case 'n': \
    case 'o': \
    case 'p': \
    case 'q': \
    case 'r': \
    case 's': \
    case 't': \
    case 'u': \
    case 'v': \
    case 'w': \
    case 'x': \
    case 'y': \
    case 'z': \
    case 'A': \
    case 'B': \
    case 'C': \
    case 'D': \
    case 'E': \
    case 'F': \
    case 'G': \
    case 'H': \
    case 'I': \
    case 'J': \
    case 'K': \
    case 'L': \
    case 'M': \
    case 'N': \
    case 'O': \
    case 'P': \
    case 'Q': \
    case 'R': \
    case 'S': \
    case 'T': \
    case 'U': \
    case 'V': \
    case 'W': \
    case 'X': \
    case 'Y': \
    case 'Z'

static void begin_token(TokenizerContext* context, TokenType type) {
    context->current_token = list_add(Token, context->token_list);

    Token* token = context->current_token;
    token->type = type;
    token->position = context->position;
    token->start = context->offset;
}

static void end_token(TokenizerContext* context) {
    Token* token = context->current_token;
    token->end = context->offset + 1;

    if (token->type == TokenType_Symbol) {
        if (token_symbol_compare(context->source, token, "fn")) {
            token->type = TokenType_KeywordFn;
        } else if (token_symbol_compare(context->source, token, "return")) {
            token->type = TokenType_KeywordReturn;
        } else if (token_symbol_compare(context->source, token, "let")) {
            token->type = TokenType_KeywordLet;
        } else if (token_symbol_compare(context->source, token, "extern")) {
            token->type = TokenType_KeywordExtern;
        }
    }
}

List* tokenize(String source) {
    TokenizerContext context = {0};
    context.source = source;
    context.token_list = calloc(1, sizeof(List));

    for (context.offset = 0; context.offset < source.length; context.offset++) {
        char current_char = *(source.data + context.offset);

        switch (context.state) {

            case TokenizerState_Start:
                switch (current_char) {

                    case WHITESPACE:
                        break;
                    case ALPHA:
                    case '_':
                        begin_token(&context, TokenType_Symbol);
                        context.state = TokenizerState_Symbol;
                        break;
                    case DIGIT:
                        begin_token(&context, TokenType_NumberLiteral);
                        context.state = TokenizerState_Number;
                        break;
                    case '"':
                        begin_token(&context, TokenType_StringLiteral);
                        context.state = TokenizerState_String;
                        break;
                    case ';':
                        begin_token(&context, TokenType_Semicolon);
                        end_token(&context);
                        break;
                    case ':':
                        begin_token(&context, TokenType_Colon);
                        end_token(&context);
                        break;
                    case ',':
                        begin_token(&context, TokenType_Comma);
                        end_token(&context);
                        break;
                    case '&':
                        begin_token(&context, TokenType_Ampersand);
                        end_token(&context);
                        break;
                    case '*':
                        begin_token(&context, TokenType_Star);
                        end_token(&context);
                        break;
                    case '{':
                        begin_token(&context, TokenType_LBrace);
                        end_token(&context);
                        break;
                    case '}':
                        begin_token(&context, TokenType_RBrace);
                        end_token(&context);
                        break;
                    case '(':
                        begin_token(&context, TokenType_LParen);
                        end_token(&context);
                        break;
                    case ')':
                        begin_token(&context, TokenType_RParen);
                        end_token(&context);
                        break;
                    case '=':
                        begin_token(&context, TokenType_Equals);
                        end_token(&context);
                        break;
                    case '+':
                        begin_token(&context, TokenType_Plus);
                        end_token(&context);
                        break;
                    case '-':
                        begin_token(&context, TokenType_Dash);
                        context.state = TokenizerState_Dash;
                        break;
                    case '/':
                        context.state = TokenizerState_Slash;
                        break;
                    default:
                        sil_panic("Unknown character: %c", current_char);
                }
                break;

            case TokenizerState_Symbol:
                switch (current_char) {
                    case ALPHA:
                    case DIGIT:
                    case '_':
                        break;
                    default:
                        context.offset -= 1;
                        context.position.column -= 1;
                        end_token(&context);
                        context.state = TokenizerState_Start;
                        break;
                }
                break;

            case TokenizerState_Number:
                switch (current_char) {
                    case DIGIT:
                        break;
                    default:
                        context.offset -= 1;
                        context.position.column -= 1;
                        end_token(&context);
                        context.state = TokenizerState_Start;
                        break;
                }
                break;

            case TokenizerState_String:
                switch (current_char) {
                    case '"':
                        end_token(&context);
                        context.state = TokenizerState_Start;
                        break;
                    default: break;
                }
                break;

            case TokenizerState_Dash:
                switch (current_char) {
                    case '>':
                        context.current_token->type = TokenType_Arrow;
                        end_token(&context);
                        context.state = TokenizerState_Start;
                        break;
                    default:
                        context.offset -= 1;
                        context.position.column -= 1;
                        end_token(&context);
                        context.state = TokenizerState_Start;
                        break;
                }
                break;

            case TokenizerState_Slash:
                if (current_char != '/') {
                    context.offset -= 1;
                    context.position.column -= 1;
                    begin_token(&context, TokenType_Slash);
                    end_token(&context);
                    context.state = TokenizerState_Start;
                    break;
                }
                context.state = TokenizerState_Comment;
                break;

            case TokenizerState_Comment:
                if (current_char == '\n') {
                    context.state = TokenizerState_Start;
                }
                break;

            default:
                sil_panic("Unknown tokenizer state");
        }

        if (current_char == '\n') {
            context.position.line += 1;
            context.position.column = 0;
        } else {
            context.position.column += 1;
        }
    }

    // end of file
    begin_token(&context, TokenType_Eof);
    end_token(&context);

    return context.token_list;
}

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
        default: return "Unknown"; break;
    }
}
