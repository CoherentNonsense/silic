#include "module.h"

void module_add_error(Module* module, Span error_message) {
    ModuleError error = { error_message };
    dynarray_push(module->errors, error);
}
