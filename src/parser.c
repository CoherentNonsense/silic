#include "parser.h"

#include "util.h"
#include "token.h"
#include <chnlib/maybe.h>
#include <chnlib/logger.h>
#include <chnlib/dynarray.h>

#include <stdio.h>
#include <stdlib.h>

typedef struct ParserContext {
    Module* module;
    unsigned int token_index;
} ParserContext;

static Token* current_token(ParserContext* context) {
    return &context->module->token_list[context->token_index];
}

static Token* consume_token(ParserContext* context) {
    Token* token = current_token(context);
    context->token_index += 1;

    return token;
}

static Token* expect_token(ParserContext* context, TokenKind kind) {
    Token* token = consume_token(context);
    if (token->kind != kind) {
        module_add_error(
            context->module,
            token,
            "unexpected token",
            "expected %s, found %s",
            token_string(kind),
            token_string(token->kind)
        );

        return null;
    }

    return token;
}

static bool expect_semicolon(ParserContext* context) {
    Token* token = current_token(context);
    if (token->kind != TokenKind_Semicolon) {
        // add the semicolon token for error
        Token* prev_token = &context->module->token_list[context->token_index - 1];

        Token* semicolon = malloc(sizeof(Token));
        semicolon->kind = TokenKind_Semicolon;
        semicolon->position = prev_token->position;
        semicolon->span = prev_token->span;
        semicolon->position.column += prev_token->span.len;
        semicolon->span.data += prev_token->span.len;
        semicolon->span.len = 1;

        module_add_error(
            context->module,
            semicolon,
            "you fool",
            "missing semicolon"
        );

        return false;
    }

    consume_token(context);

    return true;
}

static Type* parse_type(ParserContext* context) {
    Type* type = malloc(sizeof(Type));
    
    Token* token = consume_token(context);
    type->symbol = token->span;
    switch (token->kind) {
	case TokenKind_Star: {
	    type->kind = TypeKind_Ptr;
	    type->ptr.to = parse_type(context);

	    break;
	}

	case TokenKind_Bang: {
	    type->kind = TypeKind_Never;
	    break;
	}

	case TokenKind_Symbol: {
	    type->kind = TypeKind_Symbol;
	    break;
	}

	default: {
	    sil_panic("Unexpected type: %s", token_string(token->kind));
	}
    }

    return type;
}

static Stmt* parse_statement(ParserContext* context);
static Expr* parse_expression(ParserContext* context);

static void operator_precedence(TokenKind operator_kind, int* left, int* right) {
    int precedence;
    switch (operator_kind) {
        case TokenKind_KeywordAnd: precedence = OpPrec_And; break;
        case TokenKind_KeywordOr: precedence = OpPrec_Or; break;
        case TokenKind_GreaterThan: precedence = OpPrec_CmpGt; break;
        case TokenKind_LessThan: precedence = OpPrec_CmpLt; break;
        case TokenKind_Equality: precedence = OpPrec_CmpEq; break;
        case TokenKind_Inequality: precedence = OpPrec_CmpNotEq; break;
	case TokenKind_Equals: precedence = OpPrec_Assign; break;
	case TokenKind_Plus: precedence = OpPrec_Add; break;
	case TokenKind_Dash: precedence = OpPrec_Sub; break;
	case TokenKind_Star: precedence = OpPrec_Mul; break;
	case TokenKind_Slash: precedence = OpPrec_Div; break;
	default: precedence = OpPrec_Invalid;
    }

    *left = (precedence * 2) - 1;
    *right = precedence * 2;
}

static Block* parse_block(ParserContext* context) {
    Block* block = malloc(sizeof(Block));
    block->statements = dynarray_init();

    Maybe(expect_token(context, TokenKind_LBrace));
    
    while (current_token(context)->kind != TokenKind_RBrace) {
	Stmt* statement = Maybe(parse_statement(context));

	dynarray_push(block->statements, &statement);
    }

    consume_token(context);

    return block;
}

static NumberLit* parse_number_literal(ParserContext* context) {
    NumberLit* lit = malloc(sizeof(NumberLit));

    Token* token = consume_token(context);
    lit->span = token->span;

    return lit;
}

static Expr* parse_primary_expression(ParserContext* context) {
    Expr* expression = malloc(sizeof(Expr)); 

    switch (current_token(context)->kind) {
        case TokenKind_LParen: {
            consume_token(context);
            expression = Maybe(parse_expression(context));

            Maybe(expect_token(context, TokenKind_RParen));

            break;
       }

	case TokenKind_KeywordReturn: {
	    expression->kind = ExprKind_Ret;
	    consume_token(context);

	    expression->ret = Maybe(parse_expression(context));

	    break;
	}

	case TokenKind_NumberLiteral: {
	    expression->kind = ExprKind_NumberLit;
	    expression->number_literal = Maybe(parse_number_literal(context));

	    break;
	}

	case TokenKind_StringLiteral: {
	    expression->kind = ExprKind_StringLit;
	    expression->string_literal.span = consume_token(context)->span;

	    break;
	}

        case TokenKind_KeywordTrue:
        case TokenKind_KeywordFalse: {
            Token* boolean = consume_token(context);

            expression->kind = ExprKind_BoolLit;
            expression->boolean = boolean->kind == TokenKind_KeywordTrue;

            break;
        }

	case TokenKind_KeywordLet: {
	    consume_token(context);

	    expression->kind = ExprKind_Let;

	    Token* name_token = Maybe(expect_token(context, TokenKind_Symbol));
	    expression->let = Maybe(malloc(sizeof(Let)));
	    expression->let->name = name_token->span;

            Token* maybe_colon = current_token(context);
            if (maybe_colon->kind == TokenKind_Colon) {
                consume_token(context);

                expression->let->type = Maybe(parse_type(context));
            } else {
                expression->let->type = null;
            }

	    Maybe(expect_token(context, TokenKind_Equals));

	    expression->let->value = Maybe(parse_expression(context));

	    break;
	}

	case TokenKind_Symbol: {
	    Token* symbol_token = consume_token(context);
	    if (current_token(context)->kind != TokenKind_LParen) {
		expression->kind = ExprKind_Symbol;
		expression->symbol = symbol_token->span;

		break;
	    }

	    expression->kind = ExprKind_FnCall;

	    expression->fn_call = Maybe(malloc(sizeof(FnCall)));
	    expression->fn_call->name = symbol_token->span;

	    Maybe(expect_token(context, TokenKind_LParen));

	    expression->fn_call->arguments = dynarray_init();
	    while (current_token(context)->kind != TokenKind_RParen) {
		Expr* arg = Maybe(parse_expression(context));
		dynarray_push(expression->fn_call->arguments, &arg);

		if (current_token(context)->kind != TokenKind_Comma) {
		    break;
		}

		consume_token(context);
	    }

	    Maybe(expect_token(context, TokenKind_RParen));

	    break;
	}

	case TokenKind_LBrace: {
	    expression->kind = ExprKind_Block;
	    expression->block = Maybe(parse_block(context));

	    break;
	}

        case TokenKind_KeywordBreak: {
            consume_token(context);
            expression->kind = ExprKind_Break;
            break;
        }

        case TokenKind_KeywordContinue: {
            consume_token(context);
            expression->kind = ExprKind_Continue;
            break;
        }

	case TokenKind_KeywordIf: {
	    consume_token(context);
	    expression->kind = ExprKind_If;
	    expression->if_expr = Maybe(malloc(sizeof(If)));
	    expression->if_expr->condition = Maybe(parse_expression(context));

	    if (current_token(context)->kind != TokenKind_LBrace) {
		sil_panic("Expected block after if");
	    }
	    expression->if_expr->then = Maybe(parse_expression(context));
	   
	    // else (if) branch
	    if (current_token(context)->kind == TokenKind_KeywordElse) {
		consume_token(context);

		Token* next_token = current_token(context);
		if (next_token->kind != TokenKind_KeywordIf &&
		    next_token->kind != TokenKind_LBrace
                ) {
		    sil_panic("Expected 'if' or '{' after an else");
		}

		expression->if_expr->otherwise = Maybe(parse_expression(context));
	    } else {
		expression->if_expr->otherwise = null;
	    }

	    break;
	}

	case TokenKind_KeywordMatch: {
	    consume_token(context);
	    expression->kind = ExprKind_Match;
	    expression->match = Maybe(malloc(sizeof(Match)));
	    expression->match->arms = Maybe(dynarray_init());

	    expression->match->condition = Maybe(parse_expression(context));

	    Maybe(expect_token(context, TokenKind_LBrace));
	   
	    while (current_token(context)->kind != TokenKind_RBrace) {
		MatchArm* arm = Maybe(malloc(sizeof(MatchArm)));
		arm->pattern = Maybe(parse_number_literal(context));

		Maybe(expect_token(context, TokenKind_FatArrow));
		arm->then = Maybe(parse_expression(context));

		dynarray_push(expression->match->arms, &arm);

		if (current_token(context)->kind != TokenKind_RBrace) {
		    Maybe(expect_token(context, TokenKind_Comma));
		}

	    }

	    Maybe(expect_token(context, TokenKind_RBrace));

	    break;
	}

        case TokenKind_KeywordLoop: {
            consume_token(context);
            expression->kind = ExprKind_Loop;

            if (current_token(context)->kind != TokenKind_LBrace) {
                module_add_error(context->module, current_token(context), "expected '{'", "loop body must be a block");
                return null;
            }

            expression->loop = Maybe(malloc(sizeof(Loop)));
            expression->loop->body = Maybe(parse_primary_expression(context));

            break;
        }

	default: {
            module_add_error(context->module, current_token(context), "expected expression", "expression cannot start with %s", token_string(current_token(context)->kind));
            return null;
	}
    }

    return expression;
}

static Expr* parse_expression_prec(ParserContext* context, int precedence) {
    Expr* left_expression = Maybe(parse_primary_expression(context));

    while (1) {
	Token* operator_token = current_token(context);
	int left, right;
	operator_precedence(operator_token->kind, &left, &right);

	if (left == -1) { break; }

	if (left < precedence) { break; }

	consume_token(context);

	Expr* right_expression = Maybe(parse_expression_prec(context, right));

	Expr* operator = Maybe(malloc(sizeof(Expr)));
	operator->kind = ExprKind_BinOp;
	operator->binary_operator = Maybe(malloc(sizeof(BinOp)));

	switch (operator_token->kind) {
            case TokenKind_KeywordAnd: operator->binary_operator->kind = BinOpKind_And; break;
            case TokenKind_KeywordOr: operator->binary_operator->kind = BinOpKind_Or; break;
            case TokenKind_LessThan: operator->binary_operator->kind = BinOpKind_CmpLt; break;
            case TokenKind_GreaterThan: operator->binary_operator->kind = BinOpKind_CmpGt; break;
            case TokenKind_Equality: operator->binary_operator->kind = BinOpKind_CmpEq; break;
            case TokenKind_Inequality: operator->binary_operator->kind = BinOpKind_CmpNotEq; break;
	    case TokenKind_Equals: operator->binary_operator->kind = BinOpKind_Assign; break;
	    case TokenKind_Plus: operator->binary_operator->kind = BinOpKind_Add; break;
	    case TokenKind_Dash: operator->binary_operator->kind = BinOpKind_Sub; break;
	    case TokenKind_Star: operator->binary_operator->kind = BinOpKind_Mul; break;
	    case TokenKind_Slash: operator->binary_operator->kind = BinOpKind_Div; break;
	    default: sil_panic("Unhandled operator");
	}

	operator->binary_operator->left = left_expression;
	operator->binary_operator->right = right_expression;

	left_expression = operator;
    }

    return left_expression;
}

static Expr* parse_expression(ParserContext* context) {
    return parse_expression_prec(context, 0);
}

static Stmt* parse_statement(ParserContext* context) {
    Stmt* statement = Maybe(malloc(sizeof(Stmt)));

    switch (current_token(context)->kind) {
	default: {
	    statement->kind = StmtKind_Expr;
	    statement->expression = Maybe(parse_expression(context));
	   
	    if (!parser_should_remove_statement_semicolon(statement->expression)) {
		Maybe(expect_semicolon(context));
	    }

	    break;
	}
    }
    return statement;
}

static FnSig* parse_fn_signature(ParserContext* context, String* name) {
    FnSig* fn_sig = Maybe(malloc(sizeof(FnSig)));

    Maybe(expect_token(context, TokenKind_KeywordFn));

    Token* name_token = Maybe(expect_token(context, TokenKind_Symbol));
    *name = name_token->span;

    Maybe(expect_token(context, TokenKind_LParen));

    fn_sig->parameters = Maybe(dynarray_init());
    while (current_token(context)->kind != TokenKind_RParen) {
	FnParam* parameter = Maybe(malloc(sizeof(FnParam)));

	Token* name_token = Maybe(expect_token(context, TokenKind_Symbol));
	parameter->name = name_token->span;

	Maybe(expect_token(context, TokenKind_Colon));

	parameter->type = Maybe(parse_type(context));

	dynarray_push(fn_sig->parameters, &parameter);

	if (current_token(context)->kind != TokenKind_Comma) {
	    break;
	}

	consume_token(context);
    }

    Maybe(expect_token(context, TokenKind_RParen));

    if (current_token(context)->kind == TokenKind_Arrow) {
	consume_token(context);
	fn_sig->return_type = Maybe(parse_type(context));
    } else {
	Type* void_type = Maybe(malloc(sizeof(Type)));
	void_type->kind = TypeKind_Void;
	fn_sig->return_type = void_type;
    }

    return fn_sig;
}

static FnDef* parse_fn_definition(ParserContext* context, String* name) {
    FnDef* fn_decl = Maybe(malloc(sizeof(FnDef)));

    fn_decl->signature = Maybe(parse_fn_signature(context, name));

    // TODO: Make block expression
    if (current_token(context)->kind != TokenKind_LBrace) {
        module_add_error(context->module, current_token(context), "expected '{'", "Function body must be a block");
        return null;
    }

    fn_decl->body = Maybe(parse_primary_expression(context));

    return fn_decl;
}

static Constant* parse_constant(ParserContext* context, String* name) {
    Constant* constant = Maybe(malloc(sizeof(Constant)));

    Token* ident = Maybe(expect_token(context, TokenKind_Symbol));
    *name = ident->span;

    Maybe(expect_token(context, TokenKind_Colon));

    constant->type = Maybe(parse_type(context));

    Maybe(expect_token(context, TokenKind_Equals));

    constant->value = Maybe(parse_expression(context));

    return constant;
}

static Item* parse_item(ParserContext* context) {
    Item* item = malloc(sizeof(Item));

    if (current_token(context)->kind == TokenKind_KeywordPub) {
	consume_token(context);
	item->visibility.is_pub = true;
    } else {
	item->visibility.is_pub = false;
    }

    switch (current_token(context)->kind) {
	case TokenKind_KeywordFn: {
	    item->kind = ItemKind_FnDef;
	    item->fn_definition = Maybe(parse_fn_definition(context, &item->name));
	    break;
	}

	case TokenKind_KeywordExtern: {
	    item->kind = ItemKind_ExternFn;
	    consume_token(context);
	    item->extern_fn = malloc(sizeof(ExternFn));
	    item->extern_fn->signature = parse_fn_signature(context, &item->name);
	    Maybe(expect_token(context, TokenKind_Semicolon));
	    break;
	}

	case TokenKind_KeywordConst: {
	    item->kind = ItemKind_Const;
            consume_token(context);

	    item->constant = Maybe(parse_constant(context, &item->name));

            Maybe(expect_semicolon(context));

	    break;
	}
	
	default: {
            module_add_error(
                context->module,
                current_token(context),
                "expected item (e.g. fn, struct)",
                "expected item, found %s",
                token_string(current_token(context)->kind)
            );
	}
    }

    return item;
}

static AstRoot* parse_root(ParserContext* context) {
    AstRoot* root = Maybe(malloc(sizeof(AstRoot)));

    root->items = dynarray_init();

    while (current_token(context)->kind != TokenKind_Eof) {
	Item* item = Maybe(parse_item(context));

	dynarray_push(root->items, &item);
    }

    return root;
}

void parser_parse(Module* module) {
    ParserContext context;
    context.module = module;
    context.token_index = 0;

    module->ast = parse_root(&context);
}

bool parser_should_remove_statement_semicolon(Expr* expression) {
    return (
	expression->kind == ExprKind_If ||
	expression->kind == ExprKind_Match ||
	expression->kind == ExprKind_Block ||
        expression->kind == ExprKind_Loop
    );
}
