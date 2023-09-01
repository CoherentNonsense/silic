#include "compiler.h"

#include "token.h"
#include "lexer.h"
#include "parser.h"


Module compiler_compile_module(Span source) {
    Module module;
    module.source = source;

    // ------ //
    // Lexing //
    printf("Lexing File...\n");
    TokenList token_list = lexer_lex(source);

    list_foreach(token_list, token) {
        printf("%s: ", token_string(token->kind));
	token_print(token);
        printf("\n");
    }


    // ------- //
    // Parsing //
    printf("\nParsing Tokens...\n");
    module.ast = parser_parse(source, token_list);

    ast_print(module.ast);
    
    list_deinit(token_list);

    return module;
}
