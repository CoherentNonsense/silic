#ifndef HASHMAP_H
#define HASHMAP_H

#include "list.h"
#include "span.h"


typedef struct Entry {
    Span key;
    void* value;
    char used;
} Entry;
REGISTER_LIST(Entry);

typedef struct HashMap {
    EntryList entries;
} HashMap;

void map_delete(HashMap* map);
void map_insert(HashMap* map, Span key, void* value);
void* map_get(HashMap* map, Span key);
int map_has(HashMap* map,  Span key);

#define map_iterate(map, entry, cb) for (int i  = 0; i < map.entries.capacity; i++) {\
        Entry* entry = list_get_generic(sizeof(Entry), &map.entries, i);\
        if (!entry->used) {\
            continue;\
        }\
        cb\
    }

#endif // !HASHMAP_H
