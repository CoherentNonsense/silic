#ifndef HASHMAP_H
#define HASHMAP_H

#include "dynarray.h"
#include "span.h"

typedef struct Entry {
    Span key;
    void* value;
    char used;
} Entry;

typedef struct HashMap {
    DynArray(Entry) entries;
} HashMap;

void map_init(HashMap* map);
void map_delete(HashMap* map);
void map_insert(HashMap* map, Span key, void* value);
void* map_get(HashMap* map, Span key);
int map_has(HashMap* map,  Span key);

#define map_iterate(map, val, cb) for (int i  = 0; i < map.entries.capacity; i++) {\
	Entry* entry = dynarray_get_ref(map.entries, i); \
        val = entry->value; \
        if (!entry->used) {\
            continue;\
        }\
        cb\
    }

#endif // !HASHMAP_H
