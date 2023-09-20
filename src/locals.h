#ifndef LOCALS_H
#define LOCALS_H

#include "hashmap.h"
#include "dynarray.h"

typedef struct LocalEnvironment {
    HashMap bound;
} LocalEnvironment;

typedef struct Locals {
    DynArray(LocalEnvironment) envs;
} Locals;

/*
void locals_init(Locals* const locals);
void locals_push(Locals* parent, Locals* const child);
void locals_pop(Locals* locals);

void locals_insert(Locals* locals, Span name, void* value);
void locals_get(Locals* locals, Span name);
*/
#endif // LOCALS_H
