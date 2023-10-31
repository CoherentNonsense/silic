#include "symtable.h"


static void scope_init(Scope* scope, Scope* parent) {
    scope->parent = parent;
    dynarray_init(scope->children);
    map_init(scope->symbols);
}

void symtable_init(SymTable* symtable) {
    scope_init(&symtable->root_scope, NULL);
    symtable->current_scope = &symtable->root_scope;
}

void symtable_enter_scope(SymTable* const symtable) {
    Scope* current_scope = symtable->current_scope;

    dynarray_reserve(current_scope->children, 1);
    Scope* scope = dynarray_last_ref(current_scope->children);
    scope_init(scope, current_scope);

    symtable->current_scope = scope;
}

void symtable_exit_scope(SymTable* const symtable) {
    symtable->current_scope = symtable->current_scope->parent;
}

void symtable_insert(SymTable* const symtable, Span const name, Let* const let) {
    map_insert(symtable->current_scope->symbols, name, let);
}

Let* symtable_get(SymTable* const symtable, Span const name) {
    Scope* scope = symtable->current_scope;
    while (scope != NULL) {
        Let* local = map_get(scope->symbols, name);
        if (local != NULL) {
            return local;
        }

        scope = scope->parent;
    }

    return NULL;
}

Let* symtable_get_local(SymTable* const symtable, Span const name) {
    Let* let = map_get(symtable->current_scope->symbols, name);
    return let;
}
