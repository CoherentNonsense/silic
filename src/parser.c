#include "parser.h"

#include "util.h"
#include "token.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct ParserContext {
    Module* module;
    unsigned int token_index;
} ParserContext;

static Token* current_token(ParserContext* context) {
    return dynarray_get_ref(context->module->token_list, context->token_index);
}

static Token* consume_token(ParserContext* context) {
    Token* token = current_token(context);
    context->token_index += 1;

    return token;
}

static Token* expect_token(ParserContext* context, TokenKind kind) {
    Token* token = consume_token(context);
    if (token->kind != kind) {
	sil_panic("Unexpected token: got %s. expected %s", token_string(token->kind), token_string(kind));
    }

    return token;
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

	    Token* token = consume_token(context);
	    expression->number_literal.span = token->span;

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
	    expression->let.name = name_token->span;

	    expect_token(context, TokenKind_Colon);

	    expression->let.type = parse_type(context);

	    expect_token(context, TokenKind_Equals);

	    expression->let.value = parse_expression(context);

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

	default: {
	    sil_panic("Expected expression");
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

	switch (operator_token->kind) {
	    case TokenKind_Equals: operator->binary_operator.kind = BinOpKind_Eq; break;
	    case TokenKind_Plus: operator->binary_operator.kind = BinOpKind_Add; break;
	    case TokenKind_Dash: operator->binary_operator.kind = BinOpKind_Sub; break;
	    case TokenKind_Star: operator->binary_operator.kind = BinOpKind_Mul; break;
	    case TokenKind_Slash: operator->binary_operator.kind = BinOpKind_Div; break;
	    default: sil_panic("Unhandled operator");
	}

	operator->binary_operator.left = left_expression;
	operator->binary_operator.right = right_expression;

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

	    expect_token(context, TokenKind_Semicolon);

	    break;
	}
    }
    return statement;
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
	fn_sig->return_type = NULL;
    }

    return fn_sig;
}

static FnDecl* parse_fn_declaration(ParserContext* context, Span* name) {
    FnDecl* fn_decl = malloc(sizeof(FnDecl));

    fn_decl->signature = parse_fn_signature(context, name);
    fn_decl->body = parse_block(context);

    return fn_decl;
}

static Item* parse_item(ParserContext* context) {
    Item* item = malloc(sizeof(Item));

    switch (current_token(context)->kind) {
	case TokenKind_KeywordFn: {
	    item->kind = ItemKind_FnDecl;
	    item->fn_declaration = parse_fn_declaration(context, &item->name);
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

	default: {
	    sil_panic("Expected item, got %s", token_string(current_token(context)->kind));
	}
    }

    return item;
}

static AstRoot* parse_root(ParserContext* context) {
    AstRoot* root = malloc(sizeof(AstRoot));
    dynarray_init(root->items);

    while (current_token(context)->kind != TokenKind_Eof) {
	Item* item = parse_item(context);
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
