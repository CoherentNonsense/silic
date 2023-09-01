#include "ast.h"

#include "util.h"
#include <stdio.h>

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */


static void print_type(Type* type) {
    switch (type->kind) {
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
	    print_expression(expression->binary_operator.left);
	    print_expression(expression->binary_operator.right);
	    break;
	}

	case ExprKind_Let: {
	    printf("let\t" RESET);
	    printf("name: ");
	    span_print(expression->let.name);
	    printf("\ttype: ");
	    print_type(expression->let.type);
	    printf("\n");
	    print_expression(expression->let.value);
	    break;
	}

	case ExprKind_Symbol: {
	    printf("symbol\t" RESET);
	    printf("name: ");
	    span_println(expression->symbol);
	    break;
	}

	case ExprKind_FnCall: {
	    printf("fn call\t" RESET);
	    printf("name: ");
	    span_println(expression->fn_call.name);
	    printf("args: (\n");
	    for (int i = 0; i < expression->fn_call.arguments.length; i++) {
		print_expression(list_get(expression->fn_call.arguments, i));
		if (i < expression->fn_call.arguments.length - 1) {
		    printf(",\n");
		}
	    }
	    printf(")\n");
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
	    printf("\n");
	    break;
	}
    }
}

static void print_block(Block* block) {
    for (int i = 0; i < block->statements.length; i++) {
	Stmt* statement = list_get(block->statements, i);
	print_statement(statement);
    }
}

static void print_fn_signature(FnSig* fn_sig) {
    printf("signature: (");
    for (int i = 0; i < fn_sig->parameters.length; i++) {
	FnParam* parameter = list_get(fn_sig->parameters, i);
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

static void print_fn_declaration(FnDecl* fn_decl) {
    print_fn_signature(fn_decl->signature);

    printf("body: {\n\n");
    print_block(fn_decl->body);
    printf("}");
}

static void print_item(Item* item) {
    printf(BOLDMAGENTA "~ Item " RESET);

    switch (item->kind) {
	case ItemKind_FnDecl: {
	    printf(YELLOW "function declaration\n" RESET "name: ");
	    span_println(item->name);
	    print_fn_declaration(item->fn_declaration);
	    break;
	}

	case ItemKind_ExternFn: {
	    printf(YELLOW "extern function\n" RESET "name: ");
	    span_println(item->name);
	    print_fn_signature(item->extern_fn->signature);
	    break;
	}
    }

    printf("\n");
}

void ast_print(AstRoot* root) {
    printf("\n" BOLDWHITE "- AST View" RESET "\n");

    for (int i = 0; i < root->items.length; i += 1) {
	Item* item = list_get(root->items, i);
	print_item(item);
    }

    printf("\n");
}
