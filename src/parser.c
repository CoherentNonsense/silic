#include "parser.h"

#include "util.h"
#include "token.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct ParserContext {
    Module* module;
    unsigned int token_index;
} ParserContext;

#define PROPAGATE_ERROR(context) do { if (context->module->has_errors) { return NULL; }  } while(0)

static Token* current_token(ParserContext* context) {
    return dynarray_get_ref(context->module->token_list, context->token_index);
}

static Token* consume_token(ParserContext* context) {
    Token* token = current_token(context);
    context->token_index += 1;

    return token;
}

#define EXPECT_TOKEN(context, kind) do { expect_token(context, kind); PROPAGATE_ERROR(context) } while(0)
static Token* expect_token(ParserContext* context, TokenKind kind) {
    Token* token = consume_token(context);
    if (token->kind != kind) {
	sil_panic("Unexpected token[%d:%d]: got %s. expected %s", token->position.line, token->position.column, token_string(token->kind), token_string(kind));
    }

    return token;
}

static void expect_semicolon(ParserContext* context) {
    Token* token = current_token(context);
    if (token->kind != TokenKind_Semicolon) {
        // add the semicolon token for error
        Token* prev_token = dynarray_get_ref(context->module->token_list, context->token_index - 1);

        Token* semicolon = malloc(sizeof(Token));
        semicolon->kind = TokenKind_Semicolon;
        semicolon->position = prev_token->position;
        semicolon->span = prev_token->span;
        semicolon->position.column += 1;
        semicolon->span.start += 1;

        module_add_error(
            context->module,
            semicolon,
            "missing semicolon",
            "missing semicolon"
        );
    }

    consume_token(context);
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
	    // TODO: desparate for type tables :(
	    if (strncmp(token->span.start, "i32", 3) == 0) {
		type->kind = TypeKind_Int;
		break;
	    }
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
    dynarray_init(block->statements);

    expect_token(context, TokenKind_LBrace);
    
    while (current_token(context)->kind != TokenKind_RBrace) {
	Stmt* statement = parse_statement(context);
	dynarray_push(block->statements, statement);
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
	case TokenKind_KeywordReturn: {
	    expression->kind = ExprKind_Ret;
	    consume_token(context);

	    expression->ret = parse_expression(context);

	    break;
	}

	case TokenKind_NumberLiteral: {
	    expression->kind = ExprKind_NumberLit;
	    expression->number_literal = parse_number_literal(context);
	    break;
	}

	case TokenKind_StringLiteral: {
	    expression->kind = ExprKind_StringLit;
	    expression->string_literal.span = consume_token(context)->span;

	    break;
	}

	case TokenKind_KeywordLet: {
	    expression->kind = ExprKind_Let;

	    consume_token(context);

	    Token* name_token = expect_token(context, TokenKind_Symbol);
	    expression->let = malloc(sizeof(Let));
	    expression->let->name = name_token->span;

	    expect_token(context, TokenKind_Colon);

	    expression->let->type = parse_type(context);

	    expect_token(context, TokenKind_Equals);

	    expression->let->value = parse_expression(context);

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

	    expression->fn_call = malloc(sizeof(FnCall));
	    expression->fn_call->name = symbol_token->span;

	    expect_token(context, TokenKind_LParen);

	    dynarray_init(expression->fn_call->arguments);
	    while (current_token(context)->kind != TokenKind_RParen) {
		Expr* arg = parse_expression(context);
		dynarray_push(expression->fn_call->arguments, arg);

		if (current_token(context)->kind != TokenKind_Comma) {
		    break;
		}

		consume_token(context);
	    }

	    expect_token(context, TokenKind_RParen);

	    break;
	}

	case TokenKind_LBrace: {
	    expression->kind = ExprKind_Block;
	    expression->block = parse_block(context);

	    break;
	}

	case TokenKind_KeywordIf: {
	    consume_token(context);
	    expression->kind = ExprKind_If;
	    expression->if_expr = malloc(sizeof(If));
	    expression->if_expr->condition = parse_expression(context);

	    if (current_token(context)->kind != TokenKind_LBrace) {
		sil_panic("Expected block after if");
	    }
	    expression->if_expr->then = parse_expression(context);
	   
	    // else (if) branch
	    if (current_token(context)->kind == TokenKind_KeywordElse) {
		consume_token(context);

		Token* next_token = current_token(context);
		if (next_token->kind != TokenKind_KeywordIf &&
			next_token->kind != TokenKind_LBrace) {
		    sil_panic("Expected 'if' or '{' after an else");
		}

		expression->if_expr->otherwise.type = Yes;
		expression->if_expr->otherwise.value = parse_expression(context);
	    } else {
		expression->if_expr->otherwise.type = No;
	    }

	    break;
	}

	case TokenKind_KeywordMatch: {
	    consume_token(context);
	    expression->kind = ExprKind_Match;
	    expression->match = malloc(sizeof(Match));
	    dynarray_init(expression->match->arms);

	    expression->match->condition = parse_expression(context);

	    expect_token(context, TokenKind_LBrace);
	   
	    while (current_token(context)->kind != TokenKind_RBrace) {
		MatchArm* arm = malloc(sizeof(MatchArm));
		arm->pattern = parse_number_literal(context);
		expect_token(context, TokenKind_FatArrow);
		arm->then = parse_expression(context);
		dynarray_push(expression->match->arms, arm);

		if (current_token(context)->kind != TokenKind_RBrace) {
		    expect_token(context, TokenKind_Comma);
		}

	    }

	    expect_token(context, TokenKind_RBrace);

	    break;
	}

	default: {
	    sil_panic("Expected expression. Got %s", token_string(current_token(context)->kind));
	}
    }

    return expression;
}

static Expr* parse_expression_prec(ParserContext* context, int precedence) {
    Expr* left_expression;
    if (current_token(context)->kind == TokenKind_LParen) {
	consume_token(context);
	left_expression = parse_expression(context);
	expect_token(context, TokenKind_RParen);
    } else {
	left_expression = parse_primary_expression(context);
    }

    while (1) {
	Token* operator_token = current_token(context);
	int left, right;
	operator_precedence(operator_token->kind, &left, &right);

	if (left == -1) { break; }

	if (left < precedence) { break; }

	consume_token(context);

	Expr* right_expression = parse_expression_prec(context, right);
	Expr* operator = malloc(sizeof(Expr));
	operator->kind = ExprKind_BinOp;
	operator->binary_operator = malloc(sizeof(BinOp));

	switch (operator_token->kind) {
	    case TokenKind_Equals: operator->binary_operator->kind = BinOpKind_Eq; break;
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
    Stmt* statement = malloc(sizeof(Stmt));

    switch (current_token(context)->kind) {
	default: {
	    statement->kind = StmtKind_Expr;
	    statement->expression = parse_expression(context);
	   
	    if (!parser_should_remove_statement_semicolon(statement->expression)) {
		expect_token(context, TokenKind_Semicolon);
	    }

	    break;
	}
    }
    return statement;
}

static FnSig* parse_fn_signature(ParserContext* context, Span* name) {
    FnSig* fn_sig = malloc(sizeof(FnSig));

    expect_token(context, TokenKind_KeywordFn);

    Token* name_token = expect_token(context, TokenKind_Symbol);
    *name = name_token->span;

    expect_token(context, TokenKind_LParen);

    dynarray_init(fn_sig->parameters);
    while (current_token(context)->kind != TokenKind_RParen) {
	FnParam* parameter = malloc(sizeof(FnParam));

	Token* name_token = expect_token(context, TokenKind_Symbol);
	parameter->name = name_token->span;

	expect_token(context, TokenKind_Colon);

	parameter->type = parse_type(context);

	dynarray_push(fn_sig->parameters, parameter);

	if (current_token(context)->kind != TokenKind_Comma) {
	    break;
	}

	consume_token(context);
    }

    expect_token(context, TokenKind_RParen);

    if (current_token(context)->kind == TokenKind_Arrow) {
	consume_token(context);
	fn_sig->return_type = parse_type(context);
    } else {
	Type* void_type = malloc(sizeof(Type));
	void_type->kind = TypeKind_Void;
	fn_sig->return_type = void_type;
    }

    return fn_sig;
}

static FnDef* parse_fn_definition(ParserContext* context, Span* name) {
    FnDef* fn_decl = malloc(sizeof(FnDef));

    fn_decl->signature = parse_fn_signature(context, name);

    // TODO: Make block expression
    if (current_token(context)->kind != TokenKind_LBrace) {
	sil_panic("Expected block as function body");
    }
    fn_decl->body = parse_expression(context);

    return fn_decl;
}

static Constant* parse_constant(ParserContext* context, Span* name) {
    Constant* constant = malloc(sizeof(Constant));

    Token* ident = expect_token(context, TokenKind_Symbol);
    PROPAGATE_ERROR(context);

    *name = ident->span;

    expect_token(context, TokenKind_Colon);
    PROPAGATE_ERROR(context);

    constant->type = parse_type(context);
    PROPAGATE_ERROR(context);

    expect_token(context, TokenKind_Equals);
    PROPAGATE_ERROR(context);

    constant->value = parse_expression(context);
    PROPAGATE_ERROR(context);

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
	    item->fn_definition = parse_fn_definition(context, &item->name);
	    break;
	}

	case TokenKind_KeywordExtern: {
	    item->kind = ItemKind_ExternFn;
	    consume_token(context);
	    item->extern_fn = malloc(sizeof(ExternFn));
	    item->extern_fn->signature = parse_fn_signature(context, &item->name);
	    expect_token(context, TokenKind_Semicolon);
	    break;
	}

	case TokenKind_KeywordConst: {
	    item->kind = ItemKind_Const;
            consume_token(context);
	    item->constant = parse_constant(context, &item->name);
            expect_semicolon(context);
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
    AstRoot* root = malloc(sizeof(AstRoot));
    dynarray_init(root->items);

    while (current_token(context)->kind != TokenKind_Eof) {
	Item* item = parse_item(context);
        if (context->module->has_errors) {
            break;
        }
	dynarray_push(root->items, item);
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
	expression->kind == ExprKind_Block
    );
}
