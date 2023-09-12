#ifndef MODULE_H
#define MODULE_H

#include "ast.h"
#include "token.h"
#include "hashmap.h"
#include "dynarray.h"

typedef struct ModuleError {
    char* message;
} ModuleError;

typedef struct Module {
    bool build;
    Span path;
    Span source;
    DynArray(Token) token_list;
    AstRoot* ast;

    HashMap items;

    DynArray(ModuleError) errors;
} Module;

#endif //!MODULE_H
