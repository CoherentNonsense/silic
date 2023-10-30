#include "dynarray.h"
#include "token.h"

#include <stdint.h>

void dynarray_resize__polymorphic(
    DynArrayVoid* const array,
    size_t const data_size,
    size_t const new_capacity
) {
    array->capacity = new_capacity;
    if (array->length > array->capacity) {
	array->length = array->capacity;
    }
    array->data = realloc(array->data, data_size * new_capacity);
}

void dynarray_reserve__polymorphic(
    DynArrayVoid* const array,
    size_t const data_size,
    size_t const n
) {
    array->length += n;
    if (array->length > array->capacity) {
	array->capacity = array->length;
	dynarray_resize__polymorphic(array, data_size, array->capacity);
    }
}

void dynarray_push__polymorphic(
    DynArrayVoid* const array,
    size_t const data_size,
    void const* const element
) {
    if (array->length == array->capacity) {
	array->capacity *= 2;
	array->capacity += 8;
	dynarray_resize__polymorphic(array, data_size, array->capacity);
    }
    memcpy((uint8_t*)array->data + (array->length * data_size), element, data_size);
    array->length += 1;
}
