#ifndef AST_H
#define AST_H

#include "span.h"
#include "dynarray.h"
#include <stdbool.h>

typedef struct Expr Expr;
typedef struct Stmt Stmt;


// ---- //
// Type //
// ---- //
typedef struct Type Type;

typedef enum TypeKind {
    TypeKind_Symbol,
    TypeKind_Int,
    TypeKind_Float,
    TypeKind_Ptr,
    TypeKind_Array,
    TypeKind_Never,
} TypeKind;

typedef struct Int {
    bool is_signed;
} Int;

typedef struct Ptr {
    Type* to;
    bool is_mut;
} Ptr;

typedef struct Type {
    TypeKind kind;
    Span symbol;
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
    DynArray(Stmt*) statements;
} Block;

typedef struct If {
    Expr* condition;
    Block* then;
    Expr* otherwise;
} If;

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
    DynArray(Expr*) arguments;
} FnCall;

typedef struct Expr {
    ExprKind kind;
    Type type;
    union {
	StringLit string_literal;
	NumberLit number_literal;
	Block* block;
	If* if_expr;
	BinOp binary_operator;
	Let let;
	Expr* ret;
	Span symbol;
	FnCall* fn_call;
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
    ItemKind_ExternFn,
} ItemKind;

typedef struct FnParam {
    Span name;
    Type* type;
} FnParam;

typedef struct FnSig {
    DynArray(FnParam*) parameters;
    Type* return_type;
} FnSig;

typedef struct FnDecl {
    FnSig* signature;
    Block* body;
} FnDecl;

typedef struct ExternFn {
    FnSig* signature;
} ExternFn;

typedef struct Item {
    ItemKind kind;
    Span name;
    union {
	FnDecl* fn_declaration;
	ExternFn* extern_fn;
    };
} Item;

typedef struct AstRoot {
    DynArray(Item*) items;
} AstRoot;

void ast_print(AstRoot* root);

#endif // !AST_H
