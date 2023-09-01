#include "span.h"

void span_print(Span span) {
    printf("%.*s", (int)span.length, span.start);
}

void span_println(Span span) {
    printf("%.*s\n", (int)span.length, span.start);
}
