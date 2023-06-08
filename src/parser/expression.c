#include "expression.h"

#include "lexer/lexer.h"
#include "parser.h"

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

        case TokenType_Tilde: {
            AstNode* bitwise_complement = node_new(AstNodeType_UnaryOperator);
            bitwise_complement->data.unary_operator.type = UnaryOperatorType_BitwiseComplement;
            bitwise_complement->data.unary_operator.value = parse_expression_primary(context);

            return bitwise_complement;
        }

        case TokenType_Dash: {
            AstNode* bitwise_complement = node_new(AstNodeType_UnaryOperator);
            bitwise_complement->data.unary_operator.type = UnaryOperatorType_Negation;
            bitwise_complement->data.unary_operator.value = parse_expression_primary(context);

            return bitwise_complement;
        }

        case TokenType_Bang: {
            AstNode* bitwise_complement = node_new(AstNodeType_UnaryOperator);
            bitwise_complement->data.unary_operator.type = UnaryOperatorType_LogicalNegation;
            bitwise_complement->data.unary_operator.value = parse_expression_primary(context);

            return bitwise_complement;
        }

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

static AstNode* parse_expression_prec(ParserContext* context, int precedence) {
    AstNode* left_expression;
    if (current_token(context)->type == TokenType_LParen) {
        context->token_index += 1;
        left_expression = parse_expression(context);
        token_expect(context, TokenType_RParen);
        context->token_index += 1;
    } else {
        left_expression = parse_expression_primary(context);
    }

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
        
        AstNode* right_expression = parse_expression_prec(context, right);
        AstNode* operator = node_new(AstNodeType_InfixOperator);

        switch (operator_token->type) {
            case TokenType_Plus:        
                operator->data.infix_operator.type = BinaryOperatorType_Addition;
                break;
            case TokenType_Dash:
                operator->data.infix_operator.type = BinaryOperatorType_Subtraction;
                break;
            case TokenType_Star:
                operator->data.infix_operator.type = BinaryOperatorType_Multiplication;
                break;
            case TokenType_Slash:
                operator->data.infix_operator.type = BinaryOperatorType_Division;
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

AstNode* parse_expression(ParserContext* context) {
    return parse_expression_prec(context, 0);
}
