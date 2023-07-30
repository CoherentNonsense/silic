#include "environment.h"
#include "hashmap.h"
#include "llvm-c/Types.h"
#include <stdlib.h>

Environment* env_new(Environment* parent) {
    Environment* env = malloc(sizeof(Environment));
    env->parent = parent;
    env->bindings = (HashMap){0};

    return env;
}

void env_delete(Environment* env) {
    map_delete(&env->bindings);
    free(env);
}

void env_add_binding(Environment* env, String name, AstNode* type, LLVMValueRef ptr) {
    Binding* binding = malloc(sizeof(Binding));
    binding->type = type;
    binding->ptr = ptr;
    map_insert(&env->bindings, name, binding);
}

Binding* env_get_binding(Environment* env, String name) {
    Environment* current_env = env;
    Binding* value = map_get(&env->bindings, name);
    while (!value && env->parent != NULL) {
        current_env = current_env->parent;
        AstNode* value = map_get(&env->bindings, name);
    }

    return value;
}
