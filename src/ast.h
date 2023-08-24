#ifndef AST_H
#define AST_H

#include "list.h"
#include "string_buffer.h"
#include <stdbool.h>

// ---- //
// Type //
// ---- //
typedef enum TypeKind {
    TypeKind_Ptr,
    TypeKind_U8,
    TypeKind_I32,
    TypeKind_CustomType,
} TypeKind;

typedef struct Type Type;
typedef struct Ptr {
    Type* to;
} Ptr;

typedef struct Type {
    TypeKind kind;
    bool is_mut;
    union {
	Ptr ptr;
    };
} Type;


// ---------- //
// Expression //
// ---------- //
typedef enum ExprKind {
    ExprKind_StringLit,
    ExprKind_NumberLit,
    ExprKind_If,
    ExprKind_Block,
    ExprKind_UnOp,
    ExprKind_BinOp,
} ExprKind;

typedef struct StringLit {
    String literal;
} StringLit;

typedef struct Block {
    List statements;
} Block;

typedef enum BinOpKind {
    BinOpKind_Add,
    BinOpKind_Sub,
    BinOpKind_Mul,
    BinOpKind_Div,
} BinOpKind;

typedef struct BinOp {
    BinOpKind kind;
    ExprKind left;
    ExprKind right;
} BinOp;

typedef struct Expr {
    ExprKind kind;
    union {
	StringLit string_literal;
	Block body;
	BinOp binary_operator;
    };
} Expr;


// --------- //
// Statement //
// --------- //
typedef enum StmtKind {
    StmtKind_Expr,
    StmtKind_If,
} StmtKind;

typedef struct ExprStmt {
    Expr* expression;
} ExprStmt;

typedef struct IfStmt {
} IfStmt;

typedef struct Stmt {
    StmtKind kind;
    union {
	ExprStmt expression_statement;
    };
} Stmt;


// ---- //
// Item //
// ---- //
typedef enum ItemKind {
    ItemKind_FnDecl,
} ItemKind;

typedef struct FnParam {
    String name;
    Type* type;
} FnParam;

typedef struct FnDecl {
    List parameters;
    Type* return_type;
    Block* body;
} FnDecl;

typedef struct Item {
    ItemKind kind;
    String name;
    union {
	FnDecl* fn_declaration;
    };
} Item;

typedef struct AstRoot {
    List items;
} AstRoot;

void ast_print(AstRoot* root);

#endif // !AST_H
