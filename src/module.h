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
    // FIXME: should path and source be a separate String type since spans
    // 	      don't guarantee null termination (i pass them to C functions)
    Span path;
    Span source;
    DynArray(Token) token_list;
    AstRoot* ast;

    DynArray(ModuleError) errors;
} Module;

#endif //!MODULE_H
