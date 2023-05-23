#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"

typedef struct Codegen {
    
} Codegen;

void codegen_new(void);
void codegen_generate(AstNode* ast);

void codegen_print(void);

#endif
