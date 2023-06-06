#include "parser.h"
#include "lexer.h"
#include "list.h"
#include "string.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct ParserContext {
    String source;
    List* token_list;
    int token_index;
} ParserContext;

static void skip_token(ParserContext* context) {
    context->token_index += 1;
}

static Token* current_token(ParserContext* context) {
    return list_get(Token, context->token_list, context->token_index);
}

static AstNode* node_new(AstNodeType type) {
    AstNode* node = calloc(1, sizeof(AstNode));
    node->type = type;

    return node;
}

static Token* token_expect(ParserContext* context, TokenType type) {
    Token* token = current_token(context);
    if (token->type != type) {
        sil_panic(
            "Expected %s. Got %s (%d:%d)\n",
            token_string(type),
            token_string(token->type),
            token->position.line,
            token->position.column
        );
    }

    return token;
}

static void infix_precedence(Token* operator, int* left, int* right) {
    switch (operator->type) {
        case TokenType_Plus:
        case TokenType_Dash:
            *left = 1;
            *right = 2;
            break;
        case TokenType_Star:
        case TokenType_Slash:
            *left = 3;
            *right = 4;
            break;
        default:
            *left = -1;
    }
}

static AstNode* parse_type_name(ParserContext* context) {
    AstNode* type_name = node_new(AstNodeType_TypeName);

    if (current_token(context)->type == TokenType_Star) {
        context->token_index += 1;
        type_name->data.type_name.type = AstNodeTypeNameType_Pointer;
        type_name->data.type_name.child_type = parse_type_name(context);

        return type_name;
    }

    type_name->data.type_name.type = AstNodeTypeNameType_Primitive;
    Token* token = token_expect(context, TokenType_Symbol);
    context->token_index += 1;
    
    AstTypeName primitive;
    if (token_symbol_compare(context->source, token, "i8")) {
        primitive = AstTypeName_i8;
    } else if (token_symbol_compare(context->source, token, "u8")) {
        primitive = AstTypeName_u8;
    } else if (token_symbol_compare(context->source, token, "i32")) {
        primitive = AstTypeName_i32;
    } else if (token_symbol_compare(context->source, token, "unreachable")) {
        primitive = AstTypeName_unreachable;
    } else {
        sil_panic("Unknown primitive type");
    }

    type_name->data.type_name.primitive = primitive;

    return type_name;
}

static AstNode* parse_pattern(ParserContext* context) {
    AstNode* pattern = node_new(AstNodeType_Pattern);

    Token* name_token = token_expect(context, TokenType_Symbol);
    context->token_index += 1;

    pattern->data.pattern.name = string_from_buffer(
        context->source.data + name_token->start,
        name_token->end - name_token->start
    );

    token_expect(context, TokenType_Colon);
    context->token_index += 1;

    pattern->data.pattern.type = parse_type_name(context);

    return pattern;
}

static AstNode* parse_expression(ParserContext* context);

static AstNode* parse_number_literal(ParserContext* context) {
    Token* token = current_token(context);
    context->token_index += 1;

    AstNode* number_literal = node_new(AstNodeType_ExpressionNumber);

    number_literal->data.expression_number.value = string_from_buffer(
        context->source.data + token->start,
        token->end - token->start
    );

    return number_literal;
}

static AstNode* parse_expression_primary(ParserContext* context) {
    Token* token = current_token(context);
    context->token_index += 1;

    switch (token->type) {

        case TokenType_Symbol: {
            AstNode* fn_call = node_new(AstNodeType_ExpressionFunction);
            fn_call->data.expression_function.name = string_from_buffer(
                context->source.data + token->start,
                token->end - token->start
            );

            token_expect(context, TokenType_LParen);
            context->token_index += 1;

            if (current_token(context)->type != TokenType_RParen) {
                AstNode* parameter = parse_expression(context);
                list_push(
                    AstNode*,
                    &fn_call->data.expression_function.parameters,
                    &parameter
                );
            }

            token_expect(context, TokenType_RParen);
            context->token_index += 1;

            return fn_call;
        }

        case TokenType_StringLiteral: {
            AstNode* string_literal = node_new(AstNodeType_ExpressionString);

            string_literal->data.expression_number.value = string_from_buffer(
                context->source.data + token->start,
                token->end - token->start
            );
            return string_literal;
        }

        case TokenType_NumberLiteral: {
            AstNode* number_literal = node_new(AstNodeType_ExpressionNumber);

            number_literal->data.expression_number.value = string_from_buffer(
                context->source.data + token->start,
                token->end - token->start
            );
            return number_literal;
        }
        default:
            sil_panic("Unexpected expression");
        
    }
}

static AstNode* parse_expression_bp(ParserContext* context, int precedence) {
    AstNode* left_expression = parse_expression_primary(context);

    int left;
    int right;
    while (1) {
        // check if there is an infix operator (+, -, *, /)
        Token* operator_token = current_token(context);
        infix_precedence(operator_token, &left, &right);
        if (left == -1) {
            break;
        }

        if (left < precedence) {
            break;
        }
        context->token_index += 1;
        
        AstNode* right_expression = parse_expression_bp(context, right);
        AstNode* operator = node_new(AstNodeType_InfixOperator);

        switch (operator_token->type) {
            case TokenType_Plus:        
                operator->data.infix_operator.type = AstNodeOperatorType_Addition;
                break;
            case TokenType_Dash:
                operator->data.infix_operator.type = AstNodeOperatorType_Subtraction;
                break;
            case TokenType_Star:
                operator->data.infix_operator.type = AstNodeOperatorType_Multiplication;
                break;
            case TokenType_Slash:
                operator->data.infix_operator.type = AstNodeOperatorType_Division;
                break;
            default:
                sil_panic("Parser Error: Unhandled operator");
        }

        operator->data.infix_operator.left = left_expression;
        operator->data.infix_operator.right = right_expression;


        left_expression = operator;
    }

    return left_expression;
}

static AstNode* parse_expression(ParserContext* context) {
    return parse_expression_bp(context, 0);
}

// statement: [returnStatement | ExpressionStatment] ;
static AstNode* parse_statement(ParserContext* context) {
    Token* token = current_token(context);

    if (token->type == TokenType_KeywordReturn) {
        AstNode* statement = node_new(AstNodeType_StatementReturn);

        context->token_index += 1; // consume 'ret'

        AstNode* expression = parse_expression(context);

        statement->data.statement_return.expression = expression;

        token_expect(context, TokenType_Semicolon);
        context->token_index += 1;
    
        return statement;
    }

    AstNode* statement = node_new(AstNodeType_StatementExpression);

    AstNode* expression = parse_expression(context);

    statement->data.statement_expression.expression = expression;

    token_expect(context, TokenType_Semicolon);
    context->token_index += 1;

    return statement;
}

static AstNode* parse_block(ParserContext* context) {
    AstNode* body = node_new(AstNodeType_Block);

    token_expect(context, TokenType_LBrace);
    context->token_index += 1;

    while (1) {
        switch (current_token(context)->type) {
            case TokenType_RBrace:
                context->token_index += 1;
                return body;
            default: {
                AstNode* statement = parse_statement(context);
                list_push(
                    AstNode*,
                    &body->data.block.statement_list,
                    &statement
                );
            }
        }
    }
}

// fn: fn [symbol]() [params]*
static AstNode* parse_fn_proto(ParserContext* context) {
    AstNode* fn_proto = node_new(AstNodeType_FnProto);

    token_expect(context, TokenType_KeywordFn);
    context->token_index += 1;

    Token* token = token_expect(context, TokenType_Symbol);
    fn_proto->data.fn_proto.name = string_from_buffer(
        context->source.data + token->start,
        token->end - token->start
    );

    context->token_index += 1;

    token_expect(context, TokenType_LParen);
    context->token_index += 1;

    // parameters
    List* param_list = &fn_proto->data.fn_proto.parameters;
    while (current_token(context)->type != TokenType_RParen) { 
        AstNode* pattern = parse_pattern(context);
        list_push(AstNode*, param_list, &pattern);

        if (current_token(context)->type == TokenType_Comma) {
            context->token_index += 1;
        }
    }

    token_expect(context, TokenType_RParen); 
    context->token_index += 1;

    // return statement
    AstNode* return_type;
    if (current_token(context)->type == TokenType_Arrow) {
        context->token_index += 1;
        return_type = parse_type_name(context);
    } else {
        return_type = node_new(AstNodeType_TypeName);
        return_type->data.type_name.type = AstNodeTypeNameType_Primitive;
        return_type->data.type_name.primitive = AstTypeName_void;
    }
    fn_proto->data.fn_proto.return_type = return_type;

    return fn_proto;
}

static AstNode* parse_fn(ParserContext* context) {
    AstNode* fn = node_new(AstNodeType_Fn);

    fn->data.fn.prototype = parse_fn_proto(context);

    fn->data.fn.body = parse_block(context);

    return fn;
}

static AstNode* parse_extern_fn(ParserContext* context) {
    AstNode* extern_fn = node_new(AstNodeType_ExternFn);

    token_expect(context, TokenType_KeywordExtern);
    context->token_index += 1;

    extern_fn->data.extern_fn.prototype = parse_fn_proto(context);

    token_expect(context, TokenType_Semicolon);
    context->token_index += 1;

    return extern_fn;
}

static AstNode* parse_root(ParserContext* context) {
    AstNode* root = node_new(AstNodeType_Root);
    while (1) {
        switch (current_token(context)->type) {
            case TokenType_KeywordFn: {
                AstNode* fn = parse_fn(context);
                list_push(AstNode*, &root->data.root.function_list, &fn);
                break;
            }
            case TokenType_KeywordExtern: {
                AstNode* extern_fn = parse_extern_fn(context);
                list_push(AstNode*, &root->data.root.function_list, &extern_fn);
                break;
            }
            case TokenType_Eof:
                return root;
            default:
                sil_panic("Expected function declaration");
        }
    }
}

AstNode* parse(String source, List* token_list) {
    ParserContext context;
    context.source = source;
    context.token_list = token_list;
    context.token_index = 0;

    AstNode* root = parse_root(&context);

    return root;
}

void parser_print_ast(AstNode *node) {
    switch (node->type) {
        case AstNodeType_Root:
            printf("\n--Root--\n");
            for (int i = 0; i < node->data.root.function_list.length; i++) {
                parser_print_ast(*list_get(AstNode*, &node->data.root.function_list, i));
            }
            break;
        case AstNodeType_ExternFn: {
            printf("\n--External Function--\n");
            printf(
                "name: %.*s\n",
                node->data.extern_fn.prototype->data.fn_proto.name.length,
                node->data.extern_fn.prototype->data.fn_proto.name.data
            );
            parser_print_ast(node->data.extern_fn.prototype);
            break;
        }
        case AstNodeType_Fn:
            printf("\n--Function Declaration--\n");
            printf(
                "name: %.*s\n",
                node->data.fn.prototype->data.fn_proto.name.length,
                    node->data.fn.prototype->data.fn_proto.name.data
            );
            parser_print_ast(node->data.fn.prototype);
            parser_print_ast(node->data.fn.body);
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
                parser_print_ast(statement);
            }
            break;
        case AstNodeType_StatementExpression:
            printf(">\texpression statement\n");
            parser_print_ast(node->data.statement_expression.expression);
            break;
        case AstNodeType_StatementReturn:
            printf(">\treturn statement: \n");
            parser_print_ast(node->data.statement_return.expression);
            break;
        case AstNodeType_InfixOperator:
            printf(">>\tInfix operator:\n");
            break;
            
        default:
            printf("Unknown AST Node\n");
    }
}

