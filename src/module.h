#ifndef MODULE_H
#define MODULE_H

#include "ast.h"
#include "hashmap.h"

typedef struct Module {
    AstRoot* ast;
    Span source;
} Module;

#endif //!MODULE_H
