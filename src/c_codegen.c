#include "c_codegen.h"

#include "ast.h"
#include "parser.h"
#include "os.h"

#include <chnlib/strbuffer.h>
#include <chnlib/str.h>
#include <chnlib/logger.h>
#include <chnlib/map.h>
#include <stdio.h>


typedef struct CodegenContext {
    StrBuffer strbuf;
    Module* module;
    usize indent_level;
} CodegenContext;


void write_indent(CodegenContext* context) {
    for (usize i = 0; i < context->indent_level; i++) {
	strbuf_print_lit(&context->strbuf, "    ");
    }
}

void write_newline(CodegenContext* context) {
    strbuf_print_lit(&context->strbuf, "\n");
    write_indent(context);
}

// TODO: this feels hacky
static String new_tmp_var(void) {
    static usize tmp_var_counter = 0;

    StrBuffer buf = strbuf_init();
    strbuf_printf(&buf, "__sil__tmp_var_%zu", tmp_var_counter++);

    return strbuf_to_string(&buf);
}

static void generate_statement(CodegenContext* context, Stmt* statement);
static void generate_expression(CodegenContext* context, Expr* expression);
static void generate_expression_with_block(CodegenContext* context, Expr* expression, String* bind);

static void generate_type(CodegenContext* context, type_id type) {
    TypeEntry* type_entry = &context->module->type_table.types[type];
    switch (type_entry->kind) {
        case TypeEntryKind_Invalid: { sil_panic("generating invalid type"); }
        case TypeEntryKind_Void: { strbuf_print_lit(&context->strbuf, "void"); break; }
        case TypeEntryKind_Never: { strbuf_print_lit(&context->strbuf, "void /* never */"); break; }
        case TypeEntryKind_Ptr: {
            generate_type(context, type_entry->ptr.to);
            strbuf_printf(&context->strbuf, "%s*", type_entry->ptr.is_mut ? "" : " const");
            break;
        }
        case TypeEntryKind_Bool: { strbuf_print_lit(&context->strbuf, "bool"); break; }
        case TypeEntryKind_Int: {
            strbuf_printf(&context->strbuf, "%c%zu", type_entry->integral.is_signed ? 'i' : 'u', type_entry->bits);
            break;
        }
        case TypeEntryKind_Size: {
            strbuf_printf(&context->strbuf, "%csize", type_entry->integral.is_signed ? 'i' : 'u');
            break;
        }
    }
}

static void generate_type_old(CodegenContext* context, Ast_Type* type) {
    switch (type->kind) {
        case TypeKind_Void: {
            strbuf_print_lit(&context->strbuf, "void");
                break;
        }

        case TypeKind_Never: {
            strbuf_print_lit(&context->strbuf, "void");
            break;
        }

        case TypeKind_Symbol: {
            strbuf_print_str(&context->strbuf, type->symbol);
            break;
        }

        case TypeKind_Ptr: {
            generate_type_old(context, type->ptr.to);
            strbuf_printf(&context->strbuf, "%s*", type->ptr.is_mut ? "" : " const");
            break;
        }

        default: sil_panic("Unhandled silic->c type");
    }
}

static void generate_block(CodegenContext* context, Expr* block_expr, String* bind) {
    Block* block = block_expr->block;

    strbuf_print_lit(&context->strbuf, "{\n");
    context->indent_level += 1;

    for (usize i = 0; i < dynarray_len(block->statements) - 1; i++) {
        Stmt* statement = block->statements[i];
        write_indent(context);
        generate_statement(context, statement);
    }

    Stmt* last_stmt = block->statements[dynarray_len(block->statements) - 1];
    if (block_expr->codegen.type != TypeEntryKind_Void) {
        write_indent(context);
        if (last_stmt->kind == StmtKind_NakedExpr) {
            // naked expressions don't have blocks
            if (bind == null) {
                strbuf_print_lit(&context->strbuf, "return ");
                generate_expression(context, last_stmt->expression);
                strbuf_print_lit(&context->strbuf, ";\n");
            } else {
                strbuf_printf(&context->strbuf, "%.*s = ", str_format((*bind)));
                generate_expression(context, last_stmt->expression);
                strbuf_print_lit(&context->strbuf, ";\n");
            }

        } else if (should_remove_statement_semi(last_stmt->expression)) {
            // block expression
            String tmp_eval = new_tmp_var();
            generate_type(context, block_expr->codegen.type);
            strbuf_printf(&context->strbuf, " %.*s;", str_format(tmp_eval));
            write_newline(context);
            generate_expression_with_block(context, last_stmt->expression, &tmp_eval);
            write_newline(context);

            if (bind != null) {
                strbuf_printf(&context->strbuf, "%.*s = %.*s;\n", str_format((*bind)), str_format(tmp_eval));
            } else {
                strbuf_printf(&context->strbuf, "return %.*s;\n", str_format(tmp_eval));
            }

            str_deinit(tmp_eval);

        } else {
            // plain statement
            generate_statement(context, last_stmt);
        }
    }

    context->indent_level -= 1;
    write_indent(context);
    strbuf_print_lit(&context->strbuf, "}");
}

static void generate_number_literal(CodegenContext* context, NumberLit* number_literal) {
    strbuf_print_str(&context->strbuf, number_literal->span);
}

static void generate_binop(CodegenContext* context, BinOp* binop) {
    switch (binop->kind) {
        case BinOpKind_CmpEq: strbuf_print_lit(&context->strbuf, "eqi32("); break;
        case BinOpKind_CmpNotEq: strbuf_print_lit(&context->strbuf, "neqi32("); break;
        case BinOpKind_CmpGt: strbuf_print_lit(&context->strbuf, "gti32("); break;
        case BinOpKind_CmpLt: strbuf_print_lit(&context->strbuf, "lti32("); break;
        case BinOpKind_And: strbuf_print_lit(&context->strbuf, "and("); break;
        case BinOpKind_Or: strbuf_print_lit(&context->strbuf, "or("); break;
        case BinOpKind_Assign: strbuf_print_lit(&context->strbuf, "wri32(&"); break;
	case BinOpKind_Add: strbuf_print_lit(&context->strbuf, "addi32("); break;
	case BinOpKind_Sub: strbuf_print_lit(&context->strbuf, "subi32("); break;
	case BinOpKind_Mul: strbuf_print_lit(&context->strbuf, "muli32("); break;
	case BinOpKind_Div: strbuf_print_lit(&context->strbuf, "divi32("); break;
        default: sil_panic("Codegen error: Unhandled binary operator %d", binop->kind);
    }

    generate_expression(context, binop->left);
    strbuf_print_lit(&context->strbuf, ", ");
    generate_expression(context, binop->right);
    strbuf_print_lit(&context->strbuf, ")");
}

static void generate_asm(CodegenContext* context, Asm* asm, String* output) {
    String tmp_output = new_tmp_var();
    for (usize i = 0; i < dynarray_len(asm->outputs); i += 1) {
        strbuf_printf(&context->strbuf, "isize %.*s;\n", str_format(tmp_output));
    }

    write_indent(context);
    strbuf_print_lit(&context->strbuf, "__asm__ volatile (");
    for (usize i = 0; i < dynarray_len(asm->source); i += 1) {
        strbuf_print_str(&context->strbuf, asm->source[i].span);
    }

    strbuf_print_lit(&context->strbuf, ":");

    // HACK: only one output rn
    strbuf_printf(&context->strbuf, "\"=%.*s\"(%.*s):", str_format(asm->outputs[0]), str_format(tmp_output));

    for (usize i = 0; i < dynarray_len(asm->inputs); i += 1) {
        if (i > 0) { strbuf_print_lit(&context->strbuf, ","); }

        strbuf_printf(&context->strbuf, "\"%.*s\"(", str_format(asm->inputs[i].reg));
        generate_expression(context, asm->inputs[i].val);
        strbuf_print_lit(&context->strbuf, ")");
    }

    strbuf_print_lit(&context->strbuf, ":");

    for (usize i = 0; i < dynarray_len(asm->clobbers); i += 1) {
        if (i > 0) { strbuf_print_lit(&context->strbuf, ","); }
        strbuf_printf(&context->strbuf, "\"%.*s\"", str_format(asm->clobbers[i]));
    }

    strbuf_print_lit(&context->strbuf, ");");

    if (output != null) {
        strbuf_print_lit(&context->strbuf, "\n");
        write_indent(context);
        strbuf_printf(&context->strbuf, "%.*s = %.*s;", str_format((*output)), str_format(tmp_output));
    }
}

static void generate_expression_with_block(CodegenContext* context, Expr* expression, String* bind) {
    switch (expression->kind) {
	case ExprKind_NumberLit: {
	    generate_number_literal(context, expression->number_literal);
	    break;
	}

	case ExprKind_StringLit: {
	    strbuf_print_str(&context->strbuf, expression->string_literal.span);
	    break;
	}

        case ExprKind_BoolLit: {
            strbuf_printf(&context->strbuf, "%s", expression->boolean ? "true" : "false");
            break;
        }

	case ExprKind_Symbol: {
            strbuf_printf(&context->strbuf, "%.*s", str_format(expression->symbol));
	    break;
	}

	case ExprKind_FnCall: {
	    FnCall* call = expression->fn_call;

	    strbuf_printf(&context->strbuf, "%.*s(", str_format(call->name));

	    for (usize i = 0; i < dynarray_len(call->arguments); i++) {
                if (i > 0) { strbuf_print_lit(&context->strbuf, ","); }

		Expr* arg = call->arguments[i];
		generate_expression(context, arg);
	    }

	    strbuf_print_lit(&context->strbuf, ")");
	    
	    break;
	}

	case ExprKind_Let: {
            generate_type(context, expression->codegen.type);

            if (should_remove_statement_semi(expression->let->value)) {
                strbuf_printf(&context->strbuf, " %.*s;\n", str_format(expression->let->name));
                write_indent(context);
                generate_expression_with_block(context, expression->let->value, &expression->let->name);

                break;
            }

	    strbuf_printf(&context->strbuf, " %.*s = ", str_format(expression->let->name));
	    generate_expression(context, expression->let->value);

	    break;
	}

	case ExprKind_Ret: {
	    strbuf_print_lit(&context->strbuf, "return ");
	    generate_expression(context, expression->ret);
	    
	    break;
	}

	case ExprKind_Block: {
	    generate_block(context, expression, bind);
	    break;
	}

        case ExprKind_BinOp: {
            generate_binop(context, expression->binary_operator);
            break;
        }

	case ExprKind_If: {
	    strbuf_print_lit(&context->strbuf, "if (");
	    generate_expression(context, expression->if_expr->condition);
	    strbuf_print_lit(&context->strbuf, ") ");
	    generate_expression_with_block(context, expression->if_expr->then, bind);
	    if (expression->if_expr->otherwise != null) {
		strbuf_print_lit(&context->strbuf, " else ");
		generate_expression_with_block(context, expression->if_expr->otherwise, bind);
	    }

	    break;
	}

	case ExprKind_Match: {
	    Match* match = expression->match;
	    strbuf_print_lit(&context->strbuf, "switch (");
	    generate_expression_with_block(context, expression->match->condition, bind);
	    strbuf_print_lit(&context->strbuf, ") {\n");
	    context->indent_level += 1;
	    for (usize i = 0; i < dynarray_len(match->arms); i++) {
		MatchArm* arm = match->arms[i];
		write_indent(context);
		strbuf_print_lit(&context->strbuf, "case ");
		generate_number_literal(context, arm->pattern);
		strbuf_print_lit(&context->strbuf, ": ");
		generate_expression_with_block(context, arm->then, bind);
		if (arm->then->kind != ExprKind_Block) {
		    strbuf_print_lit(&context->strbuf, "; break;");
		} else {
		    strbuf_print_lit(&context->strbuf, " break;");
		}
		strbuf_print_lit(&context->strbuf, "\n");
	    }
	    context->indent_level -= 1;
	    write_indent(context);
	    strbuf_print_lit(&context->strbuf, "}");
	    break;
	}

        case ExprKind_Loop: {
            strbuf_print_lit(&context->strbuf, "while (true) ");
            generate_expression_with_block(context, expression->loop->body, bind);
            break;
        }

        case ExprKind_Break: { strbuf_print_lit(&context->strbuf, "break"); break; }
        case ExprKind_Continue: { strbuf_print_lit(&context->strbuf, "continue"); break; }
        case ExprKind_Unreachable: { strbuf_print_lit(&context->strbuf, "/* unreachable */"); break; }

        case ExprKind_Asm: {
            generate_asm(context, expression->asm, bind);
            break;
        }

        case ExprKind_Cast: {
            strbuf_print_lit(&context->strbuf, "(");
            generate_type_old(context, expression->cast->to);
            strbuf_print_lit(&context->strbuf, ")");
            generate_expression(context, expression->cast->expr);
            break;
        }

        default: sil_panic("Codegen Error: Unhandled expression %d", expression->kind);
    }
}

static void generate_expression(CodegenContext* context, Expr* expression) {
    generate_expression_with_block(context, expression, null);
}

static void generate_statement(CodegenContext* context, Stmt* statement) {
    switch (statement->kind) {
        case StmtKind_Expr:
        case StmtKind_NakedExpr:
            generate_expression(context, statement->expression);
            strbuf_printf(&context->strbuf, "%s\n", should_remove_statement_semi(statement->expression) ? "" : ";");
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

    strbuf_printf(&context->strbuf, " %.*s(", str_format(item->name));

    // void as empty parameters
    if (dynarray_len(signature->parameters) == 0) {
        strbuf_print_lit(&context->strbuf, "void");
    }
    for (usize i = 0; i < dynarray_len(signature->parameters); i++) {
        if (i > 0) { strbuf_print_lit(&context->strbuf, ", "); }
	FnParam* parameter = signature->parameters[i];
	generate_type_old(context, parameter->type);
	strbuf_printf(&context->strbuf, " const %.*s", str_format(parameter->name));
    }

    strbuf_print_lit(&context->strbuf, ")");
}

static void generate_definition(CodegenContext* context, Item* item) {
    switch (item->kind) {
	case ItemKind_FnDef:
	    if (!item->visibility.is_pub) {
		strbuf_print_lit(&context->strbuf, "static ");
	    }
	    generate_fn_signature(context, item);
	    strbuf_print_lit(&context->strbuf, " ");
	    generate_block(context, item->fn_definition->body, null);
	    strbuf_print_lit(&context->strbuf, "\n\n");
	    break;
	
	default: return;
    }
}

static void generate_forward_declarations(CodegenContext* context) {
    MapIter iter = map_iter(context->module->items);
    
    while (map_next(context->module->items, iter)) {
        Item* item = map_iter_val(context->module->items, iter);

        if (item->kind != ItemKind_ExternFn and !item->visibility.is_pub) {
            strbuf_print_lit(&context->strbuf, "static ");
        }
	generate_fn_signature(context, item);
	strbuf_print_lit(&context->strbuf, ";\n");
    };
}

static void generate_ast(CodegenContext* context, AstRoot* ast) {
    generate_forward_declarations(context);

    strbuf_print_lit(&context->strbuf, "\n");

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

