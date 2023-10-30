#ifndef HASHMAP_H
#define HASHMAP_H

#include "dynarray.h"
#include "span.h"
#include "util.h"
#include <stdbool.h>

#define Entry(T) \
    struct { \
	Span key; \
	T value; \
	bool used; \
    }

#define HashMap(T) \
    struct { \
	DynArray(Entry(T)) entries; \
    }

typedef HashMap(void*) HashMapAny;

void map_insert__polymorphic(HashMapAny* map, Span key, void* value);
void* map_get__polymorphic(HashMapAny const* const map, Span const key);
bool map_has__polymorphic(HashMapAny const* const map, Span const key);

#define map_init(map) dynarray_init(map.entries);
#define map_deinit(map) dynarray_deinit(map.entries);
#define map_insert(map, key, value) map_insert__polymorphic((HashMapAny*)&map, key, value)
#define map_get(map, key) \
    ((__typeof__(map.entries.data->value))map_get__polymorphic((HashMapAny*)&map, key))
#define map_has(map, key) map_has__polymorphic((HashMapAny*)&map, key)

#define map_iterate(map, val, cb) for (size_t i  = 0; i < map.entries.capacity; i++) {\
	val = dynarray_get_ref(map.entries, i)->value; \
        if (!dynarray_get_ref(map.entries, i)->used) {\
            continue;\
        }\
        cb\
    }

#endif // !HASHMAP_H
