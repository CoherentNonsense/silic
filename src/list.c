#include "list.h"
#include "util.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

List list_init() {
    return (List){0};
}

void list_deinit(List* list) {
    free(list->data);
}

void list_resize(size_t data_size, List* list, size_t size) {
    list->capacity = size;
    if (list->length > list->capacity) {
        list->length = list->capacity;
    }

    list->data = realloc(list->data, data_size * size);
}

void* list_add(size_t data_size, List* list) {
    if (list->length == list->capacity) {
        list->capacity *= 2;
        list->capacity += 8;
        list->data = realloc(list->data, data_size * list->capacity);
    }

    list->length += 1;
    return list_get(data_size, list, list->length - 1);
}

void list_push(size_t data_size, List* list, void* element) {
    void* new_element = list_add(data_size, list);
    memcpy(new_element, element, data_size);
}

void* list_get(size_t data_size, List* list, size_t index) {
    size_t data_index = index * data_size;
    return (void*)((char*)list->data + data_index);
}
