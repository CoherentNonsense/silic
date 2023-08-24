#include "ast.h"

#include <stdio.h>

static void print_item(Item* item) {
    printf("~Item~\n");
    printf("type: function declaration\n");
    printf("name: %s\n", item->name.data);

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
