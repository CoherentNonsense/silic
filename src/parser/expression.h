#ifndef PARSER_EXPRESSION_H
#define PARSER_EXPRESSION_H

#include "list.h"
#include "string_buffer.h"
typedef struct ParserContext ParserContext;
typedef struct AstNode AstNode;


typedef enum PrimaryExpressionType {
    PrimaryExpressionType_Number,
    PrimaryExpressionType_String,
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
        PrimaryExpressionFunctionCall function_call;
    };
} AstNodePrimaryExpression;

typedef struct AstNodeIfExpression {
    AstNode* condition;
    AstNode* body;
    AstNode* alt;
} AstNodeIfExpression;

typedef enum OperatorPrecedence {
    OperatorPrecedence_Invalid,
    OperatorPrecedence_Addition = 1,
    OperatorPrecedence_Subtration = 1,
    OperatorPrecedence_Multiplication = 2,
    OperatorPrecedence_Divisioon = 2,
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

AstNode* parse_expression(ParserContext* context);

#endif
