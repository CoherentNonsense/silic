#include "symtable.h"


static void scope_init(Scope* scope, Scope* parent) {
    scope->parent = parent;
    scope->children = dynarray_init();
    scope->symbols = map_init();
}

static void scope_deinit(Scope* scope) {
    dynarray_deinit(scope->children);
    map_deinit(scope->symbols);
}

void symtable_init(SymTable* symtable) {
    scope_init(&symtable->root_scope, NULL);
    symtable->current_scope = &symtable->root_scope;
}

void symtable_deinit(SymTable* symtable) {
    Scope* current = symtable->current_scope;
    while (current != NULL) {
        scope_deinit(current);
        current = current->parent;
    }
}

void symtable_enter_scope(SymTable* const symtable) {
    Scope* current_scope = symtable->current_scope;

    Scope* scope = dynarray_add(current_scope->children);
    scope_init(scope, current_scope);

    symtable->current_scope = scope;
}

void symtable_exit_scope(SymTable* const symtable) {
    symtable->current_scope = symtable->current_scope->parent;
}

void symtable_insert(SymTable* const symtable, String const name, SymEntry* const entry) {
    map_insert(symtable->current_scope->symbols, name, entry);
}

SymEntry* symtable_get(SymTable* const symtable, String const name) {
    Scope* scope = symtable->current_scope;
    while (scope != NULL) {
        SymEntry* entry = map_get_ref(scope->symbols, name);
        if (entry != NULL) {
            return entry;
        }

        scope = scope->parent;
    }

    return NULL;
}

SymEntry* symtable_get_local(SymTable* const symtable, String const name) {
    SymEntry* entry = map_get_ref(symtable->current_scope->symbols, name);
    return entry;
}
