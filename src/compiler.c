#include "compiler.h"

#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "analyzer.h"
#include "c_codegen.h"
#include "util.h"
#include "os.h"
#include <chnlib/logger.h>
#include <stdio.h>


Module* compiler_compile_module(String path, String source, bool build, bool debug_info) {
    Module* module = malloc(sizeof(Module));
    module_init(module, path, source);

    

    // ------ //
    // Lexing //
    lexer_lex(module);

    if (module->has_errors) {
        module_display_errors(module);
        return null;
    }

    // print tokens
    if (debug_info) {
        printf("Tokens\n------\n");
        for (usize i = 0; i < dynarray_len(module->token_list); i++) {
            Token* token = &module->token_list[i];
            printf("%s: " YELLOW, token_string(token->kind));
            token_print(token);
            printf(RESET "\n");
        }
    }

    // ------- //
    // Parsing //
    if (debug_info) {
        printf("Parsing...\n");
    }
    parser_parse(module);

    if (module->has_errors) {
        module_display_errors(module);
        return null;
    }

    if (debug_info) {
        printf(BOLDWHITE "Finished parsing\n---\n" RESET);
    }

   
    // --------- //
    // Analyzing //
    if (debug_info) {
        printf(BOLDWHITE "Analyzing AST\n" RESET);
    }
    analyzer_analyze(module);
    if (debug_info) {
        printf("Analyzed AST\n");
    }

    // ------- //
    // Codegen //
    if (debug_info) {
        printf(BOLDWHITE "Generating IR\n" RESET);
    }

    String ir = c_codegen_generate(module);

    char* prelude_text;
    int prelude_length;
    bool read_success = read_file("prelude.c", &prelude_text, &prelude_length);
    if (not read_success) {
        printf("Failed to load 'prelude.c'");
        return null;
    }

    FILE* out_file = fopen("build/ir.c", "wb");
    if (out_file == null) {
	    printf("Could not create ir\n");
	    return null;
    }

    fwrite(prelude_text, prelude_length, 1, out_file);
    fwrite(ir.ptr, ir.len, 1, out_file);

    fclose(out_file);

    if (build) {
	system("gcc -nostartfiles -O2 build/ir.c -o app");
    }

    if (debug_info) {
        printf(BOLDWHITE "Generated IR.\n" RESET);
    }


    return module;
}
