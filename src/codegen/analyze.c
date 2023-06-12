#include "analyze.h"

#include "codegen.h"
#include "list.h"
#include "hashmap.h"
#include <stdio.h>

static void analyze_function(CodegenContext* context, AstNode* fn) {
    AstNode* fn_proto = fn->data.extern_fn.prototype;
    String name = fn_proto->data.fn_proto.name;

    if (map_has(&context->function_map, name)) {
        sil_panic("Multiple function definitions: %.*s", name.length, name.data);
    }

    map_insert(&context->function_map, name, fn);
}


void codegen_analyze(CodegenContext *context, AstNode *root) {
    List* function_list = &root->data.root.function_list;
    for (int i = 0; i < function_list->length; i++) {
        AstNode* item = *list_get(AstNode*, function_list, i);
        switch (item->type) {
            case AstNodeType_Fn:
            case AstNodeType_ExternFn:
                analyze_function(context, item);
                break;
            default:
                sil_panic("Code Gen Error: Could not analyze root function");
        }
    }
}
