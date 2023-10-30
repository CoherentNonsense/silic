#include "analyzer.h"

#include "ast.h"

static void analyze_block(Module* module, Block* block) {
}

static void analyze_fn_definition(Module* module, FnDef* fn_definition) {
    FnSig* signature = fn_definition->signature;
    
    Block* block = fn_definition->body;
}

static void analyze_ast(Module* module, AstRoot* root) {
    map_init(module->functions);
    for (int i = 0; i < root->items.length; i++) {
	// add item to top level
	Item* item = dynarray_get(root->items, i);
	map_insert(module->functions, item->name, item);

	// analyze item // TODO: not only functions here
	analyze_fn_definition(module, item->fn_definition);
    }
}


void analyzer_analyze(Module* module) {
    analyze_ast(module, module->ast);
}
