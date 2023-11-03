#ifndef SPAN_H
#define SPAN_H

#include <stdlib.h>
#include <stdio.h>

typedef struct Span {
    const char* start;
    size_t length;
} Span;

#define span_from_literal(literal) (Span){ literal, sizeof(literal) - 1 }

void span_append(Span dst, Span src);

void span_print(Span span);
void span_println(Span span);

#endif
