#ifndef MODULE_H
#define MODULE_H

#include "ast.h"

typedef struct Module {
    AstRoot* ast;
    char* source;
} Module;

#endif //!MODULE_H
