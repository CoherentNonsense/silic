#ifndef AST_H
#define AST_H

#include "span.h"
#include "dynarray.h"
#include "util.h"
#include "hashmap.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct Item Item;
typedef struct Let Let;
typedef struct Stmt Stmt;
typedef struct Expr Expr;


// ------------ //
// Symbol Table //
// ------------ //
typedef struct Scope {
    struct Scope* parent;
    HashMap(Let*) symbols;
    DynArray(struct Scope*) children;
} Scope;

typedef struct SymTable {
    DynArray(Scope*) children;
} SymTable;


// ---- //
// Type //
// ---- //
typedef struct Type Type;

typedef enum TypeKind {
    TypeKind_Void,
    TypeKind_Symbol,
    TypeKind_Int,
    TypeKind_Float,
    TypeKind_Ptr,
    TypeKind_Array,
    TypeKind_Never,
} TypeKind;

typedef struct Integer {
    bool is_signed;
    uint64_t value;
} Integer;

typedef struct Decimal {
    double value;
} Decimal;

typedef struct Ptr {
    Type* to;
    bool is_mut;
} Ptr;

typedef struct Type {
    TypeKind kind;
    union {
	Span symbol;
	Ptr ptr;
	Integer integer;
	Decimal decimal;
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
    ExprKind_Match,
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
    Maybe(Expr*) otherwise;
} If;

typedef struct MatchArm {
    NumberLit* pattern;
    Expr* then;
} MatchArm;

typedef struct Match {
    Expr* condition;
    DynArray(MatchArm*) arms;
} Match;

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

typedef struct Symbol {
    Span symbol;
    void* scope;
} Symbol;

typedef struct FnCall {
    Span name;
    DynArray(Expr*) arguments;
} FnCall;

typedef struct Expr {
    ExprKind kind;
    Type type;
    union {
	StringLit string_literal;
	NumberLit* number_literal;
	Block* block;
	If* if_expr;
	Match* match;
	BinOp* binary_operator;
	Let* let;
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
    ItemKind_FnDef,
    ItemKind_ExternFn,
    ItemKind_StructDef,
    ItemKind_Const,
} ItemKind;

typedef struct Visibilty {
    bool is_pub;
} Visibility;

typedef struct FnParam {
    Span name;
    Type* type;
} FnParam;

typedef struct FnSig {
    DynArray(FnParam*) parameters;
    Type* return_type;
} FnSig;

typedef struct FnDef {
    FnSig* signature;
    Block* body;
} FnDef;

typedef struct ExternFn {
    FnSig* signature;
} ExternFn;

typedef struct StructField {
    Visibility visibility;
    Span name;
    Type* type;
} StructField;

typedef struct StructDef {
    DynArray(StructField*) fields;
} StructDef;

typedef struct Constant {
    Type* type;
    Expr* value;
} Constant;

typedef struct Item {
    Visibility visibility;
    ItemKind kind;
    Span name;
    union {
	FnDef* fn_definition;
	ExternFn* extern_fn;
	StructDef* struct_definition;
	Constant* constant;
    };
} Item;

typedef struct AstRoot {
    DynArray(Item*) items;
} AstRoot;

void ast_print(AstRoot* root);

#endif // !AST_H
