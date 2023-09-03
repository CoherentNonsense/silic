#include "compiler.h"
#include "module.h"
#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include "string.h"
#include "list.h"
#include "hashmap.h"
#include "util.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iso646.h>

#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define VERSION_PATCH 0


static Result read_file(const char* path, char** buffer, int* length) {
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

static void print_usage(char* command) {
    fprintf(stderr, "\nUsage: %s <code>.sil\n\nOther Options:\n--version\t\tprints version\n--output <outfile>\tsets output file\n\n", command);
}

int main(int argc, char** argv) {
    char* arg0 = argv[0];
    char* in_file_path = 0;
    char* out_file_path = "output";

    for (int i = 1; i < argc; i++) {
        char* arg = argv[i];

        if ((arg[0] == '-') and (arg[1] == '-')) {
            if (strcmp(arg, "--version") == 0) {
                printf("%d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
                return 0;
            } else if(strcmp(arg, "--output") == 0) {
                i += 1;
                out_file_path = argv[i];
            } else {
                print_usage(arg0);
                return EXIT_FAILURE;
            }
        } else if (in_file_path == 0) {
            in_file_path = arg;
        } else {
            print_usage(arg0);
            return EXIT_FAILURE;
        }
    }

    if ((in_file_path == 0) or (out_file_path == 0)) {
        print_usage(arg0);
        return EXIT_FAILURE;
    }

    char* buffer;
    int length;
    Result read_file_result = read_file(in_file_path, &buffer, &length);

    if (read_file_result.type != Ok) {
        result_print(read_file_result);
        return EXIT_FAILURE;
    }

    Span source = (Span){ buffer, length };

    Module module = compiler_compile_module(source);

    free(buffer);

    return EXIT_SUCCESS;
}
