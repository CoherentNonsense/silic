#include "c_codegen.h"

#include "ast.h"
#include "parser.h"
#include "os.h"

#include <chnlib/strbuffer.h>
#include <chnlib/logger.h>
#include <chnlib/map.h>
#include <stdio.h>


typedef struct CodegenContext {
    StrBuffer strbuf;
    Module* module;
    usize indent_level;
} CodegenContext;

static void write(CodegenContext* context, String span) {
    strbuf_print_str(&context->strbuf, span);
}

// using a macro so we can use sizeof on the literal
#define write_literal(context, literal) \
    strbuf_print_lit(&context->strbuf, literal)

void write_indent(CodegenContext* context) {
    for (usize i = 0; i < context->indent_level; i++) {
	write_literal(context, "    ");
    }
}

static void generate_statement(CodegenContext* context, Stmt* statement);
static void generate_expression(CodegenContext* context, Expr* expression);

static void generate_type(CodegenContext* context, TypeEntry* type) {
    switch (type->kind) {
        case TypeEntryKind_Void: { write_literal(context, "void"); break; }
        case TypeEntryKind_Never: { write_literal(context, "void"); break; }
        case TypeEntryKind_Ptr: {
            generate_type(context, type->ptr.to);
            if (not type->ptr.is_mut) { write_literal(context, " const"); }
            write_literal(context, "*");
            break;
        }
        case TypeEntryKind_Bool: { write_literal(context, "bool"); break; }
        case TypeEntryKind_Int: {
            // i'm porting to a string buffer soon i swer
            if (type->bits == 8) {
                write_literal(context, "u8");
            } else if (type->bits == 32) {
                write_literal(context, "i32");
            } else if (type->bits == 64) {
                write_literal(context, "i64");
            } else {
                sil_panic("AAAHHHHH %zu", type->bits);
            }
        }
    }
}

static void generate_type_old(CodegenContext* context, Type* type) {
    switch (type->kind) {
	case TypeKind_Void: {
	    write_literal(context, "void");
	    break;
	}

	case TypeKind_Never: {
	    write_literal(context, "void");
	    break;
	}

	case TypeKind_Symbol: {
	    write(context, type->symbol);
	    break;
	}

	case TypeKind_Ptr: {
	    generate_type_old(context, type->ptr.to);
            if (not type->ptr.is_mut) { write_literal(context, " const"); }
	    write_literal(context, "*");
	    break;
	}

	default: sil_panic("Unhandled silic->c type");
    }
}

static void generate_block(CodegenContext* context, Block* block) {
    write_literal(context, "{\n");
    context->indent_level += 1;

    for (usize i = 0; i < dynarray_len(block->statements); i++) {
	Stmt* statement = block->statements[i];
	generate_statement(context, statement);
    }

    context->indent_level -= 1;
    write_indent(context);
    write_literal(context, "}");
}

static void generate_number_literal(CodegenContext* context, NumberLit* number_literal) {
    write(context, number_literal->span);
}

static void generate_binop(CodegenContext* context, BinOp* binop) {
    switch (binop->kind) {
        case BinOpKind_CmpEq: write_literal(context, "eqi32("); break;
        case BinOpKind_CmpNotEq: write_literal(context, "neqi32("); break;
        case BinOpKind_CmpGt: write_literal(context, "gti32("); break;
        case BinOpKind_CmpLt: write_literal(context, "lti32("); break;
        case BinOpKind_And: write_literal(context, "and("); break;
        case BinOpKind_Or: write_literal(context, "or("); break;
        case BinOpKind_Assign: write_literal(context, "wri32(&"); break;
	case BinOpKind_Add: write_literal(context, "addi32("); break;
	case BinOpKind_Sub: write_literal(context, "subi32("); break;
	case BinOpKind_Mul: write_literal(context, "muli32("); break;
	case BinOpKind_Div: write_literal(context, "divi32("); break;
        default: sil_panic("Codegen error: Unhandled binary operator %d", binop->kind);
    }

    generate_expression(context, binop->left);
    write_literal(context, ", ");
    generate_expression(context, binop->right);
    write_literal(context, ")");
}

static void generate_asm(CodegenContext* context, Asm* asm) {
    // HACK: this wholbe backend is really all a hack tbh
    static char tmp_var[] = "expr_val0";
    for (usize i = 0; i < dynarray_len(asm->outputs); i += 1) {
        write_literal(context, "ssize_t ");
        write_literal(context, tmp_var);
        write_literal(context, ";\n");
    }

    write_indent(context);
    write_literal(context, "__asm__ volatile (");
    for (usize i = 0; i < dynarray_len(asm->source); i += 1) {
        write(context, asm->source[i].span);
    }

    write_literal(context, ":");

    for (usize i = 0; i < dynarray_len(asm->outputs); i += 1) {
        write_literal(context, "\"=");
        write(context, asm->outputs[i]);
        write_literal(context, "\"(");
        write_literal(context, tmp_var);
        write_literal(context, "):");
    }

    for (usize i = 0; i < dynarray_len(asm->inputs); i += 1) {
        if (i > 0) {
            write_literal(context, ",");
        }
        write_literal(context, "\"");
        write(context, asm->inputs[i].reg);
        write_literal(context, "\"(");
        generate_expression(context, asm->inputs[i].val);
        write_literal(context, ")");
    }

    write_literal(context, ":");

    for (usize i = 0; i < dynarray_len(asm->clobbers); i += 1) {
        if (i > 0) {
            write_literal(context, ",");
        }
        write_literal(context, "\"");
        write(context, asm->clobbers[i]);
        write_literal(context, "\"");
    }

    write_literal(context, ");");

    tmp_var[8] += 1;
}

static void generate_expression(CodegenContext* context, Expr* expression) {
    switch (expression->kind) {
	case ExprKind_NumberLit: {
	    generate_number_literal(context, expression->number_literal);
	    break;
	}

	case ExprKind_StringLit: {
	    write(context, expression->string_literal.span);
	    break;
	}

        case ExprKind_BoolLit: {
            expression->boolean ? write_literal(context, "true") : write_literal(context, "false");
            break;
        }

	case ExprKind_Symbol: {
            write_literal(context, "var_");
	    write(context, expression->symbol);
	    break;
	}

	case ExprKind_FnCall: {
	    FnCall* call = expression->fn_call;

	    write(context, call->name);
	    write_literal(context, "(");

	    for (usize i = 0; i < dynarray_len(call->arguments); i++) {
		Expr* arg = call->arguments[i];
		generate_expression(context, arg);
		if (i < dynarray_len(call->arguments) - 1) {
		    write_literal(context, ", ");
		}
	    }

	    write_literal(context, ")");
	    
	    break;
	}

	case ExprKind_Let: {
            generate_type(context, expression->codegen.type);

	    write_literal(context, " ");
            write_literal(context, "var_");
	    write(context, expression->let->name);
	    write_literal(context, " = ");
	    generate_expression(context, expression->let->value);

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

	case ExprKind_BinOp: generate_binop(context, expression->binary_operator); break;

	case ExprKind_If: {
	    write_literal(context, "if (");
	    generate_expression(context, expression->if_expr->condition);
	    write_literal(context, ") ");
	    generate_block(context, expression->if_expr->then->block);
	    if (expression->if_expr->otherwise != null) {
		write_literal(context, " else ");
		generate_expression(context, expression->if_expr->otherwise);
	    }

	    break;
	}

	case ExprKind_Match: {
	    Match* match = expression->match;
	    write_literal(context, "switch (");
	    generate_expression(context, expression->match->condition);
	    write_literal(context, ") {\n");
	    context->indent_level += 1;
	    for (usize i = 0; i < dynarray_len(match->arms); i++) {
		MatchArm* arm = match->arms[i];
		write_indent(context);
		write_literal(context, "case ");
		generate_number_literal(context, arm->pattern);
		write_literal(context, ": ");
		generate_expression(context, arm->then);
		if (arm->then->kind != ExprKind_Block) {
		    write_literal(context, "; break;");
		} else {
		    write_literal(context, " break;");
		}
		write_literal(context, "\n");
	    }
	    context->indent_level -= 1;
	    write_indent(context);
	    write_literal(context, "}");
	    break;
	}

        case ExprKind_Loop: {
            write_literal(context, "while (true) ");
            generate_expression(context, expression->loop->body);
            break;
        }

        case ExprKind_Break: { write_literal(context, "break"); break; }
        case ExprKind_Continue: { write_literal(context, "continue"); break; }

        case ExprKind_Asm: {
            generate_asm(context, expression->asm);
            break;
        }

        case ExprKind_Cast: {
            write_literal(context, "(");
            generate_type_old(context, expression->cast->to);
            write_literal(context, ")");
            generate_expression(context, expression->cast->expr);
            break;
        }

        default: sil_panic("Codegen Error: Unhandled expression %d", expression->kind);
    }
}

static void generate_statement(CodegenContext* context, Stmt* statement) {
    write_indent(context);
    if (statement->kind == StmtKind_Expr) {
	generate_expression(context, statement->expression);
	if (!parser_should_remove_statement_semicolon(statement->expression)) {
	    write_literal(context, ";\n");
	} else {
	    write_literal(context, "\n");
	}
    }
}

static void generate_fn_signature(CodegenContext* context, Item* item) {
    FnSig* signature;
    if (item->kind == ItemKind_FnDef) {
	signature = item->fn_definition->signature;
    } else if (item->kind == ItemKind_ExternFn) {
	signature = item->extern_fn->signature;
    } else {
	sil_panic("Cannot generate signature for item type %d", item->kind);
    }

    generate_type_old(context, signature->return_type);

    write_literal(context, " ");

    write(context, item->name);
    write_literal(context, "(");

    // void as empty parameters
    if (dynarray_len(signature->parameters) == 0) {
        write_literal(context, "void");
    }
    for (usize i = 0; i < dynarray_len(signature->parameters); i++) {
	FnParam* parameter = signature->parameters[i];
	generate_type_old(context, parameter->type);
	write_literal(context, " const ");
        write_literal(context, "var_");
	write(context, parameter->name);
	if (i < dynarray_len(signature->parameters) - 1) {
	    write_literal(context, ", ");
	}
    }

    write_literal(context, ")");
}

static void generate_definition(CodegenContext* context, Item* item) {
    switch (item->kind) {
	case ItemKind_FnDef:
	    if (!item->visibility.is_pub) {
		write_literal(context, "static ");
	    }
	    generate_fn_signature(context, item);
	    write_literal(context, " ");
	    generate_block(context, item->fn_definition->body->block);
	    write_literal(context, "\n\n");
	    break;
	
	default: return;
    }
}

static void generate_forward_declarations(CodegenContext* context) {
    MapIter iter = map_iter(context->module->items);
    
    while (map_next(context->module->items, iter)) {
        Item* item = map_iter_val(context->module->items, iter);

        if (item->kind != ItemKind_ExternFn and !item->visibility.is_pub) {
            write_literal(context, "static ");
        }
	generate_fn_signature(context, item);
	write_literal(context, ";\n");
    };
}

static void generate_ast(CodegenContext* context, AstRoot* ast) {
    generate_forward_declarations(context);

    write_literal(context, "\n");

    for (usize i = 0; i < dynarray_len(ast->items); i++) {
	Item* item = ast->items[i];
	generate_definition(context, item);
    }
}

String c_codegen_generate(Module* module) {
    CodegenContext context;
    context.indent_level = 0;
    context.module = module;
    context.strbuf = strbuf_init();

    generate_ast(&context, module->ast);

    return strbuf_to_string(&context.strbuf);
}

