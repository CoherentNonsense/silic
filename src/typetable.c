#include "typetable.h"

void typetable_init(TypeTable* table) {
    dynarray_init(table->types);
}

void typetable_deinit(TypeTable* table) {
    dynarray_deinit(table->types);
}

TypeEntry* typetable_new_type(TypeTable* table, TypeEntryKind kind, size_t bits) {
    dynarray_reserve(table->types, 1);
    TypeEntry* entry = dynarray_last_ref(table->types);
    entry->parent_ptr = NULL;
    entry->parent_ptr_mut = NULL;
    entry->kind = kind;
    entry->bits = bits;

    return entry;
}

TypeEntry* typetable_new_ptr(TypeTable* table, TypeEntry* to, bool is_mut) {
    dynarray_reserve(table->types, 1);
    TypeEntry* entry = dynarray_last_ref(table->types);
    if (is_mut) {
        to->parent_ptr_mut = entry;
    } else {
        to->parent_ptr = entry;
    }

    entry->kind = TypeEntryKind_Ptr;
    entry->ptr.to = to;
    entry->ptr.is_mut = is_mut;
    entry->bits = 8 * 8; // HACK: all ptrs are 8 bytes

    return entry;
}
