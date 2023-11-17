#include "ast.h"

bool should_remove_statement_semi(Expr* expression) {
    return (
	expression->kind == ExprKind_If ||
	expression->kind == ExprKind_Match ||
	expression->kind == ExprKind_Block ||
        expression->kind == ExprKind_Loop ||
        expression->kind == ExprKind_Asm
    );
}
