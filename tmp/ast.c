#include "ast.h"

#include "stdio.h"

void ast_print(AstNode *node) {
    switch (node->type) {
        case AstNodeType_Root:
            printf("\n--Root--\n");
            for (int i = 0; i < node->data.root.function_list.length; i++) {
                ast_print(*list_get(AstNode*, &node->data.root.function_list, i));
            }
            break;
        case AstNodeType_ExternFn: {
            printf("\n--External Function--\n");
            printf(
                "name: %.*s\n",
                node->data.extern_fn.prototype->data.fn_proto.name.length,
                node->data.extern_fn.prototype->data.fn_proto.name.data
            );
            ast_print(node->data.extern_fn.prototype);
            break;
        }
        case AstNodeType_Fn:
            printf("\n--Function Declaration--\n");
            printf(
                "name: %.*s\n",
                node->data.fn.prototype->data.fn_proto.name.length,
                    node->data.fn.prototype->data.fn_proto.name.data
            );
            ast_print(node->data.fn.prototype);
            ast_print(node->data.fn.body);
            break;
        case AstNodeType_FnProto: {
            List* parameters = &node->data.fn_proto.parameters;
            for (int i = 0; i < parameters->length; i++) {
                AstNode* param = *list_get(AstNode*, parameters, i);
                printf("param %d: %.*s\n", i,
                    param->data.pattern.name.length,
                    param->data.pattern.name.data
                );
            }
            break;
        }
        case AstNodeType_Block:
            printf("--Block--\n");
            AstNodeBlock* block = &node->data.block;
            for (int i = 0; i < node->data.block.statement_list.length; i++) {
                AstNode* statement = *list_get(AstNode*, &block->statement_list, i);
                ast_print(statement);
            }
            break;
        case AstNodeType_StatementExpression:
            printf(">\texpression statement\n");
            ast_print(node->data.statement_expression.expression);
            break;
        case AstNodeType_StatementReturn:
            printf("\t\treturn statement: \n");
            ast_print(node->data.statement_return.expression);
            break;
        case AstNodeType_PrimaryExpression:
            printf("\t\tprimary expression\n");
            break;
        case AstNodeType_BinaryOperator:
            printf(">\tInfix operator:\n");
            break;
            
        default:
            printf("Unknown AST Node: %d\n", node->type);
    }
}
