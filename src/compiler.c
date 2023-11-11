#include "compiler.h"

#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "analyzer.h"
#include "c_codegen.h"
#include "util.h"
#include <stdio.h>


Module* compiler_compile_module(String path, String source, bool build, bool debug_info) {
    Module* module = malloc(sizeof(Module));
    module_init(module, path, source);


    // ------ //
    // Lexing //
    lexer_lex(module);

    if (module->has_errors) {
        module_display_errors(module);
        return NULL;
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
    parser_parse(module);

    if (module->has_errors) {
        module_display_errors(module);
        return NULL;
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
    c_codegen_generate(module, build);
    if (debug_info) {
        printf(BOLDWHITE "Generated IR.\n" RESET);
    }


    return module;
}
