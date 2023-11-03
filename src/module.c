#include "module.h"

#include <stdarg.h>

#define RESET "\033[0m"
#define ERROR_RED "\033[1;31m"
#define PATH_BLUE "\033[34m"
#define UNDERLINE "\033[4m"

void module_init(Module* module, Span path, Span source) {
    module->path = path;
    module->source = source;
    symtable_init(&module->symbol_table);
    dynarray_init(module->errors);
    module->has_errors = false;
}

void module_add_error(Module* module, Token* token, const char* hint, const char* message, ...) {
    module->has_errors= true;

    // format message
    size_t max_length = strlen(message) + 200;
    char* formatted_message = malloc(max_length);

    va_list args;
    va_start(args, message);
    vsnprintf(formatted_message, max_length, message, args);
    va_end(args);

    // hint
    bool has_hint = true;
    if (hint == NULL) {
        has_hint = false;
        hint = "";
    }

    // add error
    ModuleError error = { token, formatted_message, hint, has_hint };
    dynarray_push(module->errors, error);
}

void module_display_errors(Module* module) {
    for (size_t i = 0; i < module->errors.length; i += 1) {
        ModuleError* error = dynarray_get_ref(module->errors, i);
        fprintf(stderr, ERROR_RED "\nerror:" RESET " %s\n", error->message);

        // print source
        TextPosition position = error->token->position;
        int line_num_width = 0;
        for (int i = error->token->position.line; i > 0; i /= 10) { line_num_width += 1; }
        fprintf(stderr, "    ┌ " PATH_BLUE "%.*s:%d:%d" RESET " ──\n    │\n", (int)module->path.length, module->path.start, position.line, position.column);
        fprintf(stderr, "%.*s" UNDERLINE "%.*d" RESET " │ ", 3 - line_num_width, "  ", line_num_width, error->token->position.line);
        char* start = (char*)error->token->span.start - (position.column - 1);
        while (*start != '\n' && *start != 0) {
            if (start == error->token->span.start) { fprintf(stderr, ERROR_RED); }
            putc(*start, stderr);
            if (start == error->token->span.start) { fprintf(stderr, RESET); }
            start += 1;
        }
        fprintf(stderr, "\n    │ ");
        for (size_t i = 0; i < position.column - 1; i++) { putc(' ', stderr); }
        fprintf(stderr, ERROR_RED "^ %s" RESET "\n    └\n", error->hint);
    }
}
