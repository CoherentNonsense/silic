#ifndef HASHMAP_H
#define HASHMAP_H

#include "dynarray.h"
#include "span.h"
#include "util.h"
#include <stdbool.h>

typedef struct Entry {
    Span key;
    void* value;
    char used;
} Entry;

typedef struct HashMap {
    DynArray(Entry) entries;
} HashMap;

typedef Maybe(void*) MaybeAny;

void map_init(HashMap* map);
void map_deinit(HashMap* map);
void map_insert(HashMap* map, Span key, void* value);
MaybeAny map_get(HashMap const* const map, Span const key);
bool map_has(HashMap const* const map,  Span key);

#define map_iterate(map, val, cb) for (int i  = 0; i < map.entries.capacity; i++) {\
	Entry* entry = dynarray_get_ref(map.entries, i); \
        val = entry->value; \
        if (!entry->used) {\
            continue;\
        }\
        cb\
    }

#endif // !HASHMAP_H
