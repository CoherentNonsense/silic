#ifndef HASHMAP_H
#define HASHMAP_H

#include "list.h"
#include "string_buffer.h"
#include "llvm-c/Types.h"

#include <stddef.h>

typedef struct Entry {
    String key;
    void* value;
} Entry;

typedef struct HashMap {
    List entries;
} HashMap;

void map_delete(HashMap* map);
void map_insert(HashMap* map, String key, void* value);
void* map_get(HashMap* map, String key);
int map_has(HashMap* map,  String key);

#endif // !HASHMAP_H
