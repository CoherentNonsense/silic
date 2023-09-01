#ifndef SPAN_H
#define SPAN_H

#include <stdlib.h>
#include <stdio.h>

typedef struct Span {
    char* start;
    size_t length;
} Span;

void span_print(Span span);
void span_println(Span span);

#endif
