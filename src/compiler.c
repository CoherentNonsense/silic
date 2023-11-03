#include "compiler.h"

#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "analyzer.h"
#include "c_codegen.h"
#include "util.h"


Module* compiler_compile_module(Span path, Span source, bool build, bool debug_info) {
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
        for (size_t i = 0; i < module->token_list.length; i++) {
            Token* token = dynarray_get_ref(module->token_list, i);
            printf("%s: " YELLOW, token_string(token->kind));
            token_print(token);
            printf(RESET "\n");
        }
    }


    // ------- //
    // Parsing //
    parser_parse(module); 

    if (debug_info) {
        printf(BOLDWHITE "AST\n---\n" RESET);
        ast_print(module->ast);
    }

   
    // --------- //
    // Analyzing //
    analyzer_analyze(module);
    if (debug_info) {
        printf("Analyzed AST\n");
    }

    // ------- //
    // Codegen //
    c_codegen_generate(module, build);
    if (debug_info) {
        printf(BOLDWHITE "Generated IR.\n" RESET);
    }


    return module;
}
