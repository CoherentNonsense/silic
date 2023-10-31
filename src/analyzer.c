#include "analyzer.h"

#include "ast.h"

static void analyze_statement(Module*, Stmt*);
static void analyze_expression(Module*, Expr*);

static void analyze_block(Module* module, Block* block) {
    symtable_enter_scope(&module->symbol_table);
    block->scope = module->symbol_table.current_scope;

    for (size_t i = 0; i < block->statements.length; i++) {
        Stmt* statement = dynarray_get(block->statements, i);
        analyze_statement(module, statement);
    }

    symtable_exit_scope(&module->symbol_table);
}

static void analyze_let(Module* module, Let* let) {
    Let* existing = symtable_get_local(&module->symbol_table, let->name);
    if (existing != NULL) {
        sil_panic("Redeclaration of variable %.*s", (int)let->name.length, let->name.start);
        return;
    }

    // TODO: use a type table
    symtable_insert(&module->symbol_table, let->name, let);
}

static void analyze_symbol(Module* module, Span symbol) {
    Let* existing = symtable_get_local(&module->symbol_table, symbol);
    if (existing == NULL) {
        sil_panic("Use of undeclared variable");
    }
}

static void analyze_binary_operator(Module* module, BinOp* binary_operator) {
    analyze_expression(module, binary_operator->left);
    analyze_expression(module, binary_operator->right);
}

static void analyze_fn_call(Module* module, FnCall* fn_call) {
    for (size_t i = 0; i < fn_call->arguments.length; i += 1) {
        analyze_expression(module, dynarray_get(fn_call->arguments, i));
    }
}

static void analyze_expression(Module* module, Expr* expression) {
    switch (expression->kind) {
        case ExprKind_Block: analyze_block(module, expression->block); break;
        case ExprKind_Let: analyze_let(module, expression->let); break;
        case ExprKind_Symbol: analyze_symbol(module, expression->symbol); break;
        case ExprKind_BinOp: analyze_binary_operator(module, expression->binary_operator); break;
        case ExprKind_FnCall: analyze_fn_call(module, expression->fn_call); break;
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
    Block* block = fn_definition->body;
    analyze_block(module, block);
}

static void analyze_ast(Module* module, AstRoot* root) {
    map_init(module->items);
    for (size_t i = 0; i < root->items.length; i++) {
	// add item to top level
	Item* item = dynarray_get(root->items, i);
	map_insert(module->items, item->name, item);

        switch (item->kind) {
            case ItemKind_FnDef: analyze_fn_definition(module, item->fn_definition); break;
            default: break;
        }
    }
}


void analyzer_analyze(Module* module) {
    analyze_ast(module, module->ast);
}
