#include "typetable.h"

void typetable_init(TypeTable* table) {
    table->types = dynarray_init();
    // type_id = 0 is invalid
    TypeEntry* entry = dynarray_add(table->types);
    entry->kind = TypeEntryKind_Invalid;
}

void typetable_deinit(TypeTable* table) {
    dynarray_deinit(table->types);
}

type_id typetable_new_type(TypeTable* table, TypeEntryKind kind, size_t bits) {
    TypeEntry* entry = dynarray_add(table->types);
    entry->parent_ptr = 0;
    entry->parent_ptr_mut = 0;
    entry->kind = kind;
    entry->bits = bits;

    return dynarray_len(table->types) - 1;
}

type_id typetable_new_ptr(TypeTable* table, type_id to, bool is_mut) {
    type_id new = typetable_new_type(table, TypeEntryKind_Ptr, 64);

    TypeEntry* new_entry = &table->types[new];
    new_entry->ptr.to = to;
    new_entry->ptr.is_mut = is_mut;

    TypeEntry* to_entry = &table->types[to];
    if (is_mut) {
        to_entry->parent_ptr_mut = new;
    } else {
        to_entry->parent_ptr = new;
    }

    return new;
}

type_id typetable_new_int(TypeTable* table, usize bits, bool is_signed) {
    type_id new = typetable_new_type(table, TypeEntryKind_Int, bits);

    TypeEntry* new_entry = &table->types[new];
    new_entry->integral.is_signed = is_signed;

    return new;
}

type_id typetable_new_size(TypeTable* table, bool is_signed) {
    type_id new = typetable_new_type(table, TypeEntryKind_Size, 64);

    TypeEntry* new_entry = &table->types[new];
    new_entry->integral.is_signed = is_signed;

    return new;
}

TypeEntry typetable_get(TypeTable* table, type_id id) {
    return table->types[id];
}
