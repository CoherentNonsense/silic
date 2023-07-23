#include "parser.h"
#include "token.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct ParserContext {
    String source;
    List token_list;
    int token_index;
} ParserContext;

static Token* current_token(ParserContext* context) {
    return list_get(Token, &context->token_list, context->token_index);
}

static void parser_error(ParserContext* context, const char* format, ...)
    __attribute__((format(printf, 2, 3)))
    __attribute__((noreturn));

static void parser_error(ParserContext* context, const char* format, ...) {
    Token* token = current_token(context);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    fprintf(stderr, " (%d:%d)", token->position.line, token->position.column);
    fprintf(stderr, "\n");
    va_end(args);
    exit(EXIT_FAILURE);
}

static void consume_token(ParserContext* context) {
    context->token_index += 1;
}

static Token* expect_token(ParserContext* context, TokenType type) {
    Token* token = current_token(context);
    if (token->type != type) {
        parser_error(
            context,
            "Expected %s. Got %s",
            token_string(type),
            token_string(token->type)
        );
    }

    consume_token(context);

    return token;
}

static AstNode* node_new(AstNodeType type) {
    AstNode* node = calloc(1, sizeof(AstNode));
    node->type = type;

    return node;
}

static AstNode* parse_expression(ParserContext* context);
static AstNode* parse_block(ParserContext* context);

static AstNode* parse_type_name(ParserContext* context) {
    AstNode* type_name = node_new(AstNodeType_TypeName);

    if (current_token(context)->type == TokenType_Star) {
        consume_token(context);
        type_name->data.type_name.type = AstNodeTypeNameType_Pointer;
        type_name->data.type_name.child_type = parse_type_name(context);

        return type_name;
    }

    type_name->data.type_name.type = AstNodeTypeNameType_Primitive;
    Token* token = expect_token(context, TokenType_Symbol);
    
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
        parser_error(context, "Unknown primitive type");
    }

    type_name->data.type_name.primitive = primitive;

    return type_name;
}

static AstNode* parse_pattern(ParserContext* context) {
    AstNode* pattern = node_new(AstNodeType_Pattern);

    Token* name_token = expect_token(context, TokenType_Symbol);

    pattern->data.pattern.name = string_from_buffer(
        context->source.data + name_token->start,
        name_token->end - name_token->start
    );

    expect_token(context, TokenType_Colon);

    pattern->data.pattern.type = parse_type_name(context);

    return pattern;
}

static void operator_precedence(Token* operator, int* left, int* right) {
    OperatorPrecedence precedence;
    switch (operator->type) {
        case TokenType_Equals: precedence = OperatorPrecedence_Assignment; break;
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
            AstNode* symbol = node_new(AstNodeType_PrimaryExpression);

            if (current_token(context)->type != TokenType_LParen) {
                symbol->data.primary_expression.type = PrimaryExpressionType_Symbol;
                symbol->data.primary_expression.symbol = string_from_token(
                    context->source.data, token
                );

                return symbol;
            }

            expect_token(context, TokenType_LParen);

            symbol->data.primary_expression.type = PrimaryExpressionType_FunctionCall;
            symbol->data.primary_expression.function_call.name = string_from_token(
                context->source.data, token
            );

            while (current_token(context)->type != TokenType_RParen) {
                AstNode* parameter = parse_expression(context);
                list_push(
                    AstNode*,
                    &symbol->data.primary_expression.function_call.parameters,
                    &parameter
                );
 
                if (current_token(context)->type == TokenType_Comma) {
                    consume_token(context);
                } else {
                    break;
                }
            }

            expect_token(context, TokenType_RParen);

            return symbol;
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
            parser_error(context, "Unexpected expression");
        
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
            case TokenType_Equals:
                operator->data.binary_operator.type = BinaryOperatorType_Assignment;
                break;
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
                parser_error(context, "Parser Error: Unhandled operator");
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

// statement = returnStatement
//           | ifStatement
//           | ExpressionStatment
//           | letStatement
//           | assignmentExpression
static AstNode* parse_statement(ParserContext* context) {
    Token* token = current_token(context);

    switch (token->type) {
        case TokenType_KeywordReturn: {
            AstNode* statement = node_new(AstNodeType_StatementReturn);

            consume_token(context);

            AstNode* expression = parse_expression(context);

            statement->data.statement_return.expression = expression;

            expect_token(context, TokenType_Semicolon);
        
            return statement;
        }

        case TokenType_KeywordIf: {
            return parse_expression(context);
        }

        case TokenType_KeywordLet: {
            AstNode* statement = node_new(AstNodeType_LetStatement);

            consume_token(context);

            statement->data.let_statement.pattern = parse_pattern(context);

            expect_token(context, TokenType_Equals);

            statement->data.let_statement.expression = parse_expression(context);

            expect_token(context, TokenType_Semicolon);

            return statement;
        }
        
        default: {
            AstNode* statement = node_new(AstNodeType_StatementExpression);

            AstNode* expression = parse_expression(context);

            statement->data.statement_expression.expression = expression;

            expect_token(context, TokenType_Semicolon);

            return statement;
        }
    } 
}

AstNode* parse_block(ParserContext* context) {
    AstNode* body = node_new(AstNodeType_Block);

    expect_token(context, TokenType_LBrace);

    while (1) {
        switch (current_token(context)->type) {
            case TokenType_RBrace:
                consume_token(context);
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

    expect_token(context, TokenType_KeywordFn);

    Token* token = expect_token(context, TokenType_Symbol);
    fn_proto->data.fn_proto.name = string_from_buffer(
        context->source.data + token->start,
        token->end - token->start
    );

    expect_token(context, TokenType_LParen);

    // parameters
    List* param_list = &fn_proto->data.fn_proto.parameters;
    while (current_token(context)->type != TokenType_RParen) { 
        AstNode* pattern = parse_pattern(context);
        list_push(AstNode*, param_list, &pattern);

        if (current_token(context)->type == TokenType_Comma) {
            consume_token(context);
        }
    }

    expect_token(context, TokenType_RParen); 

    // return statement
    AstNode* return_type;
    if (current_token(context)->type == TokenType_Arrow) {
        consume_token(context);
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

    expect_token(context, TokenType_KeywordExtern);

    extern_fn->data.extern_fn.prototype = parse_fn_proto(context);

    expect_token(context, TokenType_Semicolon);

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
                parser_error(context, "Expected function declaration");
        }
    }
}

AstNode* parse(String source, List token_list) {
    ParserContext context = {0};
    context.source = source;
    context.token_list = token_list;
    context.token_index = 0;

    AstNode* root = parse_root(&context);

    return root;
}

