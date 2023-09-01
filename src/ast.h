#ifndef AST_H
#define AST_H

#include "span.h"
#include "list.h"
#include <stdbool.h>

typedef struct Expr Expr;
typedef struct Stmt Stmt;

REGISTER_LIST_PTR(Expr);
REGISTER_LIST_PTR(Stmt);

// ---- //
// Type //
// ---- //
typedef enum TypeKind {
    TypeKind_Symbol,
    TypeKind_Ptr,
    TypeKind_Array,
} TypeKind;

typedef struct Type Type;
typedef struct Ptr {
    Type* to;
} Ptr;

typedef struct Type {
    TypeKind kind;
    bool is_mut;
    union {
	Span symbol;
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
    ExprKind_Symbol,
    ExprKind_FnCall,
} ExprKind;

typedef struct StringLit {
    Span span;
} StringLit;

typedef struct NumberLit {
    Span span;
} NumberLit;

typedef struct Block {
    StmtPtrList statements;
} Block;

typedef enum BinOpKind {
    BinOpKind_Eq,
    BinOpKind_Add,
    BinOpKind_Sub,
    BinOpKind_Mul,
    BinOpKind_Div,
} BinOpKind;

typedef struct BinOp {
    BinOpKind kind;
    Expr* left;
    Expr* right;
} BinOp;

typedef enum OpPrec {
    OpPrec_Invalid = 0,

    OpPrec_Assign = 1,

    OpPrec_Add = 2,
    OpPrec_Sub = 2,

    OpPrec_Mul = 3,
    OpPrec_Div = 3,
} OpPrec;

typedef struct Let {
    Span name;
    Type* type;
    Expr* value;
} Let;

typedef struct FnCall {
    Span name;
} FnCall;

typedef struct Expr {
    ExprKind kind;
    Type type;
    union {
	StringLit string_literal;
	NumberLit number_literal;
	Block block;
	BinOp binary_operator;
	Let let;
	Expr* ret;
	Span symbol;
	FnCall fn_call;
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
    Span name;
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
    Span name;
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
