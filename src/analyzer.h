#ifndef CODEGEN_ANALYZE_H
#define CODEGEN_ANALYZE_H

#include "codegen/environment.h"
#include "parser/parser.h"
#include "hashmap.h"

typedef struct CodegenContext {
    LLVMModuleRef module;
    LLVMBuilderRef builder;
    AstNode* current_node;
    HashMap function_map;
    Environment* current_env;
} CodegenContext;

void analyzer_decl(CodegenContext* context, AstNode* root);

#endif
