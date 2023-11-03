#ifndef MODULE_H
#define MODULE_H

#include "ast.h"
#include "token.h"
#include "symtable.h"
#include "hashmap.h"
#include "dynarray.h"

typedef struct ModuleError {
    Token* token;
    const char* message;
    const char* hint;
    bool has_hint;
} ModuleError;

typedef struct Module {
    Span path;
    Span source;
    DynArray(Token) token_list;
    AstRoot* ast;

    HashMap(Item*) items;
    SymTable symbol_table;

    bool has_errors;
    DynArray(ModuleError) errors;
} Module;


void module_init(Module* module, Span path, Span source);

__attribute__((format(printf, 4, 5)))
void module_add_error(Module* module, Token* token, const char* hint, const char* message, ...);

void module_display_errors(Module* module);

#endif //!MODULE_H
