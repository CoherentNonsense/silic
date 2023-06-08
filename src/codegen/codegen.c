#include "codegen.h"

#include "codegen/analyze.h"
#include "list.h"
#include "parser/expression.h"
#include "parser/parser.h"
#include "string_buffer.h"
#include "util.h"

#include "llvm-c/Core.h"
#include "llvm-c/Types.h"
#include <stdio.h>
#include <stdlib.h>


static LLVMTypeRef to_llvm_type(AstNode* type_name) {
    if (type_name->data.type_name.type == AstNodeTypeNameType_Pointer) {
        return LLVMPointerType(to_llvm_type(type_name->data.type_name.child_type), 0);
    }

    switch (type_name->data.type_name.primitive) {
        case AstTypeName_unreachable: return LLVMVoidType();
        case AstTypeName_void: return LLVMVoidType();
        case AstTypeName_i32: return LLVMInt32Type();
        case AstTypeName_u8: return LLVMInt8Type();
        default: sil_panic("Code Gen Error: Cannot convert sil type to LLVM type");
    }
}

static LLVMValueRef codegen_expression(CodegenContext* context, AstNode* expression);

static LLVMValueRef codegen_fn_call(CodegenContext* context, AstNode* fn_call) {
    String name = fn_call->data.expression_function.name;

    AstNode* fn = map_get(&context->function_map, name);
    if (fn == NULL) {
        sil_panic("Function not defined %.*s", name.length, name.data);
    }

    LLVMValueRef fn_ref = LLVMGetNamedFunction(context->module, name.data);
    AstNode* fn_proto = fn->data.fn.prototype;

    List* parameter_list = &fn_call->data.expression_function.parameters;
    int param_count = fn_call->data.expression_function.parameters.length;
    if (param_count != fn->data.fn.prototype->data.fn_proto.parameters.length) {
        sil_panic("Wrong number of arguments");
    }

    LLVMValueRef* parameters = malloc(sizeof(LLVMValueRef) * param_count);
    for (int i = 0; i < param_count; i++) {
        parameters[i] = codegen_expression(context, *list_get(AstNode*, parameter_list, i));
    }


    LLVMValueRef call_ref = LLVMBuildCall2(
        context->builder,
        fn_proto->data.fn_proto.llvm_fn_type,
        fn_ref,
        parameters,
        param_count,
        ""
    );

    free(parameters);

    return call_ref;
}

static LLVMValueRef codegen_expression(CodegenContext* context, AstNode* expression) {
    switch (expression->type) {
        case AstNodeType_ExpressionFunction:
            return codegen_fn_call(context, expression);
        case AstNodeType_ExpressionString: {
            String string_text = expression->data.expression_string.value;
            LLVMValueRef string_global = LLVMBuildGlobalString(context->builder, string_text.data, "");
            return LLVMBuildPointerCast(
                context->builder,
                string_global,
                LLVMPointerType(LLVMInt8Type(), 0),
                ""
            );
        }
        case AstNodeType_ExpressionNumber: {
            String number_text = expression->data.expression_number.value;
            return LLVMConstIntOfStringAndSize(LLVMInt32Type(), number_text.data, number_text.length, 10);
        }
        case AstNodeType_UnaryOperator: {
            LLVMValueRef value = codegen_expression(
                context,
                expression->data.unary_operator.value
            );

            switch (expression->data.unary_operator.type) {
                case UnaryOperatorType_Negation:
                    return LLVMBuildNeg(context->builder, value, "");
                default:
                    sil_panic("Code Gen Error: Unhandled unary operator");
            }
        }
        case AstNodeType_InfixOperator: {
            LLVMValueRef left = codegen_expression(
                context,
                expression->data.infix_operator.left
            );
            LLVMValueRef right = codegen_expression(
                context,
                expression->data.infix_operator.right
            );

            switch (expression->data.infix_operator.type) {
                case BinaryOperatorType_Addition:
                    return LLVMBuildAdd(context->builder, left, right, "");
                case BinaryOperatorType_Subtraction:
                    return LLVMBuildSub(context->builder, left, right, "");
                case BinaryOperatorType_Multiplication:
                    return LLVMBuildMul(context->builder, left, right, "");
                case BinaryOperatorType_Division:
                    return LLVMBuildSDiv(context->builder, left, right, "");
                default:
                    sil_panic("Code Gen Error: Unhandled infix operator");
            }
        }
        default:
            sil_panic("Code Gen Error: Invalid expression");
    }
}

static void codegen_statement(CodegenContext* context, AstNode* statement) {
    switch (statement->type) {
        case AstNodeType_StatementReturn: {
            LLVMValueRef return_value = codegen_expression(context, statement->data.statement_return.expression);
            LLVMBuildRet(context->builder, return_value);
            break;
        } 
        case AstNodeType_StatementExpression:
            codegen_expression(context, statement->data.statement_expression.expression);
            break;
        default:
            sil_panic("Code Gen Error: Expected statement");
    }
}

static void codegen_block(CodegenContext* context, AstNode* block) {
    List* statement_list = &block->data.block.statement_list;
    for (int i = 0; i < statement_list->length; i++) {
        AstNode* statement = *list_get(AstNode*, statement_list, i);
        codegen_statement(context, statement);
    }
}

static LLVMValueRef codegen_fn_proto(CodegenContext* context, AstNode* fn_proto) {
    String name = fn_proto->data.fn_proto.name;
    LLVMTypeRef return_type = to_llvm_type(fn_proto->data.fn_proto.return_type);
    List* parameters = &fn_proto->data.fn_proto.parameters;
    LLVMTypeRef* param_types = malloc(sizeof(LLVMTypeRef) * parameters->length);
    for (int i = 0; i < parameters->length; i++) {
        AstNode* parameter = *list_get(AstNode*, parameters, i);
        param_types[i] = to_llvm_type(parameter->data.pattern.type);
    }
    
    LLVMTypeRef function_type = LLVMFunctionType(return_type, param_types, parameters->length, 0);
    LLVMValueRef function = LLVMAddFunction(context->module, name.data, function_type);

    fn_proto->data.fn_proto.llvm_fn_type = function_type;

    free(param_types);

    return function;
}

static void codegen_extern_fn(CodegenContext* context, AstNode* extern_fn) {
    AstNode* fn_proto = extern_fn->data.extern_fn.prototype;
    LLVMValueRef function = codegen_fn_proto(context, fn_proto);

    LLVMSetLinkage(function, LLVMExternalLinkage);
    LLVMSetFunctionCallConv(function, LLVMCCallConv);
}

static void codegen_fn(CodegenContext* context, AstNode* fn) {
    AstNode* fn_proto = fn->data.fn.prototype;
    LLVMValueRef function = codegen_fn_proto(context, fn_proto);

    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(function, "entry");
    LLVMPositionBuilderAtEnd(context->builder, entry);

    codegen_block(context, fn->data.fn.body);
}

static void codegen_root(CodegenContext* context) {
    for (int i = 0; i < context->function_map.entries.length; i++) {
        Entry* entry = list_get(Entry, &context->function_map.entries, i);
        AstNode* node = entry->value;
        switch (node->type) {
            case AstNodeType_ExternFn:
                codegen_extern_fn(context, node);
                break;
            case AstNodeType_Fn:
                codegen_fn(context, node);
                break;
            default:
                sil_panic("Code Gen Error: Unexpected Node in root");
            
        }    
    }
    
}

void codegen_generate(AstNode* ast) {
    CodegenContext context = {0};
    context.current_node = ast;

    context.builder = LLVMCreateBuilder();
    context.module = LLVMModuleCreateWithName("SilModule");

    codegen_analyze(&context, ast);

    codegen_root(&context);

    LLVMDumpModule(context.module);
    // LLVMPrintModuleToFile(context.module, "hello.ll", NULL);
}
