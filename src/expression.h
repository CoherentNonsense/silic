#ifndef EXPRESSION_H

typedef enum ExpressionKind {
    ExpressionKind_StringLiteral,
    ExpressionKind_NumberLiteral,
    ExpressionKind_IfExpression,
} ExpressionKind;

typedef struct Expression {
    ExpressionKind kind;
} Expression;

#endif // !EXPRESSION_H
