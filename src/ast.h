#ifndef AST_H
#define AST_H

#include "list.h"
#include "string_buffer.h"
#include "llvm-c/Types.h"

typedef enum AstNodeType {
    AstNodeType_Root,
    AstNodeType_TypeName,
    AstNodeType_Pattern,
    AstNodeType_ExternFn,
    AstNodeType_Fn,
    AstNodeType_FnProto,
    AstNodeType_Struct,
    AstNodeType_Block,
    AstNodeType_StatementReturn,
    AstNodeType_StatementExpression,
    AstNodeType_LetStatement,
    AstNodeType_PrimaryExpression,
    AstNodeType_IfExpression,
    AstNodeType_BinaryOperator,
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

typedef struct AstNodeStruct {
    String name;
    List elements;
} AstNodeStruct;

typedef struct AstNodeBlock {
    List statement_list;
} AstNodeBlock;

typedef struct AstNodeStatementReturn {
    AstNode* expression;
} AstNodeStatementReturn;

typedef struct AstNodeStatementExpression {
    AstNode* expression;
} AstNodeStatementExpression;

typedef struct AstNodeLetStatement {
    AstNode* pattern;
    AstNode* expression;
} AstNodeLetStatement;

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

typedef enum PrimaryExpressionType {
    PrimaryExpressionType_Number,
    PrimaryExpressionType_String,
    PrimaryExpressionType_FunctionCall,
    PrimaryExpressionType_Symbol,
} PrimaryExpressionType;

typedef struct PrimaryExpressionFunctionCall {
    String name;
    List parameters;
} PrimaryExpressionFunctionCall;

typedef struct AstNodePrimaryExpression {
    PrimaryExpressionType type;
    union {
        String number;
        String string;
        String symbol;
        PrimaryExpressionFunctionCall function_call;
    };
} AstNodePrimaryExpression;

typedef struct AstNodeIfExpression {
    AstNode* condition;
    AstNode* body;
    AstNode* alt;
} AstNodeIfExpression;

typedef enum OperatorPrecedence {
    OperatorPrecedence_Invalid = 0,

    OperatorPrecedence_Assignment = 1,

    OperatorPrecedence_Addition = 2,
    OperatorPrecedence_Subtration = 2,

    OperatorPrecedence_Multiplication = 3,
    OperatorPrecedence_Divisioon = 3,
} OperatorPrecedence;

typedef enum UnaryOperatorType {
    UnaryOperatorType_Negation,
    UnaryOperatorType_BitwiseComplement,
    UnaryOperatorType_LogicalNegation,
} UnaryOperatorType;

typedef struct AstNodeUnaryOperator {
    UnaryOperatorType type;
    AstNode* value;
} AstNodeUnaryOperator;

typedef enum  BinaryOperatorType {
    BinaryOperatorType_Assignment,
    BinaryOperatorType_Addition,
    BinaryOperatorType_Subtraction,
    BinaryOperatorType_Multiplication,
    BinaryOperatorType_Division,
} BinaryOperatorType;

typedef struct AstNodeBinaryOperator {
    BinaryOperatorType type;
    AstNode* left;
    AstNode* right;
} AstNodeInfixOperator;

typedef struct AstNode {
    AstNodeType type;
    AstTypeName type_name;
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
        AstNodeLetStatement let_statement;
        AstNodePrimaryExpression primary_expression;
        AstNodeIfExpression if_expression;
        AstNodeInfixOperator binary_operator;
        AstNodeUnaryOperator unary_operator;
    } data;
} AstNode;


void ast_print(AstNode* node);

#endif
