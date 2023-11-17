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
    type_id type;
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
typedef struct Ast_Type Ast_Type;

typedef enum Ast_TypeKind {
    TypeKind_Void,
    TypeKind_Symbol,
    TypeKind_Ptr,
    TypeKind_Array,
    TypeKind_Never,
    TypeKind_Type,
} Ast_TypeKind;

typedef struct Ast_Integral {
    bool is_signed;
    uint64_t value;
} Ast_Integer;

typedef struct Ast_Decimal {
    double value;
} Ast_Decimal;

typedef struct Ast_Ptr {
    Ast_Type* to;
    bool is_mut;
} Ast_Ptr;

typedef struct Ast_Type {
    Ast_TypeKind kind;
    union {
	String symbol;
	Ast_Ptr ptr;
	Ast_Integer integer;
	Ast_Decimal decimal;
    };
} Ast_Type;


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
    ExprKind_Loop,
    ExprKind_Break,
    ExprKind_Continue,
    ExprKind_Unreachable,
    ExprKind_Asm,
    ExprKind_Field,
    ExprKind_Index,
    ExprKind_Cast,
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

typedef struct Loop {
    Expr* body;
} Loop;

typedef enum BinOpKind {
    BinOpKind_And,
    BinOpKind_Or,
    BinOpKind_CmpEq,
    BinOpKind_CmpNotEq,
    BinOpKind_CmpGt,
    BinOpKind_CmpLt,
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

typedef enum OpPrec {
    OpPrec_Invalid = 0,

    OpPrec_And = 5,
    OpPrec_Or = 5,

    OpPrec_CmpEq = 10,
    OpPrec_CmpNotEq = 10,

    OpPrec_CmpGt = 15,
    OpPrec_CmpLt = 15,

    OpPrec_Assign = 20,

    OpPrec_Add = 30,
    OpPrec_Sub = 30,

    OpPrec_Mul = 40,
    OpPrec_Div = 40,
} OpPrec;

typedef struct Let {
    String name;
    Ast_Type* type;
    Expr* value;
} Let;

typedef struct FnCall {
    String name;
    DynArray(Expr*) arguments;
} FnCall;

typedef struct AsmInput {
    String reg;
    Expr* val;
} AsmInput;

typedef struct Asm {
    DynArray(AsmInput) inputs;
    DynArray(String) clobbers;
    DynArray(String) outputs;
    DynArray(StringLit) source;
} Asm;

typedef struct Cast {
    Expr* expr;
    Ast_Type* to;
} Cast;

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
        Loop* loop;
        Asm* asm;
        Cast* cast;
    };

    struct {
        type_id type;
    } codegen;
} Expr;


// --------- //
// Statement //
// --------- //
typedef enum StmtKind {
    StmtKind_Expr,
    StmtKind_NakedExpr,
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
    Ast_Type* type;
} FnParam;

typedef struct FnSig {
    DynArray(FnParam*) parameters;
    Ast_Type* return_type;
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
    Ast_Type* type;
} StructField;

typedef struct StructDef {
    DynArray(StructField*) fields;
} StructDef;

typedef struct Constant {
    Ast_Type* type;
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

bool should_remove_statement_semi(Expr* expression);

#endif // !AST_H
