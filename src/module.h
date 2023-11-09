#ifndef MODULE_H
#define MODULE_H

#include "ast.h"
#include "token.h"
#include "symtable.h"
#include "typetable.h"
#include "hashmap.h"
#include "dynarray.h"

typedef enum ModuleErrorType {
    ModuleErrorType_Warning,
    ModuleErrorType_Error,
} ModuleErrorType;

typedef struct ModuleError {
    Token* token;
    ModuleErrorType type;
    const char* message;
    const char* hint;
    bool has_hint;
} ModuleError;

typedef struct Module {
    Span path;
    Span source;
    DynArray(Token) token_list;
    AstRoot* ast;

    HashMap(Item*) items;
    HashMap(TypeEntry*) types;
    SymTable symbol_table;
    TypeTable type_table;

    struct {
        TypeEntry* entry_void;

        TypeEntry* entry_c_char;
        
        TypeEntry* entry_u8;
        TypeEntry* entry_u16;
        TypeEntry* entry_u32;
        TypeEntry* entry_u64;

        TypeEntry* entry_i8;
        TypeEntry* entry_i16;
        TypeEntry* entry_i32;
        TypeEntry* entry_i64;
    } primitives;

    bool has_errors;
    DynArray(ModuleError) errors;
} Module;

// yucky
#define RET_ON_ERR(context) do { if (context->module->has_errors) { return NULL; }  } while(0)

void module_init(Module* module, Span path, Span source);
void module_deinit(Module* module);

__attribute__((format(printf, 4, 5)))
void module_add_error(Module* module, Token* token, const char* hint, const char* message, ...);

void module_display_errors(Module* module);

#endif //!MODULE_H
