#ifndef TYPETABLE_H
#define TYPETABLE_H

#include <chnlib/dynarray.h>
#include <stdbool.h>


typedef struct TypeEntry TypeEntry;
typedef enum TypeEntryKind {
    TypeEntryKind_Void,
    TypeEntryKind_Ptr,
    TypeEntryKind_Int,
} TypeEntryKind;

typedef struct TypePtr {
    TypeEntry* to;
    bool is_mut;
} TypePtr;

typedef struct TypeIntegral {
    bool is_signed;
} TypeIntegral;

typedef struct TypeEntry {
    TypeEntryKind kind;
    size_t bits;
    struct TypeEntry* parent_ptr_mut;
    struct TypeEntry* parent_ptr;

    union {
        TypePtr ptr;
        TypeIntegral integral;
    };
} TypeEntry;

typedef struct TypeTable {
    DynArray(TypeEntry) types;
} TypeTable;


void typetable_init(TypeTable* table);
void typetable_deinit(TypeTable* table);

TypeEntry* typetable_new_type(TypeTable* table, TypeEntryKind kind, size_t bits);
TypeEntry* typetable_new_ptr(TypeTable* table, TypeEntry* to, bool is_mut);

#endif
