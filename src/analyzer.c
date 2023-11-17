#include "analyzer.h"

#include "ast.h"
#include "typetable.h"
#include <chnlib/logger.h>
#include <chnlib/maybe.h>
#include <stdbool.h>
#include <iso646.h>

static bool analyze_statement(Module*, Stmt*);
static type_id analyze_expression(Module*, Expr*);

static void setup_primitive_types(Module* module) {

    // void
    module->primitives.entry_void = typetable_new_type(&module->type_table, TypeEntryKind_Void, 0);
    map_insert(module->types, str_from_lit("void"), &module->primitives.entry_void);

    // never
    module->primitives.entry_never = typetable_new_type(&module->type_table, TypeEntryKind_Never, 0);

    // char
    module->primitives.entry_c_char = typetable_new_int(&module->type_table, 8, false);
    map_insert(module->types, str_from_lit("c_char"), &module->primitives.entry_c_char);

    // bool
    module->primitives.entry_bool = typetable_new_type(&module->type_table, TypeEntryKind_Bool, 8);
    map_insert(module->types, str_from_lit("bool"), &module->primitives.entry_bool);

    // usize
    module->primitives.entry_usize = typetable_new_size(&module->type_table, false);
    map_insert(module->types, str_from_lit("usize"), &module->primitives.entry_usize);

    // isize
    module->primitives.entry_isize = typetable_new_size(&module->type_table, true);
    map_insert(module->types, str_from_lit("isize"), &module->primitives.entry_isize);

    // unsigned int
    module->primitives.entry_u8 = typetable_new_int(&module->type_table, 8, false);
    module->primitives.entry_u16 = typetable_new_int(&module->type_table, 16, false);
    module->primitives.entry_u32 = typetable_new_int(&module->type_table, 32, false);
    module->primitives.entry_u64 = typetable_new_int(&module->type_table, 64, false);

    map_insert(module->types, str_from_lit("u8"), &module->primitives.entry_u8);
    map_insert(module->types, str_from_lit("u16"), &module->primitives.entry_u16);
    map_insert(module->types, str_from_lit("u32"), &module->primitives.entry_u32);
    map_insert(module->types, str_from_lit("u64"), &module->primitives.entry_u64);

    // c str
    module->primitives.entry_c_str = typetable_new_ptr(
        &module->type_table,
        module->primitives.entry_u8,
        false
    );

    // signed int
    module->primitives.entry_i8 = typetable_new_int(&module->type_table, 8, true);
    module->primitives.entry_i16 = typetable_new_int(&module->type_table, 16, true);
    module->primitives.entry_i32 = typetable_new_int(&module->type_table, 32, true);
    module->primitives.entry_i64 = typetable_new_int(&module->type_table, 64, true);

    map_insert(module->types, str_from_lit("i8"), &module->primitives.entry_i8);
    map_insert(module->types, str_from_lit("i16"), &module->primitives.entry_i16);
    map_insert(module->types, str_from_lit("i32"), &module->primitives.entry_i32);
    map_insert(module->types, str_from_lit("i64"), &module->primitives.entry_i64);
}

static type_id resolve_type(Module* module, Ast_Type* type) {
    if (type == null) { return 0; }
    switch (type->kind) {
        case TypeKind_Void: return module->primitives.entry_void;
        case TypeKind_Never: return module->primitives.entry_never;

        case TypeKind_Symbol: {
            Maybe(type_id) entry = map_get(module->types, type->symbol);
            if (entry == None) {
                sil_panic("Codegen Error: unhandled type");
            }
            return unwrap(entry);
        }

        case TypeKind_Ptr: {
            type_id child = resolve_type(module, type->ptr.to);
            TypeEntry* child_entry = &module->type_table.types[child];
            type_id existing = type->ptr.is_mut ? child_entry->parent_ptr_mut : child_entry->parent_ptr;
            if (existing != 0) {
                return existing;
            }

            type_id new = typetable_new_ptr(&module->type_table, child, type->ptr.is_mut);
            return new;
        }

        default: sil_panic("cannot resolve type %d", type->kind);
    }
}

static type_id analyze_bin_op(Module* module, BinOp* bin_op) {
    switch (bin_op->kind) {
        case BinOpKind_Assign: return module->primitives.entry_void;

        case BinOpKind_Add:
        case BinOpKind_Sub:
        case BinOpKind_Mul:
        case BinOpKind_Div: {
	    type_id left_type = analyze_expression(module, bin_op->left);
	    type_id right_type = analyze_expression(module, bin_op->right);

            if (left_type != right_type) {
                sil_panic("binop: incompatable types");
            }

            return left_type;
        }

        case BinOpKind_And:
        case BinOpKind_Or: {
            type_id left_type = analyze_expression(module, bin_op->left);
            type_id right_type = analyze_expression(module, bin_op->right);

            TypeEntry* left_type_entry = &module->type_table.types[left_type];
            TypeEntry* right_type_entry = &module->type_table.types[right_type];
    
            if (left_type_entry->kind != TypeEntryKind_Bool or right_type_entry->kind != TypeEntryKind_Bool) {
                sil_panic("and/or can only compare boolean expressions");
            }

            return module->primitives.entry_bool;
        }

        case BinOpKind_CmpGt:
        case BinOpKind_CmpLt:
        case BinOpKind_CmpEq:
        case BinOpKind_CmpNotEq: {
	    type_id left_type = analyze_expression(module, bin_op->left);
	    type_id right_type = analyze_expression(module, bin_op->right);

            if (left_type != right_type) {
                sil_panic("binop: incomparable types");
            }

            return module->primitives.entry_bool;
        }
        default: sil_panic("Analyze Error: unhandled bin op");
    }
}

static type_id analyze_expression(Module* module, Expr* expression) {
    switch (expression->kind) {
        case ExprKind_Block: {
	    symtable_enter_scope(&module->symbol_table);
	    Block* block = expression->block;
	    block->scope = module->symbol_table.current_scope;

	    for (size_t i = 0; i < dynarray_len(block->statements); i++) {
		Stmt* statement = block->statements[i];
		analyze_statement(module, statement);
	    }

	    symtable_exit_scope(&module->symbol_table);

            Stmt* last_stmt = block->statements[dynarray_len(block->statements) - 1];

            type_id ret_type = module->primitives.entry_void;
            if (last_stmt->kind == StmtKind_NakedExpr or
                (last_stmt->kind == StmtKind_Expr and should_remove_statement_semi(last_stmt->expression))
            ) {
                ret_type = last_stmt->expression->codegen.type;
            }

            expression->codegen.type = ret_type;
            break;
	}
	case ExprKind_Let: {
	    Let* let = expression->let;
	    SymEntry* existing = symtable_get_local(&module->symbol_table, let->name);
	    if (existing != null) {
		sil_panic("Redeclaration of variable %.*s", str_format(let->name));
	    }

            type_id explicit_type = resolve_type(module, let->type);
	    type_id implicit_type = analyze_expression(module, let->value);

            expression->codegen.type = implicit_type;

	    if (explicit_type != 0 and explicit_type != implicit_type) {
		sil_panic(
                    "Sem Analysis Error: Implicit type conversion not allowed %zu %zu",
                    explicit_type,
                    implicit_type
                );
	    }

            SymEntry entry;
            entry.type = implicit_type;
	    symtable_insert(&module->symbol_table, let->name, &entry);

            expression->codegen.type = implicit_type;
            break;
	}
        case ExprKind_Symbol: {
	    String symbol = expression->symbol;
	    SymEntry* existing = symtable_get(&module->symbol_table, symbol);
	    if (existing == null) {
		sil_panic("Use of undeclared variable %.*s", str_format(symbol));
	    }

            expression->codegen.type = existing->type;
            break;
	}
        case ExprKind_BinOp: { 
            expression->codegen.type = analyze_bin_op(module, expression->binary_operator);
            break;
        }
        case ExprKind_FnCall: {
	    FnCall* fn_call = expression->fn_call;

	    Maybe(Item*) item = map_get(module->items, fn_call->name);
            if (item == None) {
                sil_panic("Call to undeclared function %.*s", str_format(fn_call->name));
            }

	    FnDef* fn_definition = unwrap(item)->fn_definition;

            // analyze arguments
	    for (size_t i = 0; i < dynarray_len(fn_call->arguments); i += 1) {
	        type_id arg_type = analyze_expression(module, fn_call->arguments[i]);

                FnParam* param = fn_definition->signature->parameters[i];
                type_id param_type = resolve_type(module, param->type);

                if (arg_type != param_type) {
                        sil_panic("function call with invalid arguments");
                }
	    }

            expression->codegen.type = resolve_type(module, fn_definition->signature->return_type);
            break;
	}
        case ExprKind_If: {
            If* if_expr = expression->if_expr;

            type_id condition_type = analyze_expression(module, if_expr->condition);
            TypeEntry* condition_type_entry = &module->type_table.types[condition_type];
            if (condition_type_entry->kind != TypeEntryKind_Bool) {
                sil_panic("if condition must be a bool");
            }

            type_id eval_type = analyze_expression(module, if_expr->then);

            if (if_expr->otherwise != null) {
                type_id otherwise_type = analyze_expression(module, if_expr->otherwise);
                if (otherwise_type != eval_type) {
                    sil_panic("branches must return the same type");
                }
            }

            expression->codegen.type = eval_type;
            break;
        }
        case ExprKind_Loop: {
            analyze_expression(module, expression->loop->body);
            expression->codegen.type = module->primitives.entry_void;
            break;
        }
        case ExprKind_Continue:
        case ExprKind_Break: {
            expression->codegen.type = module->primitives.entry_void;
            break;
        }
        case ExprKind_Unreachable: {
            expression->codegen.type = module->primitives.entry_never;
            break;
        }
        case ExprKind_Ret: {
            expression->codegen.type = analyze_expression(module, expression->ret);
            break;
        }
        case ExprKind_Asm: {
            expression->codegen.type = module->primitives.entry_isize;
            break;
        }
	case ExprKind_NumberLit: {
            expression->codegen.type = module->primitives.entry_i32;
            break;
	}
        case ExprKind_StringLit: {
            expression->codegen.type = module->primitives.entry_c_str;
            break;
        }
        case ExprKind_BoolLit: {
            expression->codegen.type = module->primitives.entry_bool;
            break;
        }
        case ExprKind_Cast: {
            // TODO: actual make sure cast is legit
            analyze_expression(module, expression->cast->expr);
            expression->codegen.type = resolve_type(module, expression->cast->to);
            break;
        }
        default: sil_panic("Analyzer Error: unhandled expression %d", expression->kind);
    }

    return expression->codegen.type;
}

static bool analyze_statement(Module* module, Stmt* statement) {
    switch (statement->kind) {
        case StmtKind_NakedExpr:
        case StmtKind_Expr: analyze_expression(module, statement->expression); break;
        default: sil_panic("Analyzer Error: Unhandled Statement Kind");
    }

    return true;
}

static bool analyze_fn_definition(Module* module, FnDef* fn_definition) {
    symtable_enter_scope(&module->symbol_table);

    // add paramaters to symtable
    for (size_t i = 0; i < dynarray_len(fn_definition->signature->parameters); i += 1) {
        FnParam* param = fn_definition->signature->parameters[i];
        type_id param_type = resolve_type(module, param->type);
        SymEntry entry = (SymEntry){ param_type };
        symtable_insert(&module->symbol_table, param->name, &entry);
    }

    Expr* block = fn_definition->body;
    analyze_expression(module, block);

    Stmt* last_stmt = block->block->statements[dynarray_len(block->block->statements) - 1];

    if (last_stmt->kind != StmtKind_NakedExpr and last_stmt->kind != StmtKind_Expr) {
        chn_error("last statement in a block must be an expression",);
    }
    if (last_stmt->expression->codegen.type != resolve_type(module, fn_definition->signature->return_type)) {
        type_id last = last_stmt->expression->codegen.type;
        type_id sig = resolve_type(module, fn_definition->signature->return_type);
        sil_panic("return value doesn't match signature %zu %zu %.*s", last, sig, str_format(fn_definition->signature->return_type->symbol));
    }

    symtable_exit_scope(&module->symbol_table);

    return true;
}

static bool analyze_ast(Module* module, AstRoot* root) {
    for (size_t i = 0; i < dynarray_len(root->items); i += 1) {
	// add item to top level
	Item* item = root->items[i];
        switch (item->kind) {
            case ItemKind_ExternFn: // make these part of an import table
            case ItemKind_FnDef:
                map_insert(module->items, item->name, &item); break;
            default:
                sil_panic("Analyzer Error: unhandled top level item");
        }
    }

    for (size_t i = 0; i < dynarray_len(root->items); i += 1) {
	Item* item = root->items[i];
        switch (item->kind) {
            case ItemKind_FnDef: analyze_fn_definition(module, item->fn_definition); break;
            default: break;
        }
    }

    return true;
}


void analyzer_analyze(Module* module) {
    setup_primitive_types(module);

    analyze_ast(module, module->ast);
}
