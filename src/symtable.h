#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "ast.h"
#include "hashmap.h"
#include "dynarray.h"


typedef struct Scope {
    HashMap(Let*) variables;
} LocalEnvironment;

typedef struct SymTable {
    DynArray(LocalEnvironment) envs;
} SymTable;

void symtable_init(SymTable* const locals);
void symtable_push(SymTable* const parent);
void symtable_pop(SymTable* const locals);

void symtable_insert(SymTable* const locals, Span const name, Let* const value);
Let* symtable_get(SymTable* const locals, Span const name);

#endif // SYMTABLE_H
