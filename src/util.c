#include "util.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>


void sil_panic(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(EXIT_FAILURE);
}
