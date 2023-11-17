#ifndef MODULE_H
#define MODULE_H

#include "ast.h"
#include "token.h"
#include "symtable.h"
#include "typetable.h"
#include <chnlib/dynarray.h>
#include <chnlib/map.h>


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
    String path;
    String source;
    DynArray(Token) token_list;
    AstRoot* ast;

    Map(Item*) items;
    Map(type_id) types;
    SymTable symbol_table;
    TypeTable type_table;

    struct {
        type_id entry_void;
        type_id entry_never;

        type_id entry_c_char;
        type_id entry_c_str;

        type_id entry_bool;

        type_id entry_usize;
        type_id entry_isize;
        
        type_id entry_u8;
        type_id entry_u16;
        type_id entry_u32;
        type_id entry_u64;

        type_id entry_i8;
        type_id entry_i16;
        type_id entry_i32;
        type_id entry_i64;
    } primitives;

    bool has_errors;
    DynArray(ModuleError) errors;
} Module;


void module_init(Module* module, String path, String source);
void module_deinit(Module* module);

__attribute__((format(printf, 4, 5)))
void module_add_error(Module* module, Token* token, const char* hint, const char* message, ...);

void module_display_errors(Module* module);

#endif //!MODULE_H
