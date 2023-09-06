#include "os.h"

#include <stdio.h>
#include <stdlib.h>

Result read_file(const char* path, char** buffer, int* length) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
        return RESULT_ERR("Could not open file.");
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        return RESULT_ERR("Could not read file.");
    }

    long buffer_length = ftell(file);
    if (buffer_length == -1) {
        return RESULT_ERR("Could not read file.");
    }

    *buffer = malloc(sizeof(char) * buffer_length + 1);

    if (fseek(file, 0, SEEK_SET) != 0) {
        free(*buffer);
        return RESULT_ERR("Could not read file.");
    }

    size_t file_read = fread(*buffer, sizeof(char), buffer_length, file);
    if (ferror(file) != 0) {
        return RESULT_ERR("Could not read file.");
    }

    *length = file_read;
    (*buffer)[file_read] = 0;

    fclose(file);

    return RESULT_OK;
}
