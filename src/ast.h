#ifndef AST_H

typedef enum AstNodeKind {
    AstNodeKind_Root,
    AstNodeKind_Type,
    AstNodeKind_Pattern,
    AstNodeKind_FnProto,
    AstNodeKind_Fn,
    AstNodeKind_ExternFn,
    AstNodeKind_Struct,
    AstNodeKind_Block,
    AstNodeKind_Statement,
    AstNodeKind_Expression,
    AstNodeKind_UnaryOperator,
    AstNodeKind_BinaryOperator,
} AstNodeKind;

typedef struct AstNode {
    AstNodeKind kind;
} AstNode;

AstNode* ast_make(AstNodeKind kind);

void ast_debug(AstNode* root);

#endif // !AST_H
