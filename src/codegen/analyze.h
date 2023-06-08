#ifndef CODEGEN_ANALYZE_H
#define CODEGEN_ANALYZE_H

#include "parser/parser.h"

typedef struct CodegenContext CodegenContext;

void codegen_analyze(CodegenContext* context, AstNode* root);

#endif
