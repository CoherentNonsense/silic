#ifndef LIST_H
#define LIST_H

#include <stddef.h>

typedef struct List {
    size_t length;
    size_t capacity;
    void* data;
} List;

void list_delete(List* list);

void list_resize_generic(size_t data_size, List* list, size_t size);
#define list_resize(T, list, size) list_resize_generic(sizeof(T), list, size)

void* list_add_generic(size_t data_size, List* list);
#define list_add(T, list) (T*) list_add_generic(sizeof(T), list)

void list_push_generic(size_t data_size, List* list, void* element);
#define list_push(T, list, element) list_push_generic(sizeof(T), list, element)

void* list_get_generic(size_t data_size, List* list, size_t index);
#define list_get(T, list, index) (T*) list_get_generic(sizeof(T), list, index)

size_t list_length(List* list);




#endif // !LIST_H
