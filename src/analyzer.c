#include "analyzer.h"

#include "ast.h"

static void analyze_block(Module* module, Block* block) {
    printf("analyzing block\n");
}

static void analyze_fn_definition(Module* module, FnDef* fn_definition) {
    Block* block = fn_definition->body;
    analyze_block(module, block);
}

static void analyze_extern_fn(Module* module, ExternFn* extern_fn) {
    printf("analyzing extern fn\n");
}

static void analyze_ast(Module* module, AstRoot* root) {
    map_init(module->items);
    for (size_t i = 0; i < root->items.length; i++) {
	// add item to top level
	Item* item = dynarray_get(root->items, i);
	map_insert(module->items, item->name, item);

        switch (item->kind) {
            case ItemKind_FnDef: analyze_fn_definition(module, item->fn_definition); break;
            case ItemKind_ExternFn: analyze_extern_fn(module, item->extern_fn); break;
            default: sil_panic("Analysis Error: Top level item not implemented");
        }
    }
}


void analyzer_analyze(Module* module) {
    analyze_ast(module, module->ast);
}
