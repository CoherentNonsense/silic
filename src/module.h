#ifndef MODULE_H
#define MODULE_H

#include "ast.h"
#include "token.h"
#include "hashmap.h"
#include "dynarray.h"

typedef struct ModuleError {
} ModuleError;

typedef struct Module {
    Span source;
    DynArray(Token) token_list;
    AstRoot* ast;
} Module;

#endif //!MODULE_H
