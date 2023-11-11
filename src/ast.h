#ifndef AST_H
#define AST_H

#include "util.h"
#include "typetable.h"
#include <chnlib/str.h>
#include <chnlib/map.h>
#include <chnlib/dynarray.h>

typedef struct Item Item;
typedef struct Let Let;
typedef struct Stmt Stmt;
typedef struct Expr Expr;


// ------------ //
// Symbol Table //
// ------------ //
// HACK: ast nodes keep a reference to the scope they're a part of and type info
//       so they can be references later. don't...
typedef struct SymEntry {
    TypeEntry* type;
} SymEntry;

typedef struct Scope {
    struct Scope* parent;
    DynArray(struct Scope) children;
    Map(SymEntry) symbols;
} Scope;

typedef struct SymTable {
    Scope root_scope;
    Scope* current_scope;
} SymTable;


// ---- //
// Type //
// ---- //
typedef struct Type Type;

typedef enum TypeKind {
    TypeKind_Void,
    TypeKind_Symbol,
    TypeKind_Ptr,
    TypeKind_Array,
    TypeKind_Never,
} TypeKind;

typedef struct Integral {
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
	String symbol;
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
    ExprKind_BoolLit,
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
    String span;
} StringLit;

typedef struct NumberLit {
    String span;
} NumberLit;

typedef struct Block {
    Scope* scope;
    DynArray(Stmt*) statements;
} Block;

typedef struct If {
    Expr* condition;
    Expr* then;
    Expr* otherwise;
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
    BinOpKind_CmpEq,
    BinOpKind_CmpNotEq,
    BinOpKind_Assign,
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

// 1 + 2 == i = 4 
typedef enum OpPrec {
    OpPrec_Invalid = 0,

    OpPrec_Equality = 1,
    OpPrec_Inequality = 1,

    OpPrec_Assign = 2,

    OpPrec_Add = 3,
    OpPrec_Sub = 3,

    OpPrec_Mul = 4,
    OpPrec_Div = 4,
} OpPrec;

typedef struct Let {
    String name;
    Type* type;
    Expr* value;
} Let;

typedef struct FnCall {
    String name;
    DynArray(Expr*) arguments;
} FnCall;

typedef struct Expr {
    ExprKind kind;
    union {
	StringLit string_literal;
	NumberLit* number_literal;
	Block* block;
	If* if_expr;
	Match* match;
	BinOp* binary_operator;
	Let* let;
	Expr* ret;
	String symbol;
	FnCall* fn_call;
        bool boolean;
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
    String name;
    Type* type;
} FnParam;

typedef struct FnSig {
    DynArray(FnParam*) parameters;
    Type* return_type;
} FnSig;

typedef struct FnDef {
    FnSig* signature;
    Expr* body;
} FnDef;

typedef struct ExternFn {
    FnSig* signature;
} ExternFn;

typedef struct StructField {
    Visibility visibility;
    String name;
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
    String name;
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

#endif // !AST_H
