#ifndef PARSER_EXPRESSION_H
#define PARSER_EXPRESSION_H

typedef struct ParserContext ParserContext;
typedef struct AstNode AstNode;


typedef struct AstNodeIfExpression {
    AstNode* condition;
    AstNode* body;
    AstNode* alt;
} AstNodeIfExpression;

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

typedef struct AstNodeInfixOperator {
    BinaryOperatorType type;
    AstNode* left;
    AstNode* right;
} AstNodeInfixOperator;

AstNode* parse_expression(ParserContext* context);

#endif
