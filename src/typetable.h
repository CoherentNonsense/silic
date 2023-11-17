#ifndef TYPETABLE_H
#define TYPETABLE_H

#include <chnlib/dynarray.h>
#include <stdbool.h>


typedef usize type_id;

typedef struct TypeEntry TypeEntry;
typedef enum TypeEntryKind {
    TypeEntryKind_Invalid,
    TypeEntryKind_Void,
    TypeEntryKind_Never,
    TypeEntryKind_Ptr,
    TypeEntryKind_Int,
    TypeEntryKind_Size,
    TypeEntryKind_Bool,
} TypeEntryKind;

typedef struct TypePtr {
    type_id to;
    bool is_mut;
} TypePtr;

typedef struct TypeIntegral {
    bool is_signed;
} TypeIntegral;

typedef struct TypeEntry {
    TypeEntryKind kind;
    size_t bits;
    type_id parent_ptr_mut;
    type_id parent_ptr;

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

type_id typetable_new_type(TypeTable* table, TypeEntryKind kind, size_t bits);
type_id typetable_new_ptr(TypeTable* table, type_id to, bool is_mut);
type_id typetable_new_int(TypeTable* table, usize bits, bool is_signed);
type_id typetable_new_size(TypeTable* table, bool is_signed);

TypeEntry typetable_get(TypeTable* table, type_id id);

#endif
