#include "hashmap.h"
#include "list.h"

#include <stdlib.h>

void map_delete(HashMap* map) {
    list_delete(&map->entries);
}

void map_insert(HashMap* map, String key, void* value) {
    Entry* entry = list_add(Entry, &map->entries);
    entry->key = key;
    entry->value = value;
}

// i'm not using a hash function shhhhh
void* map_get(HashMap* map, String key) {
    for (int i = 0; i < map->entries.length; i++) {
        Entry* entry = list_get(Entry, &map->entries, i);
        if (string_compare(key, entry->key)) {
            return entry->value;
        }
    }

    return NULL;
}

int map_has(HashMap* map, String key) {
    for (int i = 0; i < map->entries.length; i++) {
        Entry* entry = list_get(Entry, &map->entries, i);
        if (string_compare(key, entry->key)) {
            return 1;
        }
    }

    return 0;
}
