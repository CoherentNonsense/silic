#include "os.h"

#include <stdio.h>
#include <stdlib.h>

bool read_file(const char* path, char** buffer, int* length) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        return false;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        return false;
    }

    long buffer_length = ftell(file);
    if (buffer_length == -1) {
        return false;
    }

    *buffer = malloc(sizeof(char) * buffer_length + 1);

    if (fseek(file, 0, SEEK_SET) != 0) {
        free(*buffer);
        return false;
    }

    size_t file_read = fread(*buffer, sizeof(char), buffer_length, file);
    if (ferror(file) != 0) {
        return false;
    }

    *length = file_read;
    (*buffer)[file_read] = 0;

    fclose(file);

    return true;
}
