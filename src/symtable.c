#include "symtable.h"
#include "hashmap.h"


void symtable_init(SymTable* const symtable) {
    dynarray_init(symtable->envs);
}

void symtable_push(SymTable* const symtable) {
    dynarray_reserve(symtable->envs, 1);
    LocalEnvironment* const env = dynarray_last_ref(symtable->envs);

    map_init(env->variables);
}

void symtable_pop(SymTable* const symtable) {
    LocalEnvironment const* const env = dynarray_last_ref(symtable->envs);
    map_deinit(env->variables);
    
    symtable->envs.length -= 1;
}

void symtable_insert(SymTable* const symtable, Span const name, Let* const value) {
    LocalEnvironment const* const env = dynarray_last_ref(symtable->envs);
    map_insert(env->variables, name, value);
}

Let* symtable_get(SymTable* const symtable, Span const name) {
    for (int i = symtable->envs.length - 1; i >= 0; i--) {
	LocalEnvironment const* const env = dynarray_get_ref(symtable->envs, i);
	if (map_has(env->variables, name)) {
	    return map_get(env->variables, name);
	}
    }

    return NULL;
}
