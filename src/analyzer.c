#include "analyzer.h"

#include "ast.h"

typedef struct AnalyzerContext {
    Module* module;
} AnalyzerContext;

static void analyze_fn_definition(AnalyzerContext* context, FnDef* fn_definition) {
    
}

static void analyze_ast(AnalyzerContext* context, AstRoot* root) {
    map_init(context->module->functions);
    for (int i = 0; i < root->items.length; i++) {
	Item* item = dynarray_get(root->items, i);
	map_insert(context->module->functions, item->name, item);
    }
}


void analyzer_analyze(Module* module) {
    AnalyzerContext context;
    context.module = module;

    analyze_ast(&context, module->ast);
}
