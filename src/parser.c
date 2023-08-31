#include "parser.h"

#include "util.h"
#include "token.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct ParserContext {
    String source;
    TokenList token_list;
    unsigned int token_index;
} ParserContext;

static Token* current_token(ParserContext* context) {
    return list_get_ref(context->token_list, context->token_index);
}

static Token* consume_token(ParserContext* context) {
    Token* token = current_token(context);
    context->token_index += 1;

    return token;
}

// TODO: Turn this into a macro that returns an error node on failure
static Token* expect_token(ParserContext* context, TokenKind kind) {
    Token* token = consume_token(context);
    if (token->kind != kind) {
	sil_panic("Unexpected token");
    }

    return token;
}

static Type* parse_type(ParserContext* context) {
    return NULL;
}

static Expr* parse_expression(ParserContext* context) {
    Expr* expression = malloc(sizeof(Expr)); 

    switch (current_token(context)->kind) {
	case TokenKind_KeywordReturn: {
	    expression->kind = ExprKind_Ret;
	    consume_token(context);

	    expression->ret = parse_expression(context);

	    break;
	}

	case TokenKind_NumberLiteral: {
	    Token* token = consume_token(context);
	    expression->kind = ExprKind_NumberLit;
	    expression->number_literal.text = string_copy(token->text);

	    break;
	}

	default: {
	    sil_panic("Expected expression");
	}
    }

    return expression;
}

static Stmt* parse_statement(ParserContext* context) {
    Stmt* statement = malloc(sizeof(Stmt));

    switch (current_token(context)->kind) {
	case TokenKind_KeywordReturn: {
	    statement->kind = StmtKind_Expr;
	    statement->expression = parse_expression(context);

	    expect_token(context, TokenKind_Semicolon);

	    break;
	}

	default: {
	    sil_panic("Expected statement");
	}
    }
    return statement;
}

static Block* parse_block(ParserContext* context) {
    Block* block = malloc(sizeof(Block));
    list_init(block->statements);

    expect_token(context, TokenKind_LBrace);
    
    while (current_token(context)->kind != TokenKind_RBrace) {
	Stmt* statement = parse_statement(context);
	list_push(block->statements, statement);
    }

    consume_token(context);

    return block;
}

static FnDecl* parse_fn_declaration(ParserContext* context, Item** item) {
    FnDecl* fn_declaration = malloc(sizeof(FnDecl));
    (*item)->kind = ItemKind_FnDecl;

    expect_token(context, TokenKind_KeywordFn);

    Token* name_token = expect_token(context, TokenKind_Symbol);
    (*item)->name = string_from_token(context->source.data, name_token);

    expect_token(context, TokenKind_LParen);
    expect_token(context, TokenKind_RParen);

    fn_declaration->body = parse_block(context);

    return fn_declaration;
}

static Item* parse_item(ParserContext* context) {
    Item* item = malloc(sizeof(Item));

    switch (current_token(context)->kind) {
	case TokenKind_KeywordFn: {
	    item->fn_declaration = parse_fn_declaration(context, &item);
	    break;
	}

	default: {
	    sil_panic("Expected fn");
	}
    }

    return item;
}

static AstRoot* parse_root(ParserContext* context) {
    AstRoot* root = malloc(sizeof(AstRoot));
    list_init(root->items);

    while (current_token(context)->kind != TokenKind_Eof) {
	Item* item = parse_item(context);
	list_push(root->items, item);
    }

    return root;
}

Module parser_parse(String source, TokenList token_list) {
    ParserContext context;
    context.source = source;
    context.token_list = token_list;
    context.token_index = 0;

    AstRoot* ast = parse_root(&context);

    return (Module) { ast };
}
