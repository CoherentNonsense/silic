#ifndef HASHMAP_H
#define HASHMAP_H

#include "parser.h"
#include "list.h"
#include "string.h"
#include "llvm-c/Types.h"

#include <stddef.h>

typedef struct Entry {
    String key;
    AstNode* value;
} Entry;

typedef struct HashMap {
    List entries;
} HashMap;

void map_delete(HashMap* map);
void map_insert(HashMap* map, String key, AstNode* value);
AstNode* map_get(HashMap* map, String key);
int map_has(HashMap* map,  String key);

#endif // !HASHMAP_H
