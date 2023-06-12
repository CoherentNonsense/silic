#include "expression.h"

#include "lexer/lexer.h"
#include "parser.h"

static void operator_precedence(Token* operator, int* left, int* right) {
    OperatorPrecedence precedence;
    switch (operator->type) {
        case TokenType_Plus: precedence = OperatorPrecedence_Addition; break;
        case TokenType_Dash: precedence = OperatorPrecedence_Subtration; break;
        case TokenType_Star: precedence = OperatorPrecedence_Multiplication; break;
        case TokenType_Slash: precedence = OperatorPrecedence_Divisioon; break;
        default: precedence = OperatorPrecedence_Invalid;
    }

    *left = (precedence * 2) - 1;
    *right = precedence * 2;
}

static AstNode* parse_expression_primary(ParserContext* context) {
    Token* token = current_token(context);
    consume_token(context);

    switch (token->type) {

        // TODO: Move to Pratt Parser
        case TokenType_Tilde: {
            AstNode* bitwise_complement = node_new(AstNodeType_UnaryOperator);
            bitwise_complement->data.unary_operator.type = UnaryOperatorType_BitwiseComplement;
            bitwise_complement->data.unary_operator.value = parse_expression_primary(context);

            return bitwise_complement;
        }

        case TokenType_Dash: {
            AstNode* negation = node_new(AstNodeType_UnaryOperator);
            negation->data.unary_operator.type = UnaryOperatorType_Negation;
            negation->data.unary_operator.value = parse_expression_primary(context);

            return negation;
        }

        case TokenType_Bang: {
            AstNode* logical_negation = node_new(AstNodeType_UnaryOperator);
            logical_negation->data.unary_operator.type = UnaryOperatorType_LogicalNegation;
            logical_negation->data.unary_operator.value = parse_expression_primary(context);

            return logical_negation;
        }

        case TokenType_KeywordIf: {
            AstNode* if_expression = node_new(AstNodeType_IfExpression);
            if_expression->data.if_expression.condition = parse_expression(context);
            if_expression->data.if_expression.body = parse_block(context);
            if (current_token(context)->type == TokenType_KeywordElse) {
                consume_token(context);
                if (current_token(context)->type == TokenType_KeywordIf) {
                    if_expression->data.if_expression.alt = parse_expression_primary(context);
                } else {
                    if_expression->data.if_expression.alt = parse_block(context);
                }
            }
            return if_expression;
        }

        case TokenType_Symbol: {
            AstNode* fn_call = node_new(AstNodeType_PrimaryExpression);
            fn_call->data.primary_expression.type = PrimaryExpressionType_Symbol;
            fn_call->data.primary_expression.function_call.name = string_from_token(
                context->source.data, token
            );

            expect_token(context, TokenType_LParen);

            if (current_token(context)->type != TokenType_RParen) {
                AstNode* parameter = parse_expression(context);
                list_push(
                    AstNode*,
                    &fn_call->data.primary_expression.function_call.parameters,
                    &parameter
                );
            }

            expect_token(context, TokenType_RParen);

            return fn_call;
        }

        case TokenType_StringLiteral: {
            AstNode* string_literal = node_new(AstNodeType_PrimaryExpression);

            string_literal->data.primary_expression.type = PrimaryExpressionType_String;
            string_literal->data.primary_expression.string = string_from_token(
                context->source.data, token
            );
            return string_literal;
        }
        case TokenType_NumberLiteral: {
            AstNode* number_literal = node_new(AstNodeType_PrimaryExpression);

            number_literal->data.primary_expression.type = PrimaryExpressionType_Number;
            number_literal->data.primary_expression.number = string_from_token(
                context->source.data, token
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
        consume_token(context);
        left_expression = parse_expression(context);
        expect_token(context, TokenType_RParen);
    } else {
        left_expression = parse_expression_primary(context);
    }

    int left;
    int right;
    while (1) {
        // check if there is an infix operator (+, -, *, /)
        Token* operator_token = current_token(context);
        operator_precedence(operator_token, &left, &right);
        if (left == -1) {
            break;
        }

        if (left < precedence) {
            break;
        }
        consume_token(context);
        
        AstNode* right_expression = parse_expression_prec(context, right);
        AstNode* operator = node_new(AstNodeType_BinaryOperator);

        switch (operator_token->type) {
            case TokenType_Plus:        
                operator->data.binary_operator.type = BinaryOperatorType_Addition;
                break;
            case TokenType_Dash:
                operator->data.binary_operator.type = BinaryOperatorType_Subtraction;
                break;
            case TokenType_Star:
                operator->data.binary_operator.type = BinaryOperatorType_Multiplication;
                break;
            case TokenType_Slash:
                operator->data.binary_operator.type = BinaryOperatorType_Division;
                break;
            default:
                sil_panic("Parser Error: Unhandled operator");
        }

        operator->data.binary_operator.left = left_expression;
        operator->data.binary_operator.right = right_expression;


        left_expression = operator;
    }

    return left_expression;
}

AstNode* parse_expression(ParserContext* context) {
    return parse_expression_prec(context, 0);
}
