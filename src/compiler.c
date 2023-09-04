#include "compiler.h"

#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "c_codegen.h"
#include "util.h"


Module* compiler_compile_module(Span source) {
    Module* module = malloc(sizeof(Module));
    module->source = source;


    // ------ //
    // Lexing //
    printf(BOLDWHITE "Lexing File...\n" RESET);
    lexer_lex(module);

    dynarray_foreach(module->token_list, token) {
        printf("%s: ", token_string(token->kind));
	token_print(token);
        printf("\n");
    }


    // ------- //
    // Parsing //
    printf(BOLDWHITE "\nParsing Tokens...\n" RESET);
    parser_parse(module); 

    ast_print(module->ast);


    // ------- //
    // Codegen //
    printf(BOLDWHITE "\nGENERATING IR...\n" RESET);
    c_codegen_generate(module);


    return module;
}
