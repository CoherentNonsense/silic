#include "dynarray.h"
#include "token.h"
typedef DynArray(void) DynArrayVoid;

void dynarray_resize__polymorphic(
    void* const array,
    size_t const data_size,
    size_t const new_capacity
) {
    DynArrayVoid* const poly = (DynArrayVoid* const)array;

    poly->capacity = new_capacity;
    if (poly->length > poly->capacity) {
	poly->length = poly->capacity;
    }
    poly->data = realloc(poly->data, data_size * new_capacity);
}

void dynarray_reserve__polymorphic(
    void* const array,
    size_t const data_size,
    size_t const n
) {
    DynArrayVoid* const poly = (DynArrayVoid* const)array;
    
    poly->length += n;
    if (poly->length > poly->capacity) {
	poly->capacity = poly->length;
	dynarray_resize__polymorphic(array, data_size, poly->capacity);
    }
}

void dynarray_push__polymorphic(
    void* const array,
    size_t const data_size,
    void const* const element
) {
    DynArrayVoid* const poly = (DynArrayVoid* const)array;

    if (poly->length == poly->capacity) {
	poly->capacity *= 2;
	poly->capacity += 8;
	dynarray_resize__polymorphic(array, data_size, poly->capacity);
    }
    memcpy(poly->data + (poly->length * data_size), element, data_size);
    poly->length += 1;
}
