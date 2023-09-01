#include "ast.h"

#include "util.h"
#include <stdio.h>

static void print_type(Type* type) {
    printf("~Type~\n");
    switch (type->kind) {
	case TypeKind_Symbol: {
	    printf("kind: symbol\n");
	    printf("name: ");
	    span_println(type->symbol);
	    break;
	}

	case TypeKind_Ptr: {
	    printf("kind: pointer\n");
	    print_type(type->ptr.to);
	    break;
	}
	
	default: {
	    printf("unhandled type\n");
	}
    }
}

static void print_expression(Expr* expression) {
    switch (expression->kind) {
	case ExprKind_Ret: {
	    printf("kind: return\n");
	    print_expression(expression->ret);
	    break;
	}
	
	case ExprKind_NumberLit: {
	    printf("kind: number literal\n");
	    break;
	}

	case ExprKind_BinOp: {
	    printf("kind: binary operator\n");
	    print_expression(expression->binary_operator.left);
	    print_expression(expression->binary_operator.right);
	    break;
	}

	case ExprKind_Let: {
	    printf("kind: let\t");
	    printf("name: ");
	    span_println(expression->let.name);
	    print_type(expression->let.type);
	    print_expression(expression->let.value);
	    break;
	}

	case ExprKind_Symbol: {
	    printf("kind: symbol\t");
	    printf("name: ");
	    span_println(expression->symbol);
	    break;
	}

	case ExprKind_FnCall: {
	    printf("kind: fn call\n");
	    printf("name: ");
	    span_println(expression->fn_call.name);
	    break;
        }

	default: {
	    sil_panic("Unhandled expression %d", expression->kind);
	}
    }
}

static void print_statement(Stmt* statement) {
    switch (statement->kind) {
	case StmtKind_Expr: {
	    printf("~Expression Statement~\n");
	    print_expression(statement->expression);
	    break;
	}
    }
}

static void print_block(Block* block) {
    for (int i = 0; i < block->statements.length; i++) {
	printf(" ");
	Stmt* statement = list_get(block->statements, i);
	print_statement(statement);
    }
}

static void print_fn_declaration(FnDecl* fn_declaration) {
    printf("signature: (");
    for (int i = 0; i < fn_declaration->parameters.length; i++) {
	FnParam* parameter = list_get(fn_declaration->parameters, i);
	span_print(parameter->name);
	printf(": T");
	if (i < fn_declaration->parameters.length - 1) {
	    printf(", ");
	}
    }

    printf(") -> T\n{\n");

    print_block(fn_declaration->body);

    printf("}\n");
}

static void print_item(Item* item) {
    printf("~Item~\n");
    printf("name: ");
    span_println(item->name);

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
