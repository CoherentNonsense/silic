#include "parser.h"

#include "util.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct ParserContext {
    String source;
    List token_list;
    unsigned int token_index;
} ParserContext;

static Token* current_token(ParserContext* context) {
    return list_get(sizeof(Token), &context->token_list, context->token_index);
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

static Stmt* parse_statement(ParserContext* context) {
    return NULL;
}

static Block* parse_block(ParserContext* context) {
    Block* body = malloc(sizeof(Block));
    body->statements = list_init();

    expect_token(context, TokenKind_LBrace);
    
    while (current_token(context)->kind != TokenKind_RBrace) {
	Stmt* statement = list_add(sizeof(Stmt), &body->statements);
	statement = parse_statement(context);
    }

    consume_token(context);

    return NULL;
}

static Item* parse_fn_declaration(ParserContext* context, String* name) {
    Item* item = malloc(sizeof(Item));
    item->kind = ItemKind_FnDecl;

    expect_token(context, TokenKind_KeywordFn);

    Token* name_token = expect_token(context, TokenKind_Symbol);
    item->name = string_from_token(context->source.data, name_token);

    expect_token(context, TokenKind_LParen);
    expect_token(context, TokenKind_RParen);

    item->fn_declaration.body = parse_block(context);

    return item;
}

static Item* parse_item(ParserContext* context) {
    Item* item = malloc(sizeof(Item));

    switch (current_token(context)->kind) {
	case TokenKind_KeywordFn: {
	    item = parse_fn_declaration(context, &item->name);
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
    root->items = list_init();

    while (current_token(context)->kind != TokenKind_Eof) {
	list_push(sizeof(Item), &root->items, parse_item(context));
    }

    return root;
}

AstRoot* parser_parse(String source, List token_list) {
    ParserContext context;
    context.source = source;
    context.token_list = token_list;
    context.token_index = 0;

    return parse_root(&context);
}
