#include "typetable.h"

void typetable_init(TypeTable* table) {
    table->types = dynarray_init();
}

void typetable_deinit(TypeTable* table) {
    dynarray_deinit(table->types);
}

TypeEntry* typetable_new_type(TypeTable* table, TypeEntryKind kind, size_t bits) {
    TypeEntry* entry = dynarray_add(table->types);
    entry->parent_ptr = null;
    entry->parent_ptr_mut = null;
    entry->kind = kind;
    entry->bits = bits;

    return entry;
}

TypeEntry* typetable_new_ptr(TypeTable* table, TypeEntry* to, bool is_mut) {
    TypeEntry* entry = dynarray_add(table->types);

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
