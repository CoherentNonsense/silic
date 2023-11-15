#include "module.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define RESET "\033[0m"
#define ERROR_RED "\033[1;31m"
#define OTHER_BLUE "\033[34m"
#define FADED "\033[90m"

void module_init(Module* module, String path, String source) {
    module->path = path;
    module->source = source;
    module->has_errors = false;

    symtable_init(&module->symbol_table);
    module->errors = dynarray_init();
    typetable_init(&module->type_table);
    module->types = map_init();
    module->items = map_init();
}

void module_deinit(Module* module) {
    symtable_deinit(&module->symbol_table);
    dynarray_deinit(module->errors);
    typetable_deinit(&module->type_table);
    map_deinit(module->types);
    map_deinit(module->items);
}

void module_add_error(Module* module, Token* token, const char* hint, const char* message, ...) {
    module->has_errors= true;

    // format message
    usize max_length = strlen(message) + 200;
    char* formatted_message = malloc(max_length);

    va_list args;
    va_start(args, message);
    vsnprintf(formatted_message, max_length, message, args);
    va_end(args);

    // hint
    bool has_hint = true;
    if (hint == null) {
        has_hint = false;
        hint = "";
    }

    // add error
    ModuleError error = { token, ModuleErrorType_Error, formatted_message, hint, has_hint };
    dynarray_push(module->errors, &error);
}

void module_display_errors(Module* module) {
    for (usize i = 0; i < dynarray_len(module->errors); i += 1) {
        ModuleError* error = &module->errors[i];
        fprintf(stderr, ERROR_RED "error:" RESET " %s\n", error->message);

        // TODO: absolutely hideous
        TextPosition position = error->token->position;
        int line_num_width = 0;
        for (int i = error->token->position.line; i > 0; i /= 10) { line_num_width += 1; }
        // print border
        fprintf(stderr, FADED "    ╭─[" RESET "%.*s:%d:%d" FADED "]───\n", str_format(module->path), position.line, position.column);
        fprintf(stderr, "%.*s%.*d │ " RESET, 3 - line_num_width, "  ", line_num_width, error->token->position.line);
        // print source
        char* start = (char*)error->token->span.ptr - (position.column - 1);
        while (*start != '\n' && *start != 0) {
            if (start == error->token->span.ptr) { fprintf(stderr, ERROR_RED); }
            putc(*start, stderr);
            if (start == error->token->span.ptr+ error->token->span.len - 1) { fprintf(stderr, RESET); }
            start += 1;
        }
        fprintf(stderr, FADED "\n    │ ");
        for (usize i = 0; i < position.column - 1; i++) { putc(' ', stderr); }
        fprintf(stderr, ERROR_RED);
        for (usize i = 0; i < error->token->span.len; i += 1) { fprintf(stderr, "^"); }
        fprintf(stderr, " %s" FADED "\n    ╰", error->hint);
        fprintf(stderr, "─────\n" RESET);
    }
}
