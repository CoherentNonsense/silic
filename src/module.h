#ifndef MODULE_H
#define MODULE_H

#include "ast.h"
#include "token.h"
#include "symtable.h"
#include "hashmap.h"
#include "dynarray.h"

typedef struct ModuleError {
    Span message;
} ModuleError;

typedef struct Module {
    bool build;
    Span path;
    Span source;
    DynArray(Token) token_list;
    AstRoot* ast;

    HashMap(Item*) items;
    SymTable symbol_table;

    DynArray(ModuleError) errors;
} Module;

void module_add_error(Module* module, Span error);

#endif //!MODULE_H
