#ifndef CODEGEN_ENVIRONMENT_H
#define CODEGEN_ENVIRONMENT_H

#include "hashmap.h"
#include "ast.h"
#include "string_buffer.h"
#include "llvm-c/Types.h"

typedef struct Binding {
    LLVMValueRef ptr;
    AstNode* type;
} Binding;

typedef struct Environment {
    struct Environment* parent;
    HashMap bindings;
} Environment;

Environment* env_new(Environment* parent);
void env_delete(Environment* env);
void env_add_binding(Environment* env, String name, AstNode* type, LLVMValueRef ptr);
Binding* env_get_binding(Environment* env, String name);

#endif // !CODEGEN_ENVIRONMENT_H
