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
    printf(BOLDWHITE "\nLexing File...\n" RESET);
    lexer_lex(module);

    dynarray_foreach(module->token_list, token) {
        printf("%s: " YELLOW, token_string(token->kind));
	token_print(token);
        printf(RESET "\n");
    }
    printf(BOLDWHITE "Lexed File.\n" RESET);


    // ------- //
    // Parsing //
    printf(BOLDWHITE "\nParsing Tokens...\n" RESET);
    parser_parse(module); 

    ast_print(module->ast);
    printf(BOLDWHITE "Parsed Tokens.\n" RESET);

   
    // --------- //
    // Analyzing //
    printf(BOLDWHITE "\nAnalyzing AST...\n" RESET);
    printf(BOLDWHITE "Analyzed Ast.\n" RESET);


    // ------- //
    // Codegen //
    printf(BOLDWHITE "\nGenerating IR...\n" RESET);
    c_codegen_generate(module);
    printf(BOLDWHITE "Generated IR.\n" RESET);


    return module;
}
