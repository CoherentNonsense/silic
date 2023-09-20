#include "ast.h"

#include "util.h"
#include <stdio.h>

static void print_statement(Stmt* statement);

static void print_type(Type* type) {
    switch (type->kind) {
	case TypeKind_Void: {
	    printf(BOLDGREEN "void" RESET);
	    break;
	}

	case TypeKind_Symbol: {
	    printf(BOLDGREEN);
	    span_print(type->symbol);
	    printf(RESET);
	    break;
	}

	case TypeKind_Ptr: {
	    printf(BOLDGREEN "*");
	    print_type(type->ptr.to);
	    break;
	}

	case TypeKind_Never: {
	    printf(BOLDGREEN "!" RESET);
	    break;
	}
	
	default: {
	    sil_panic("unhandled type %d", type->kind);
	}
    }
}

static void print_block(Block* block) {
    printf("{\n\n");
    for (int i = 0; i < block->statements.length; i++) {
	Stmt* statement = dynarray_get(block->statements, i);
	print_statement(statement);
    }
    printf("}\n");
}

static void print_expression(Expr* expression) {
    printf("expr: " YELLOW);
    switch (expression->kind) {
	case ExprKind_Ret: {
	    printf("return\n" RESET);
	    print_expression(expression->ret);
	    break;
	}
	
	case ExprKind_NumberLit: {
	    printf("number literal\n" RESET);
	    break;
	}

	case ExprKind_StringLit: {
	    printf("string literal\n" RESET);
	    break;
	}

	case ExprKind_BinOp: {
	    printf("binop\n" RESET);
	    print_expression(expression->binary_operator->left);
	    print_expression(expression->binary_operator->right);
	    break;
	}

	case ExprKind_Let: {
	    printf("let\t" RESET "name: ");
	    span_print(expression->let->name);
	    printf("\ttype: ");
	    print_type(expression->let->type);
	    printf("\n");
	    print_expression(expression->let->value);
	    break;
	}

	case ExprKind_Match: {
	    printf("match\n" RESET);
	    break;
	}

	case ExprKind_Symbol: {
	    printf("symbol\t" RESET "name: ");
	    span_println(expression->symbol);
	    break;
	}

	case ExprKind_FnCall: {
	    printf("fn call\t" RESET "name: ");
	    span_println(expression->fn_call->name);
	    printf("args: (\n");
	    for (int i = 0; i < expression->fn_call->arguments.length; i++) {
		print_expression(dynarray_get(expression->fn_call->arguments, i));
		if (i < expression->fn_call->arguments.length - 1) {
		    printf(",\n");
		}
	    }
	    printf(")\n");
	    break;
        }

	case ExprKind_Block: {
	    print_block(expression->block);
	    break;
	}

	case ExprKind_If: {
	    printf("if\n" RESET "condition: (\n");
	    print_expression(expression->if_expr->condition);
	    printf(")\nthen: ");
	    print_block(expression->if_expr->then);
	    if (expression->if_expr->otherwise.type == No) {
		break;
	    }
	    printf("else: ");
	    print_expression(expression->if_expr->otherwise.value);
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
	    printf(BOLDWHITE "Expression Statement\n" RESET);
	    print_expression(statement->expression);
	    break;
	}
    }
}

static void print_fn_signature(FnSig* fn_sig) {
    printf("signature: (");
    for (int i = 0; i < fn_sig->parameters.length; i++) {
	FnParam* parameter = dynarray_get(fn_sig->parameters, i);
	span_print(parameter->name);
	printf(": ");
	print_type(parameter->type);
	if (i < fn_sig->parameters.length - 1) {
	    printf(", ");
	}
    }

    printf(") -> ");
    print_type(fn_sig->return_type);
    printf("\n");
}

static void print_fn_definition(FnDef* fn_decl) {
    print_fn_signature(fn_decl->signature);

    printf("body: ");
    print_block(fn_decl->body);
}

static void print_item(Item* item) {
    printf(BOLDMAGENTA "~ Item " RESET);

    switch (item->kind) {
	case ItemKind_FnDef: {
	    printf(YELLOW "function declaration\n" RESET "name: ");
	    span_println(item->name);
	    print_fn_definition(item->fn_definition);
	    break;
	}

	case ItemKind_ExternFn: {
	    printf(YELLOW "extern function\n" RESET "name: ");
	    span_println(item->name);
	    print_fn_signature(item->extern_fn->signature);
	    break;
	}

	case ItemKind_StructDef: {
	    break;
	}
    }

    printf("\n");
}

void ast_print(AstRoot* root) {

    for (int i = 0; i < root->items.length; i += 1) {
	Item* item = dynarray_get(root->items, i);
	print_item(item);
    }
}
