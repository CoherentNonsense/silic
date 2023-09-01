#include "hashmap.h"

#include "util.h"


static size_t hash_function(Span string) {
    // FNV 32-bit hash
    unsigned int h = 2166136261;
    for (int i = 0; i < string.length; i += 1) {
        h = h ^ ((unsigned char)*(string.start + i));
        h = h * 16777619;
    }

    return h;
}

void map_delete(HashMap* map) {
    list_deinit(map->entries);
}

void map_insert(HashMap* map, Span key, void* value) {
    if (map->entries.length * 5 >= map->entries.capacity * 4) {
        size_t old_capacity = map->entries.capacity;
        list_resize(map->entries, map->entries.capacity * 2 + 8);
        for (int i = old_capacity; i < map->entries.capacity; i++) {
            Entry* entry = list_get_ref(map->entries, i);
            entry->used = 0;
        }
    }

    size_t start_index = hash_function(key);
    for (int i = 0; i < map->entries.capacity; i++) {
        size_t index = (start_index + i) % map->entries.capacity;
        Entry* entry = list_get_ref(map->entries, index);

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

void* map_get(HashMap* map, Span key) {
    size_t start_index = hash_function(key);
    for (int i = 0; i < map->entries.capacity; i++) {
        size_t index = (start_index + i) % map->entries.capacity;
        Entry* entry = list_get_ref(map->entries, index);
        if (entry->used && strncmp(key.start, entry->key.start, key.length)) {
            return entry->value;
        }
    }

    return NULL;
}

int map_has(HashMap* map, Span key) {
    return map_get(map, key) != NULL;
}
