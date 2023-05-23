#include "string.h"
#include <stdlib.h>
#include <string.h>

String string_from_buffer(char* start, const size_t length) {
    char* data = malloc(length + 1);
    strncpy(data, start, length);
    data[length] = 0;

    return (String){ data, length };
}

void string_delete(String a) {
    free(a.data);
}

int string_compare(const String a, const String b) {
    for (int i = 0; i < a.length; i++) {
        if (a.data[i] != b.data[i]) {
            return 0;
        }
    }

    return 1;
}

int string_compare_literal(const String a, const char* b) {
    for (int i = 0; i < a.length; i++) {
        if (a.data[i] != b[i]) {
            return 0;
        }
    }

    return 1;
}
