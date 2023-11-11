#ifndef SYMTABLE_H
#define SYMTABLE_H

#include "ast.h"

#include <chnlib/str.h>


// SymTable defined in ast.h

void symtable_init(SymTable* symtable);
void symtable_deinit(SymTable* symtable);

void symtable_enter_scope(SymTable* const symtable);
void symtable_exit_scope(SymTable* const symtable);

void symtable_insert(SymTable* const symtable, String const name, SymEntry* const entry);
SymEntry* symtable_get(SymTable* const symtable, String const name);
SymEntry* symtable_get_local(SymTable* const symtable, String const name);

#endif
