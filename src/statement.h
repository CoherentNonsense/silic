#ifndef STATEMENT_H

typedef struct AstNode AstNode;

typedef enum StatementKind {
    StatementKind_ExpressionStatement,
    StatementKind_LetStatement,
    StatementKind_ReturnStatement,
} StatementKind;

typedef struct AstNodeLetStatement {
    AstNode* pattern;
    AstNode* expression;
} AstNodeLetStatement;

typedef struct Statement {
    StatementKind kind;
} Statement;

#endif // !STATEMENT_H
