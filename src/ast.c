#include "ast.h"

#include <stdio.h>

static void print_statement(Stmt* statement) {
    printf("~Statement~\n");
}

static void print_block(Block* block) {
    printf("~Block~\n");
    for (int i = 0; i < block->statements.length; i++) {
	Stmt** statement = list_get(sizeof(Stmt*), &block->statements, i);
	print_statement(*statement);
    }
}

static void print_fn_declaration(FnDecl* fn_declaration) {
    printf("signature: () -> void\n");
    print_block(fn_declaration->body);
}

static void print_item(Item* item) {
    printf("~Item~\n");
    printf("name: %s\n", item->name.data);

    switch (item->kind) {
	case ItemKind_FnDecl: {
	    printf("type: function declaration\n");
	    print_fn_declaration(item->fn_declaration);
	}
    }

    FnDecl* fn_declaration = item->fn_declaration;
}

void ast_print(AstRoot* root) {
    printf("\n__AST__\n\n");

    for (int i = 0; i < root->items.length; i += 1) {
	Item** item = list_get(sizeof(Item*), &root->items, i);
	print_item(*item);
    }

    printf("\n");
}
