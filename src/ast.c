#include "ast.h"

#include "util.h"
#include <stdio.h>

static void print_expression(Expr* expression) {
    printf("~Expression~\n");
    switch (expression->kind) {
	case ExprKind_Ret: {
	    printf("type: return\n");
	    print_expression(expression->ret);
	    break;
	}
	
	case ExprKind_NumberLit: {
	    printf("type: number literal\n");
	    break;
	}

	case ExprKind_BinOp: {
	    printf("type: binary operator\n");
	    print_expression(expression->binary_operator.left);
	    print_expression(expression->binary_operator.right);
	    break;
	}

	case ExprKind_Let: {
	    printf("type: let\n");
	    printf("name: %s\n", expression->let.name.data);
	    print_expression(expression->let.value);
	    break;
	}

	default: {
	    sil_panic("Unhandled expression %d", expression->kind);
	}
    }
}

static void print_statement(Stmt* statement) {
    printf("~Statement~\n");
    switch (statement->kind) {
	case StmtKind_Expr: {
	    print_expression(statement->expression);
	    break;
	}
    }
}

static void print_block(Block* block) {
    printf("~Block~\n");
    for (int i = 0; i < block->statements.length; i++) {
	Stmt* statement = list_get(block->statements, i);
	print_statement(statement);
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
	Item* item = list_get(root->items, i);
	print_item(item);
    }

    printf("\n");
}
