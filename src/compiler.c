#include "compiler.h"

#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "qbe_codegen.h"
#include "util.h"


Module compiler_compile_module(Span source) {
    Module module;
    module.source = source;

    // ------ //
    // Lexing //
    printf(BOLDWHITE "Lexing File...\n" RESET);
    TokenList token_list = lexer_lex(source);

    list_foreach(token_list, token) {
        printf("%s: ", token_string(token->kind));
	token_print(token);
        printf("\n");
    }


    // ------- //
    // Parsing //
    printf(BOLDWHITE "\nParsing Tokens...\n" RESET);
    module.ast = parser_parse(source, token_list); 
    list_deinit(token_list);

    ast_print(module.ast);


    // ------- //
    // Codegen //
    printf(BOLDWHITE "\nGENERATING QBE IR...\n" RESET);
    qbe_codegen_generate(&module);


    return module;
}
