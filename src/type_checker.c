#include "type_checker.h"
#include "hashmap.h"

typedef struct TypeCheckerContext {
    int test;
} TypeCheckerContext;

static void tc_root(TypeCheckerContext* context, AstNode* root) {
}

void tc_check(AstNode* root) {
    TypeCheckerContext context = {0};

    tc_root(&context, root);
}
