#include "analyzer.h"

#include "ast.h"
#include "typetable.h"
#include <stdbool.h>
#include <iso646.h>

static void analyze_statement(Module*, Stmt*);
static TypeEntry* analyze_expression(Module*, Expr*);

static void setup_primitive_types(Module* module) {

    // void
    module->primitives.entry_void = typetable_new_type(&module->type_table, TypeEntryKind_Void, 0);

    map_insert(module->types, span_from_literal("void"), module->primitives.entry_void);

    // char
    module->primitives.entry_c_char = typetable_new_type(&module->type_table, TypeEntryKind_Int, 8);
    module->primitives.entry_c_char->integral.is_signed = true;
    map_insert(module->types, span_from_literal("c_char"), module->primitives.entry_c_char);

    // integral
    module->primitives.entry_u8 = typetable_new_type(&module->type_table, TypeEntryKind_Int, 8);
    module->primitives.entry_u16 = typetable_new_type(&module->type_table, TypeEntryKind_Int, 16);
    module->primitives.entry_u32 = typetable_new_type(&module->type_table, TypeEntryKind_Int, 32);
    module->primitives.entry_u64 = typetable_new_type(&module->type_table, TypeEntryKind_Int, 64);

    module->primitives.entry_u8->integral.is_signed = false;
    module->primitives.entry_u16->integral.is_signed = false;
    module->primitives.entry_u32->integral.is_signed = false;
    module->primitives.entry_u64->integral.is_signed = false;

    map_insert(module->types, span_from_literal("u8"), module->primitives.entry_u8);
    map_insert(module->types, span_from_literal("u16"), module->primitives.entry_u16);
    map_insert(module->types, span_from_literal("u32"), module->primitives.entry_u32);
    map_insert(module->types, span_from_literal("u64"), module->primitives.entry_u64);


    module->primitives.entry_i8 = typetable_new_type(&module->type_table, TypeEntryKind_Int, 8);
    module->primitives.entry_i16 = typetable_new_type(&module->type_table, TypeEntryKind_Int, 16);
    module->primitives.entry_i32 = typetable_new_type(&module->type_table, TypeEntryKind_Int, 32);
    module->primitives.entry_i64 = typetable_new_type(&module->type_table, TypeEntryKind_Int, 64);

    module->primitives.entry_i8->integral.is_signed = true;
    module->primitives.entry_i16->integral.is_signed = true;
    module->primitives.entry_i32->integral.is_signed = true;
    module->primitives.entry_i64->integral.is_signed = true;

    map_insert(module->types, span_from_literal("i8"), module->primitives.entry_i8);
    map_insert(module->types, span_from_literal("i16"), module->primitives.entry_i16);
    map_insert(module->types, span_from_literal("i32"), module->primitives.entry_i32);
    map_insert(module->types, span_from_literal("i64"), module->primitives.entry_i64);
}

static TypeEntry* resolve_type(Module* module, Type* type) {
    switch (type->kind) {
        case TypeKind_Symbol: {
            TypeEntry* entry = map_get(module->types, type->symbol);
            if (entry == NULL) {
                sil_panic("reference to undeclared type");
            }
            return entry;
        }
        case TypeKind_Ptr: {
            TypeEntry* child = resolve_type(module, type->ptr.to);
            TypeEntry* existing = type->ptr.is_mut ? child->parent_ptr_mut : child->parent_ptr;
            if (existing != NULL) {
                return existing;
            }

            return typetable_new_ptr(&module->type_table, child, type->ptr.is_mut);
        }
        default: sil_panic("cannot resolve type %d", type->kind);
    }
}

static bool type_eq(TypeEntry* a, TypeEntry* b) {
    return a == b;
}

static TypeEntry* analyze_expression(Module* module, Expr* expression) {
    switch (expression->kind) {
        case ExprKind_Block: {
	    symtable_enter_scope(&module->symbol_table);
	    Block* block = expression->block;
	    block->scope = module->symbol_table.current_scope;

	    for (size_t i = 0; i < block->statements.length; i++) {
		Stmt* statement = dynarray_get(block->statements, i);
		analyze_statement(module, statement);
	    }

	    symtable_exit_scope(&module->symbol_table);

            // TODO: evaluate to last expression
	    return module->primitives.entry_void;
	}
	case ExprKind_Let: {
	    Let* let = expression->let;
	    SymEntry* existing = symtable_get_local(&module->symbol_table, let->name);
	    if (existing != NULL) {
		sil_panic("Redeclaration of variable %.*s", (int)let->name.length, let->name.start);
	    }

            TypeEntry* explicit_type = resolve_type(module, let->type);
	    TypeEntry* implicit_type = analyze_expression(module, let->value);

	    if (not type_eq(explicit_type, implicit_type)) {
		sil_panic(
                    "Sem Analysis Error: Implicit type conversion not allowed %d %d",
                    explicit_type->kind,
                    implicit_type->kind
                );
	    }

            SymEntry entry;
            entry.type = explicit_type;
	    symtable_insert(&module->symbol_table, let->name, &entry);

            return explicit_type;
	}
        case ExprKind_Symbol: {
	    Span symbol = expression->symbol;
	    SymEntry* existing = symtable_get(&module->symbol_table, symbol);
	    if (existing == NULL) {
		sil_panic("Use of undeclared variable %.*s", (int)symbol.length, symbol.start);
	    }

            return existing->type;
	}
        case ExprKind_BinOp: {
	    BinOp* binary_operator = expression->binary_operator;
	    TypeEntry* left_type = analyze_expression(module, binary_operator->left);
	    TypeEntry* right_type = analyze_expression(module, binary_operator->right);

            if (not type_eq(left_type, right_type)) {
                sil_panic("binop: incompatable types");
            }

            return left_type;
	}
        case ExprKind_FnCall: {
	    FnCall* fn_call = expression->fn_call;

	    Item* item = map_get(module->items, fn_call->name);
            if (item == NULL) {
                sil_panic("Call to undeclared function %.*s", (int)fn_call->name.length, fn_call->name.start);
            }

	    FnDef* fn_definition = item->fn_definition;

            // analyze arguments
	    for (size_t i = 0; i < fn_call->arguments.length; i += 1) {
		TypeEntry* arg_type = analyze_expression(module, dynarray_get(fn_call->arguments, i));

                FnParam* param = dynarray_get(fn_definition->signature->parameters, i);
                TypeEntry* param_type = resolve_type(module, param->type);

                if (not type_eq(arg_type, param_type)) {
                        sil_panic("function call with invalid arguments");
                }
	    }

            return resolve_type(module, fn_definition->signature->return_type);
	}
	case ExprKind_NumberLit: {
            return module->primitives.entry_i32;
	}
        case ExprKind_Ret: {
            return analyze_expression(module, expression->ret);
        }
        case ExprKind_StringLit: {
            return typetable_new_ptr(&module->type_table, module->primitives.entry_c_char, true);
        }
        default: sil_panic("Analyzer Error: unhandled expression %d", expression->kind);
    }
}

static void analyze_statement(Module* module, Stmt* statement) {
    switch (statement->kind) {
        case StmtKind_Expr: analyze_expression(module, statement->expression); break;
        default: sil_panic("Analyzer Error: Unhandled Statement Kind");
    }
}

static void analyze_fn_definition(Module* module, FnDef* fn_definition) {
    symtable_enter_scope(&module->symbol_table);

    // add paramaters to symtable
    for (size_t i = 0; i < fn_definition->signature->parameters.length; i += 1) {
        FnParam* param = dynarray_get(fn_definition->signature->parameters, i);
        SymEntry entry = (SymEntry){ resolve_type(module, param->type) };
        symtable_insert(&module->symbol_table, param->name, &entry);
    }

    Expr* block = fn_definition->body;
    analyze_expression(module, block);

    symtable_exit_scope(&module->symbol_table);
}

static void analyze_ast(Module* module, AstRoot* root) {
    for (size_t i = 0; i < root->items.length; i += 1) {
	// add item to top level
	Item* item = dynarray_get(root->items, i);
        switch (item->kind) {
            case ItemKind_ExternFn: // make these part of an import table
            case ItemKind_FnDef:
                map_insert(module->items, item->name, item); break;
            default:
                sil_panic("Analyzer Error: unhandled top level item");
        }
    }

    for (size_t i = 0; i < root->items.length; i += 1) {
	Item* item = dynarray_get(root->items, i);
        switch (item->kind) {
            case ItemKind_FnDef: analyze_fn_definition(module, item->fn_definition); break;
            default: break;
        }
    }
}


void analyzer_analyze(Module* module) {
    setup_primitive_types(module);

    analyze_ast(module, module->ast);
}
