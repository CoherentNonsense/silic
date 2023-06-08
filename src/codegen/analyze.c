#include "analyze.h"

#include "codegen.h"
#include "list.h"
#include "hashmap.h"

static void analyze_extern_function(CodegenContext* context, AstNode* extern_fn) {
    AstNode* fn_proto = extern_fn->data.extern_fn.prototype;
    String name = fn_proto->data.fn_proto.name;

    if (map_has(&context->function_map, name)) {
        sil_panic("Multiple function definitions: %.*s", name.length, name.data);
    }
    map_insert(&context->function_map, name, extern_fn);
}

static void analyze_function_definition(CodegenContext* context, AstNode* fn) {
    AstNode* fn_proto = fn->data.fn.prototype;
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
                analyze_function_definition(context, item);
                break;
            case AstNodeType_ExternFn:
                analyze_extern_function(context, item);
                break;
            default:
                sil_panic("Code Gen Error: Could not analyze root function");
        }
    }
}
