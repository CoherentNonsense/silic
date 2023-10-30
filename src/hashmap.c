#include "hashmap.h"

#include "util.h"
#include <iso646.h>


typedef Entry(void*) EntryAny;

static size_t hash_function(Span string) {
    // FNV 32-bit hash
    unsigned int h = 2166136261;
    for (size_t i = 0; i < string.length; i += 1) {
        h = h ^ ((unsigned char)*(string.start + i));
        h = h * 16777619;
    }

    return h;
}

void map_insert__polymorphic(HashMapAny* map, Span key, void* value) {
    if (map->entries.length * 5 >= map->entries.capacity * 4) {
        size_t old_capacity = map->entries.capacity;
        dynarray_resize(map->entries, map->entries.capacity * 2 + 8);
        for (size_t i = old_capacity; i < map->entries.capacity; i++) {
            EntryAny* entry = (EntryAny*)dynarray_get_ref(map->entries, i);
            entry->used = 0;
        }
    }

    size_t start_index = hash_function(key);
    for (size_t i = 0; i < map->entries.capacity; i++) {
        size_t index = (start_index + i) % map->entries.capacity;
        EntryAny* entry = (EntryAny*)dynarray_get_ref(map->entries, index);

        if (entry->used) {
            continue;
        }

        entry->key = key;
        entry->value = value;
        entry->used = 1;

        map->entries.length += 1;

        return;
    }

    sil_panic("Hashmap::map_insert trying to insert into full hashmap");
}

void* map_get__polymorphic(HashMapAny const* const map, Span const key) {
    size_t start_index = hash_function(key);
    for (size_t i = 0; i < map->entries.capacity; i++) {
        size_t index = (start_index + i) % map->entries.capacity;
        EntryAny* entry = (EntryAny*)dynarray_get_ref(map->entries, index);
        if (entry->used and strncmp(key.start, entry->key.start, key.length)) {
            return entry->value;
        }
    }

    return NULL;
}

bool map_has__polymorphic(HashMapAny const* const map, Span const key) {
    return map_get__polymorphic(map, key) != NULL;
}
