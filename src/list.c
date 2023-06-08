#include "list.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const size_t INITIAL_CAPACITY = 1;

void list_delete(List* list) {
    free(list->data);
}

void* list_add_generic(size_t size, List* list) {
    if (list->length == list->capacity) {
        list->capacity *= 2;
        list->capacity += 8;
        list->data = realloc(list->data, size * list->capacity);
    }

    list->length += 1;
    return list_get_generic(size, list, list->length - 1);
}

void list_push_generic(size_t size, List* list, void* element) {
    void* new_element = list_add_generic(size, list);
    memcpy(new_element, element, size);
}

void* list_get_generic(size_t size, List* list, size_t index) {
    size_t data_index = index * size;
    return (void*)((char*)list->data + data_index);
}

size_t list_length(List *list) {
    return list->length;
}
