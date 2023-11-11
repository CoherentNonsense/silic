#include "lexer.h"

#include "token.h"
#include "util.h"

#include <chnlib/logger.h>

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
    LexerState_Equals,
    LexerState_Dot,
    LexerState_DotDot,
    LexerState_Slash,
    LexerState_Bang,
    LexerState_Comment,
    LexerState_MultilineComment,
} LexerState;

typedef struct LexerContext {
    Module* module;
    usize offset;
    LexerState state;
    TextPosition position;
    Token* current_token;
} LexerContext;

static char get_char(LexerContext* context, unsigned int offset) {
    return *(context->module->source.data + offset);
}

static void begin_token(LexerContext* context, TokenKind kind) {
    context->current_token = dynarray_add(context->module->token_list);

    Token* token = context->current_token;
    token->kind = kind;
    token->position = context->position;
    token->span.data = context->module->source.data + context->offset;
}

static void end_token(LexerContext* context) {
    Token* token = context->current_token;
    token->span.len = (context->offset - (usize)(token->span.data - context->module->source.data)) + 1;

    if (token->kind == TokenKind_Symbol) {
        if (token_compare_literal(token, "fn")) {
            token->kind = TokenKind_KeywordFn;
        } else if (token_compare_literal(token, "return")) {
            token->kind = TokenKind_KeywordReturn;
        } else if (token_compare_literal(token, "let")) {
            token->kind = TokenKind_KeywordLet;
        } else if (token_compare_literal(token, "extern")) {
            token->kind = TokenKind_KeywordExtern;
        } else if (token_compare_literal(token, "if")) {
            token->kind = TokenKind_KeywordIf;
	} else if (token_compare_literal(token, "match")) {
	    token->kind = TokenKind_KeywordMatch;
        } else if (token_compare_literal(token, "else")) {
            token->kind = TokenKind_KeywordElse;
        } else if (token_compare_literal(token, "true")) {
            token->kind = TokenKind_KeywordTrue;
        } else if (token_compare_literal(token, "false")) {
            token->kind = TokenKind_KeywordFalse;
        } else if (token_compare_literal(token, "type")) {
            token->kind = TokenKind_KeywordType;
        } else if (token_compare_literal(token, "pub")) {
	    token->kind = TokenKind_KeywordPub;
	} else if (token_compare_literal(token, "const")) {
	    token->kind = TokenKind_KeywordConst;
	}
    }
}

void lexer_lex(Module* module) {
    LexerContext context;
    context.module = module;
    context.offset = 0;
    context.state = LexerState_Start;
    context.position = (TextPosition){ 1, 1};
    context.current_token = 0;

    context.module->token_list = dynarray_init();

    for (context.offset = 0; context.offset < module->source.len; context.offset++) {
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
		    case '.':
			begin_token(&context, TokenKind_Dot);
			context.state = LexerState_Dot;
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
                        context.state = LexerState_Bang;
                        break;
                    case '=':
                        begin_token(&context, TokenKind_Equals);
			context.state = LexerState_Equals;
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
		    case '%':
			begin_token(&context, TokenKind_Percent);
			end_token(&context);
			break;
                    default:
                        begin_token(&context, TokenKind_Eof);
                        end_token(&context);
                        module_add_error(
                            context.module,
                            context.current_token,
                            "Did you mean to add this?",
                            "Unexpected character '%c'",
                            current_char
                        );
                        return;
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

	    case LexerState_Dot:
		switch (current_char) {
		    case '.':
			context.current_token->kind = TokenKind_Range;
			context.state = LexerState_DotDot;
			break;
		    default:
			context.offset -= 1;
			context.position.column -= 1;
			end_token(&context);
			context.state = LexerState_Start;
			break;
		}
		break;

	    case LexerState_DotDot:
		switch (current_char) {
		    case '.':
			context.current_token->kind = TokenKind_RangeInclusive;
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

            case LexerState_Bang:
                switch (current_char) {
                    case '=':
                        context.current_token->kind = TokenKind_Inequality;
                        end_token(&context);
                        break;
                    default:
                        context.offset -= 1;
                        context.position.column -= 1;
                        end_token(&context);
                        break;
                }
                context.state = LexerState_Start;
                break;

	    case LexerState_Equals:
		switch (current_char) {
		    case '>':
			context.current_token->kind = TokenKind_FatArrow;
			end_token(&context);
			context.state = LexerState_Start;
			break;
		    case '=':
			context.current_token->kind = TokenKind_Equality;
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
                if ((current_char == '*') and (get_char(&context, context.offset + 1) == '/')) {
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
}
