#ifndef PARSER_H
#define PARSER_H

#include "expression.h"
#include "lexer/lexer.h"
#include "list.h"
#include "string_buffer.h"
#include "llvm-c/Types.h"
#include <stddef.h>

typedef struct ParserContext {
    String source;
    List* token_list;
    int token_index;
} ParserContext;

typedef enum AstNodeType {
    AstNodeType_Root,
    AstNodeType_TypeName,
    AstNodeType_Pattern,
    AstNodeType_ExternFn,
    AstNodeType_Fn,
    AstNodeType_FnProto,
    AstNodeType_Block,
    AstNodeType_StatementReturn,
    AstNodeType_StatementExpression,
    AstNodeType_ExpressionNumber,
    AstNodeType_ExpressionString,
    AstNodeType_ExpressionFunction,
    AstNodeType_InfixOperator,
    AstNodeType_UnaryOperator,
} AstNodeType;

typedef enum AstTypeName {
    AstTypeName_unreachable,
    AstTypeName_void,
    AstTypeName_i8,
    AstTypeName_u8,
    AstTypeName_i32,
} AstTypeName;

typedef struct AstNode AstNode;

typedef struct AstNodeRoot {
    List function_list;
} AstNodeRoot;

typedef enum AstNodeTypeNameType {
    AstNodeTypeNameType_Primitive,
    AstNodeTypeNameType_Pointer,
} AstNodeTypeNameType;

typedef struct AstNodeTypeName {
    AstNodeTypeNameType type;
    AstTypeName primitive;
    AstNode* child_type;
} AstNodeTypeName;

typedef struct AstNodePattern {
    String name;
    AstNode* type;
} AstNodePattern;

typedef struct AstNodeExternFn {
    AstNode* prototype;
} AstNodeExternFn;

typedef struct AstNodeFn {
    AstNode* prototype;
    AstNode* body;
} AstNodeFn;

typedef struct AstNodeFnProto {
    String name;
    AstNode* return_type;
    List parameters;
    LLVMTypeRef llvm_fn_type;
} AstNodeFnProto;

typedef struct AstNodeFnParam {
    String name;
    AstNode* type;
} AstNodeFnParam;

typedef struct AstNodeBlock {
    List statement_list;
} AstNodeBlock;

typedef struct AstNodeStatementReturn {
    AstNode* expression;
} AstNodeStatementReturn;

typedef struct AstNodeStatementExpression {
    AstNode* expression;
} AstNodeStatementExpression;

typedef struct AstNodeExpressionNumber {
    String value;
} AstNodeExpressionNumber;

typedef struct AstNodeExpressionString {
    String value;
} AstNodeExpressionString;

typedef struct AstNodeExpressionFunction {
    String name;
    List parameters;
} AstNodeExpressionFunction;

typedef struct AstNode {
    AstNodeType type;
    union {
        AstNodeRoot root;
        AstNodeTypeName type_name;
        AstNodePattern pattern;
        AstNodeExternFn extern_fn;
        AstNodeFn fn;
        AstNodeFnProto fn_proto;
        AstNodeFnParam fn_param;
        AstNodeBlock block;
        AstNodeStatementReturn statement_return;
        AstNodeStatementExpression statement_expression;
        AstNodeExpressionNumber expression_number;
        AstNodeExpressionString expression_string;
        AstNodeExpressionFunction expression_function;
        AstNodeInfixOperator infix_operator;
        AstNodeUnaryOperator unary_operator;
    } data;
} AstNode;

void skip_token(ParserContext* context);
Token* current_token(ParserContext* context);
Token* token_expect(ParserContext* context, TokenType type);
AstNode* node_new(AstNodeType type);

AstNode* parse(String source, List* token_list);

void parser_print_ast(AstNode* node);

#endif
