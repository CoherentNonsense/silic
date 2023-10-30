#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "ast.h"
#include "hashmap.h"
#include "dynarray.h"


// SymTable defined in ast.h

SymTable symtable_init(void);

void symtable_enterScope(SymTable* const symtable);
void symtable_exitScope(SymTable* const symtable);

void symtable_insert(SymTable* const locals, Span const name, Let* const value);
Let* symtable_get(SymTable* const locals, Span const name);

#endif // SYMTABLE_H
