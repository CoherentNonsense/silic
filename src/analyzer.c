#include "analyzer.h"

#include "ast.h"
#include <stdbool.h>
#include <iso646.h>

static void analyze_statement(Module*, Stmt*);
static void analyze_expression(Module*, Expr*);

static bool type_eq(Type* a, Type* b) {
    if (a->kind != b->kind) { return false; }

    if (a->kind == TypeKind_Ptr) { return type_eq(a->ptr.to, b->ptr.to); }

    return true;
}

static void analyze_expression(Module* module, Expr* expression) {
    switch (expression->kind) {
        case ExprKind_Block: {
	    symtable_enter_scope(&module->symbol_table);
	    Block* block = expression->block;
	    block->scope = module->symbol_table.current_scope;

	    for (size_t i = 0; i < block->statements.length; i++) {
		Stmt* statement = dynarray_get(block->statements, i);
		analyze_statement(module, statement);
	    }

	    expression->type = malloc(sizeof(Type));
	    expression->type->kind = TypeKind_Void;

	    symtable_exit_scope(&module->symbol_table);
	    break;
	}
	case ExprKind_Let: {
	    Let* let = expression->let;
	    Let* existing = symtable_get_local(&module->symbol_table, let->name);
	    if (existing != NULL) {
		sil_panic("Redeclaration of variable %.*s", (int)let->name.length, let->name.start);
		return;
	    }

	    analyze_expression(module, let->value);
	    if (!type_eq(let->type, let->value->type)) {
		sil_panic("Sem Analysis Error: Implicit type conversion not allowed");
	    }

	    // TODO: use a type table
	    symtable_insert(&module->symbol_table, let->name, let);
	    break;
	}
        case ExprKind_Symbol: {
	    Span symbol = expression->symbol;
	    Let* existing = symtable_get_local(&module->symbol_table, symbol);
	    if (existing == NULL) {
		sil_panic("Use of undeclared variable");
	    }
	    break;
	}
        case ExprKind_BinOp: {
	    BinOp* binary_operator = expression->binary_operator;
	    analyze_expression(module, binary_operator->left);
	    analyze_expression(module, binary_operator->right);
	    break;
	}
        case ExprKind_FnCall: {
	    FnCall* fn_call = expression->fn_call;
	    for (size_t i = 0; i < fn_call->arguments.length; i += 1) {
		analyze_expression(module, dynarray_get(fn_call->arguments, i));
	    }

	    Item* item = map_get(module->items, fn_call->name);
	    if (item == NULL or (item->kind != ItemKind_FnDef and item->kind != ItemKind_ExternFn)) {
		sil_panic("Call to undeclared function %.*s", (int)fn_call->name.length, fn_call->name.start);
	    }
	    FnDef* fn_definition = item->fn_definition;

	    expression->type = malloc(sizeof(Type));
	    *expression->type = *fn_definition->signature->return_type;

	    break;
	}
	case ExprKind_NumberLit: {
	    expression->type = malloc(sizeof(Type));
	    expression->type->kind = TypeKind_Int;
	    break;
	}
        default: break;
    }
}

static void analyze_statement(Module* module, Stmt* statement) {
    switch (statement->kind) {
        case StmtKind_Expr: analyze_expression(module, statement->expression); break;
        default: sil_panic("Analyzer Error: Unhandled Statement Kind");
    }
}

static void analyze_fn_definition(Module* module, FnDef* fn_definition) {
    Expr* block = fn_definition->body;
    analyze_expression(module, block);
}

static void analyze_ast(Module* module, AstRoot* root) {
    map_init(module->items);
    for (size_t i = 0; i < root->items.length; i += 1) {
	// add item to top level
	Item* item = dynarray_get(root->items, i);
	map_insert(module->items, item->name, item);
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
    analyze_ast(module, module->ast);
}
