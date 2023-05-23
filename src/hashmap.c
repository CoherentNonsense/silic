#include "hashmap.h"
#include "list.h"
#include "string.h"
#include <stdlib.h>
#include <string.h>

void map_delete(HashMap* map) {
    list_delete(&map->entries);
}

void map_insert(HashMap* map, String key, AstNode* value) {
    Entry* entry = list_add(Entry, &map->entries);
    entry->key = key;
    entry->value = value;
}

// i'm not using a hash function shhhhh
AstNode* map_get(HashMap* map, String key) {
    for (int i = 0; i < map->entries.length; i++) {
        Entry* entry = list_get(Entry, &map->entries, i);
        if (!strncmp(key.data, entry->key.data, key.length)) {
            return entry->value;
        }
    }

    return NULL;
}

int map_has(HashMap* map, String key) {
    for (int i = 0; i < map->entries.length; i++) {
        Entry* entry = list_get(Entry, &map->entries, i);
        if (!strncmp(key.data, entry->key.data, key.length)) {
            return 1;
        }
    }

    return 0;
}
