#ifndef LIST_H
#define LIST_H

#include <stddef.h>

typedef struct List {
    size_t length;
    size_t capacity;
    void* data;
} List;

List list_init();
void list_deinit(List* list);
void list_resize(size_t data_size, List* list, size_t size);
void* list_add(size_t data_size, List* list);
void list_push(size_t data_size, List* list, void* element);
void* list_get(size_t data_size, List* list, size_t index);

#endif // !LIST_H
