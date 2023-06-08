#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser/parser.h"
#include "hashmap.h"

typedef struct CodegenContext {
    LLVMModuleRef module;
    LLVMBuilderRef builder;
    AstNode* current_node;
    HashMap function_map;
} CodegenContext;

void codegen_new(void);
void codegen_generate(AstNode* ast);

void codegen_print(void);

#endif
