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

#define dynarray_init(arr) arr = (__typeof__(arr)){0}

#define dynarray_deinit(arr) free((arr).data)

#define dynarray_resize(arr, new_capacity) \
    { \
	(arr).capacity = new_capacity; \
	if ((arr).length > (arr).capacity) { \
	    (arr).length = (arr).capacity; \
	} \
	(arr).data = realloc((arr).data, sizeof(*((arr).data)) * (arr).capacity); \
    }

#define dynarray_reserve(arr, n) \
    { \
	(arr).length += n; \
	if ((arr).length > (arr).capacity) { \
	    (arr).capacity = (arr).length; \
	    dynarray_resize(arr, (arr).capacity); \
	} \
    }

#define dynarray_push(arr, element) \
    { \
	if ((arr).length == (arr).capacity) { \
	    (arr).capacity *= 2; \
	    (arr).capacity += 8; \
	    dynarray_resize((arr), (arr).capacity); \
	} \
	(arr).data[(arr).length] = element; \
	(arr).length += 1; \
    }

#define dynarray_get(arr, i) (arr).data[i]

#define dynarray_get_ref(arr, i) &(arr).data[i]

#define dynarray_foreach(arr, elem) \
    for (int i = 0, once = 1; i < (arr).length; once = 1, i++) \
    for (__typeof__((arr).data) elem = dynarray_get_ref(arr, i); once; once = 0)

#endif // !LIST_H
