#include "compiler.h"
#include "module.h"
#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "string.h"
#include "hashmap.h"
#include "util.h"
#include "os.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iso646.h>

#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define VERSION_PATCH 0


static void print_usage(char* command) {
    fprintf(stderr, "\nUsage: %s <code>.sil\n\nOther Options:\n--version\t\tprints version\n--output <outfile>\tsets output file\n--build\tbuild the C(IR)\n\n", command);
}

int main(int argc, char** argv) {
    char* arg0 = argv[0];
    char* in_file_path = 0;
    char* out_file_path = "output";
    bool build = false;
    bool debug_info = false;

    for (int i = 1; i < argc; i++) {
        char* arg = argv[i];

        if ((arg[0] == '-') and (arg[1] == '-')) {
            if (strcmp(arg, "--version") == 0) {
                printf("%d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
                return 0;
            } else if(strcmp(arg, "--output") == 0) {
                i += 1;
                out_file_path = argv[i];
	    } else if (strcmp(arg, "--build") == 0) {
		build = true;
            } else if (strcmp(arg, "--debug") == 0) {
                debug_info = true;
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

    Span path = (Span){ in_file_path, strlen(in_file_path) };
    Span source = (Span){ buffer, length };

    compiler_compile_module(path, source, build, debug_info);

    free(buffer);

    return EXIT_SUCCESS;
}
