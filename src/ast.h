#ifndef AST_H
#define AST_H

#include "list.h"
#include "string_buffer.h"
#include <stdbool.h>

typedef struct Expr Expr;
typedef struct Stmt Stmt;

REGISTER_LIST_PTR(Expr);
REGISTER_LIST_PTR(Stmt);

// ---- //
// Type //
// ---- //
typedef enum TypeKind {
    TypeKind_Void,
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
    ExprKind_Let,
    ExprKind_Ret,
} ExprKind;

typedef struct StringLit {
    String text;
} StringLit;

typedef struct NumberLit {
    String text;
} NumberLit;

typedef struct Block {
    StmtPtrList statements;
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
	NumberLit number_literal;
	Block block;
	BinOp binary_operator;
	Expr* ret;
    };
} Expr;


// --------- //
// Statement //
// --------- //
typedef enum StmtKind {
    StmtKind_Expr,
} StmtKind;

typedef struct Stmt {
    StmtKind kind;
    union {
	Expr* expression;
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
REGISTER_LIST_PTR(FnParam);

typedef struct FnDecl {
    FnParamPtrList parameters;
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
REGISTER_LIST_PTR(Item);

typedef struct AstRoot {
    ItemPtrList items;
} AstRoot;

void ast_print(AstRoot* root);

#endif // !AST_H
