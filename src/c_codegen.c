#include "c_codegen.h"

#include "ast.h"
#include "os.h"
#include <iso646.h>


typedef struct CodegenContext {
    FILE* out_file;
    size_t indent_level;
} CodegenContext;

static void write(CodegenContext* context, Span span) {
    fwrite(span.start, span.length, 1, context->out_file);
}

// using a macro so we can use sizeof on the literal
#define write_literal(context, literal) \
    fwrite(literal, sizeof(literal) - 1, 1, context->out_file)

void write_indent(CodegenContext* context) {
    for (int i = 0; i < context->indent_level; i++) {
	write_literal(context, "    ");
    }
}

static void generate_statement(CodegenContext* context, Stmt* statement);

static void generate_type(CodegenContext* context, Type* type) {
    switch (type->kind) {
	case TypeKind_Never: {
	    write_literal(context, "void");
	    break;
	}

	case TypeKind_Symbol: {
	    write(context, type->symbol);
	    break;
	}

	case TypeKind_Ptr: {
	    generate_type(context, type->ptr.to);
	    write_literal(context, "*");
	    break;
	}

	default: sil_panic("Unhandled silic->c type");
    }
}

static void generate_block(CodegenContext* context, Block* block) {
    write_literal(context, "{\n");
    context->indent_level += 1;

    for (int i = 0; i < block->statements.length; i++) {
	Stmt* statement = dynarray_get(block->statements, i);
	generate_statement(context, statement);
    }

    context->indent_level -= 1;
    write_indent(context);
    write_literal(context, "}");
}

static void generate_expression(CodegenContext* context, Expr* expression) {
    switch (expression->kind) {
	case ExprKind_NumberLit: {
	    write(context, expression->number_literal.span);
	    break;
	}

	case ExprKind_StringLit: {
	    write(context, expression->string_literal.span);
	    break;
	}

	case ExprKind_FnCall: {
	    FnCall* call = expression->fn_call;

	    write(context, call->name);
	    write_literal(context, "(");

	    for (int i = 0; i < call->arguments.length; i++) {
		Expr* arg = dynarray_get(call->arguments, i);
		generate_expression(context, arg);
	    }

	    write_literal(context, ")");
	    
	    break;
	}

	case ExprKind_Ret: {
	    write_literal(context, "return ");
	    generate_expression(context, expression->ret);
	    
	    break;
	}

	case ExprKind_Block: {
	    generate_block(context, expression->block);
	    break;
	}

	case ExprKind_If: {
	    write_literal(context, "if (");
	    generate_expression(context, expression->if_expr->condition);
	    write_literal(context, ") ");
	    generate_block(context, expression->if_expr->then);
	    if (expression->if_expr->otherwise != NULL) {
		write_literal(context, " else ");
		generate_expression(context, expression->if_expr->otherwise);
	    }

	    break;
	}
			
	default: sil_panic("Unhandled expression");
    }
}

static void generate_statement(CodegenContext* context, Stmt* statement) {
    write_indent(context);
    if (statement->kind == StmtKind_Expr) {
	generate_expression(context, statement->expression);
	if (statement->expression->kind != ExprKind_If) {
	    write_literal(context, ";\n");
	} else {
	    write_literal(context, "\n");
	}
    }
}

static void generate_fn_signature(CodegenContext* context, Item* item) {
    FnSig* signature;
    if (item->kind == ItemKind_FnDecl) {
	signature = item->fn_declaration->signature;
    } else if (item->kind == ItemKind_ExternFn) {
	signature = item->extern_fn->signature;
    } else {
	sil_panic("Cannot generate signature for item type %d", item->kind);
    }

    generate_type(context, signature->return_type);

    write_literal(context, " ");

    write(context, item->name);
    write_literal(context, "(");

    for (int i = 0; i < signature->parameters.length; i++) {
	FnParam* parameter = dynarray_get(signature->parameters, i);
	generate_type(context, parameter->type);
	write_literal(context, " ");
	write(context, parameter->name);
    }

    write_literal(context, ")");
}

static void generate_definition(CodegenContext* context, Item* item) {
    switch (item->kind) {
	case ItemKind_FnDecl:
	    generate_fn_signature(context, item);
	    write_literal(context, " ");
	    generate_block(context, item->fn_declaration->body);
	    break;
	
	default: return;
    }
}

static void generate_forward_declaration(CodegenContext* context, Item* item) {
    switch (item->kind) {
	case ItemKind_FnDecl:
	case ItemKind_ExternFn:
	    generate_fn_signature(context, item);
	    write_literal(context, ";\n");
	    break;
    
	default: return;
    }
}

static void generate_ast(CodegenContext* context, AstRoot* ast) {
    for (int i = 0; i < ast->items.length; i++) {
        Item* item = dynarray_get(ast->items, i);
        generate_forward_declaration(context, item);
    }

    write_literal(context, "\n");

    for (int i = 0; i < ast->items.length; i++) {
	Item* item = dynarray_get(ast->items, i);
	generate_definition(context, item);
    }
}

void c_codegen_generate(Module* module) {
    CodegenContext context;
    context.indent_level = 0;

    char* prelude_text;
    int prelude_length;
    Result prelude_result = read_file("prelude.c", &prelude_text, &prelude_length);
    if (prelude_result.type != Ok) {
        printf("Failed to load 'prelude.c'");
        return;
    }

    context.out_file = fopen("build/ir.c", "wb");
    if (context.out_file == NULL) {
	    printf("Could not create ir\n");
	    return;
    }

    fwrite(prelude_text, prelude_length, 1, context.out_file);

    generate_ast(&context, module->ast);

    fclose(context.out_file);

    system("gcc -O1 build/ir.c -o app");
}

