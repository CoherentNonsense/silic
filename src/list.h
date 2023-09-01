#ifndef LIST_H
#define LIST_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REGISTER_LIST(type) \
    typedef struct type##List{ \
	size_t length; \
	size_t capacity; \
	type* data; \
    } type##List

#define REGISTER_LIST_PTR(type) \
    typedef struct type##PtrList { \
	size_t length; \
	size_t capacity; \
	type** data; \
    } type##PtrList

#define list_init(list) list = (__typeof__(list)){0}

#define list_deinit(list) free((list).data)

#define list_resize(list, new_capacity) \
    { \
	(list).capacity = new_capacity; \
	if ((list).length > (list).capacity) { \
	    (list).length = (list).capacity; \
	} \
	(list).data = realloc((list).data, sizeof(*((list).data)) * (list).capacity); \
    }

#define list_reserve(list, n) \
    { \
	(list).length += n; \
	if ((list).length > (list).capacity) { \
	    (list).capacity = (list).length; \
	    list_resize(list, (list).capacity); \
	} \
    }

#define list_push(list, element) \
    { \
	if ((list).length == (list).capacity) { \
	    (list).capacity *= 2; \
	    (list).capacity += 8; \
	    list_resize((list), (list).capacity); \
	} \
	(list).data[(list).length] = element; \
	(list).length += 1; \
    }

#define list_get(list, i) (list).data[i]

#define list_get_ref(list, i) &(list).data[i]

#define list_foreach(list, elem) \
    for (int i = 0, once = 1; i < (list).length; once = 1, i++) \
    for (__typeof__((list).data) elem = list_get_ref(list, i); once; once = 0)

#endif // !LIST_H
