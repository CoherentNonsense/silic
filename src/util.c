#include "util.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>


void result_print(Result result) {
    switch (result.type) {
        case Error:
            fprintf(stderr, "Error: %s\n", result.msg);
            break;
        default:
            break;
    }
}

void sil_panic(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
    abort();
}
