#ifndef LIST_H
#define LIST_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define DynArray(type) \
    struct { \
	size_t length; \
	size_t capacity; \
	type* data; \
    }

typedef DynArray(void) DynArrayVoid;

// init
#define dynarray_init(array) array = (__typeof__(array)){0}

// deinit
#define dynarray_deinit(array) free((array).data)

// resize
void dynarray_resize__polymorphic(
    DynArrayVoid* const array,
    size_t const data_size,
    size_t const new_capacity
);
#define dynarray_resize(array, new_capacity) \
    dynarray_resize__polymorphic((DynArrayVoid*)&(array), sizeof(*(array).data), new_capacity)

// reserve
void dynarray_reserve__polymorphic(
    DynArrayVoid* const array,
    size_t const data_size,
    size_t const n
);
#define dynarray_reserve(array, n) \
    dynarray_reserve__polymorphic((DynArrayVoid*)&(array), sizeof(*(array).data), n)

// push
void dynarray_push__polymorphic(
    DynArrayVoid* const array,
    size_t const data_size,
    void const* const element
);
#define dynarray_push(array, element) do { \
	__typeof__(*(array).data) temp = (element); \
	dynarray_push__polymorphic((DynArrayVoid*)&(array), sizeof(*(array).data), &temp); \
    } while(0)

#define dynarray_get_ref(array, index) \
    ((__typeof__((array).data))((array).data + (index)))

#define dynarray_get(array, index) *dynarray_get_ref((array), index)

#define dynarray_last_ref(array) dynarray_get_ref((array), (array).length - 1)

#endif // !LIST_H
