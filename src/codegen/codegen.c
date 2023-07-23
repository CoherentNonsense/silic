#include "codegen.h"

#include "ast.h"
#include "codegen/analyze.h"
#include "codegen/environment.h"
#include "hashmap.h"
#include "list.h"
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
    String name = fn_call->data.primary_expression.function_call.name;

    AstNode* fn = map_get(&context->function_map, name);
    if (fn == NULL) {
        sil_panic("Function not defined %.*s", name.length, name.data);
    }

    LLVMValueRef fn_ref = LLVMGetNamedFunction(context->module, name.data);
    AstNode* fn_proto = fn->data.fn.prototype;

    List* parameter_list = &fn_call->data.primary_expression.function_call.parameters;
    int param_count = fn_call->data.primary_expression.function_call.parameters.length;
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

static LLVMValueRef codegen_primary_expression(CodegenContext* context, AstNode* primary) {
    switch (primary->data.primary_expression.type) {
        case PrimaryExpressionType_Number:  {
            String number_text = primary->data.primary_expression.number;
            return LLVMConstIntOfStringAndSize(LLVMInt32Type(), number_text.data, number_text.length, 10);
        }

        case PrimaryExpressionType_String: {
            String string_text = primary->data.primary_expression.string;
            LLVMValueRef string_global = LLVMBuildGlobalString(context->builder, string_text.data, "");
            return LLVMBuildPointerCast(
                context->builder,
                string_global,
                LLVMPointerType(LLVMInt8Type(), 0),
                ""
            );
        }

        case PrimaryExpressionType_Symbol: {
            Binding* binding = env_get_binding(context->current_env, primary->data.primary_expression.symbol);
            return LLVMBuildLoad2(
                context->builder,
                to_llvm_type(binding->type),
                binding->ptr,
                ""
            );
        }

        case PrimaryExpressionType_FunctionCall:
            return codegen_fn_call(context, primary);

        default:
            sil_panic("Code Gen Error: Unhandled primary expression");
    }
}

static LLVMValueRef codegen_expression(CodegenContext* context, AstNode* expression) {
    switch (expression->type) {
        case AstNodeType_PrimaryExpression:
            return codegen_primary_expression(context, expression);

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
        case AstNodeType_BinaryOperator: {
            LLVMValueRef left = codegen_expression(
                context,
                expression->data.binary_operator.left
            );
            LLVMValueRef right = codegen_expression(
                context,
                expression->data.binary_operator.right
            );

            switch (expression->data.binary_operator.type) {
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
        case AstNodeType_LetStatement: {
            AstNodeLetStatement* let_statement = &statement->data.let_statement;
            LLVMValueRef ptr = LLVMBuildAlloca(
                context->builder,
                to_llvm_type(let_statement->pattern->data.pattern.type),
                let_statement->pattern->data.pattern.name.data
            );
            LLVMBuildStore(
                context->builder,
                codegen_expression(context, let_statement->expression),
                ptr
            );
            // TODO: Move this to analyzer?
            env_add_binding(
                context->current_env,
                let_statement->pattern->data.pattern.name,
                let_statement->pattern->data.pattern.type,
                ptr
            );
            break;
        }
        default:
            sil_panic("Code Gen Error: Expected statement %d", statement->type);
    }
}

static void codegen_block(CodegenContext* context, AstNode* block) {
    Environment* block_env = env_new(context->current_env);
    context->current_env = block_env;

    List* statement_list = &block->data.block.statement_list;
    for (int i = 0; i < statement_list->length; i++) {
        AstNode* statement = *list_get(AstNode*, statement_list, i);
        codegen_statement(context, statement);
    }

    context->current_env = block_env->parent;
    env_delete(block_env);
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
    map_iterate(context->function_map, entry, {
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
    })
}

void codegen_generate(AstNode* ast) {
    CodegenContext context = {0};
    context.current_node = ast;

    context.builder = LLVMCreateBuilder();
    context.module = LLVMModuleCreateWithName("SilModule");

    codegen_analyze(&context, ast);

    Environment* global_env = env_new(NULL);
    context.current_env = global_env;

    codegen_root(&context);

    env_delete(global_env);

    LLVMDumpModule(context.module);
    // LLVMPrintModuleToFile(context.module, "hello.ll", NULL);
}
