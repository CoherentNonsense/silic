#include "analyzer.h"

#include "ast.h"

typedef struct AnalyzerContext {
    Module* module;
} AnalyzerContext;

static void analyze_ast(AnalyzerContext* context, AstRoot* root) {
}


void analyzer_run(Module* module) {
    AnalyzerContext context;
    context.module = module;

    analyze_ast(&context, module->ast);
}
