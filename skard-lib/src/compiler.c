#include "compiler.h"

#include <stdio.h>
#include <assert.h>

#include "utils.h"


static void ast_node_expression_free(ASTNodeExpression *node);

static void ast_node_expression_free(ASTNodeExpression *node)
{
    switch (node->kind) {
        case AST_EXPR_VALUE:
            break;
        case AST_EXPR_UNARY:
            ast_node_free((ASTNode *) node->as.node_unary.child);
            break;
        case AST_EXPR_BINARY:
            ast_node_free((ASTNode *) node->as.node_binary.first);
            ast_node_free((ASTNode *) node->as.node_binary.second);
            break;
        case AST_EXPR_GROUPING:
            ast_node_free((ASTNode *) node->as.node_grouping.child);
            break;
    }
}


void ast_node_free(ASTNode *node)
{
    switch (node->kind) {
        case AST_NODE_EXPRESSION:
            ast_node_expression_free(&node->as.node_expression);
    }

    free(node);
}


static void ast_print_invalid(void);

static void ast_expression_value_print(ASTExpressionValue *value);
static void ast_expression_unary_print(ASTExpressionUnary *unary);
static void ast_expression_binary_print(ASTExpressionBinary *binary);
static void ast_expression_grouping_print(ASTExpressionGrouping *grouping);

static void ast_node_expression_print(ASTNodeExpression *expression);


static void ast_print_invalid(void)
{
    printf("INVALID");
}


static void ast_expression_value_print(ASTExpressionValue *value)
{
    print_value(value->value);
}

static void ast_expression_unary_print(ASTExpressionUnary *unary)
{
    switch (unary->operator) {
        case OTOR_MINUS:
            printf("-");
            break;
        default:
            ast_print_invalid();
            break;
    }
    printf(" ");

    ast_node_print((ASTNode *) unary->child, false);
}

static void ast_expression_binary_print(ASTExpressionBinary *binary)
{
    switch (binary->operator) {
        case OTOR_PLUS:
            printf("+");
            break;
        case OTOR_MINUS:
            printf("-");
            break;
        case OTOR_STAR:
            printf("*");
            break;
        case OTOR_SLASH:
            printf("/");
            break;
        case OTOR_DIV:
            printf("|");
    }
    printf(" ");

    ast_node_print((ASTNode *) binary->first, false);
    ast_node_print((ASTNode *) binary->second, false);
}

static void ast_expression_grouping_print(ASTExpressionGrouping *grouping)
{
    printf("gr ");
    ast_node_print((ASTNode *) grouping->child, false);
}


static void ast_node_expression_print(ASTNodeExpression *expression)
{
    switch (expression->type.type) {
        case VAL_UNKNOWN:
            printf("UNKNOWN");
            break;
        case VAL_REAL:
            printf("REAL");
            break;
    }
    printf(" ");

    switch (expression->kind) {
        case AST_EXPR_VALUE:
            ast_expression_value_print(&expression->as.node_value);
            break;
        case AST_EXPR_UNARY:
            ast_expression_unary_print(&expression->as.node_unary);
            break;
        case AST_EXPR_BINARY:
            ast_expression_binary_print(&expression->as.node_binary);
            break;
        case AST_EXPR_GROUPING:
            ast_expression_grouping_print(&expression->as.node_grouping);
            break;
    }
}


void ast_node_print(ASTNode *node, bool end_line)
{
    printf("(");

    switch (node->kind) {
        case AST_NODE_EXPRESSION:
            ast_node_expression_print(&node->as.node_expression);
    }

    printf(")");
    if (end_line) {
        printf("\n");
    }
}


void compiler_init(Compiler *compiler)
{
    compiler->lexer = NULL;
    compiler->is_error = false;
    compiler->is_panic = false;
}


bool compiler_compile_file(Compiler *compiler, const char *filename, Chunk *chunk)
{
    char *source = read_file(filename);
    bool result = compiler_compile_source(compiler, source, chunk);
    free(source);

    return result;
}

bool compiler_compile_source(Compiler *compiler, const char *source, Chunk *chunk)
{
    (void) compiler;
    (void) chunk;

    Lexer lexer;
    compiler->lexer = &lexer;
    lexer_init(compiler->lexer, source);
    lexer_print(compiler->lexer);

    lexer_reset(compiler->lexer);
    compiler_generate_ast(compiler);

    return false;
}

static ASTNode *make_ast_node_expression(ASTNodeExpression node_expression);
static ASTNode *make_ast_node_value(Value value);
static ASTNode *make_ast_node_unary(ASTNode *child, ASTOperator operator);
static ASTNode *make_ast_node_binary(ASTNode *first, ASTNode *second, ASTOperator operator);
static ASTNode *make_ast_node_grouping(ASTNode *child);

static void compiler_handle_error_at_current(Compiler *compiler, const char *message);
static void compiler_handle_error_at_previous(Compiler *compiler, const char *message);
static void compiler_handle_error(Compiler *compiler, Token *token, const char *message);
static void compiler_advance(Compiler *compiler);
static void compiler_consume(Compiler *compiler, TokenType type, const char *message);

static ParseRule *get_parse_rule(TokenType type);

static ASTNode *compiler_parse_precedence(Compiler *compiler, Precedence precedence);
static ASTNode *compiler_parse_expression(Compiler *compiler);
static ASTNode *compiler_parse_grouping(Compiler *compiler);
static ASTNode *compiler_parse_binary(Compiler *compiler, ASTNode *first);
static ASTNode *compiler_parse_unary(Compiler *compiler);
static ASTNode *compiler_parse_real(Compiler *compiler);


static ASTNode *make_ast_node_expression(ASTNodeExpression node_expression)
{
    ASTNode *node = SKARD_ALLOCATE(ASTNode);
    node->kind = AST_NODE_EXPRESSION;
    node->as.node_expression = node_expression;
    return node;
}

static ASTNode *make_ast_node_value(Value value)
{
    ASTNodeExpression node_expression;
    node_expression.kind = AST_EXPR_VALUE;
    node_expression.type = make_skard_type_unknown();
    node_expression.as.node_value = (ASTExpressionValue) { .value = value };

    return make_ast_node_expression(node_expression);
}

static ASTNode *make_ast_node_unary(ASTNode *child, ASTOperator operator)
{
    ASTNodeExpression node_expression;
    node_expression.kind = AST_EXPR_UNARY;
    node_expression.type = make_skard_type_unknown();
    node_expression.as.node_unary = (ASTExpressionUnary) { .child = (struct ASTNode *) child, .operator = operator };

    return make_ast_node_expression(node_expression);
}

static ASTNode *make_ast_node_binary(ASTNode *first, ASTNode *second, ASTOperator operator)
{
    ASTNodeExpression node_expression;
    node_expression.kind = AST_EXPR_BINARY;
    node_expression.type = make_skard_type_unknown();
    node_expression.as.node_binary = (ASTExpressionBinary) {
        .first = (struct ASTNode *) first,
        .second = (struct ASTNode *) second,
        .operator = operator };

    return make_ast_node_expression(node_expression);
}

static ASTNode *make_ast_node_grouping(ASTNode *child)
{
    ASTNodeExpression node_expression;
    node_expression.kind = AST_EXPR_GROUPING;
    node_expression.type = make_skard_type_unknown();
    node_expression.as.node_grouping = (ASTExpressionGrouping) { .child = (struct ASTNode *) child };

    return make_ast_node_expression(node_expression);
}


static void compiler_handle_error_at_current(Compiler *compiler, const char *message)
{
    compiler_handle_error(compiler, &compiler->current, message);
}

static void compiler_handle_error_at_previous(Compiler *compiler, const char *message)
{
    compiler_handle_error(compiler, &compiler->previous, message);
}

static void compiler_handle_error(Compiler *compiler, Token *token, const char *message)
{
    if (compiler->is_panic) {
        return;
    }
    compiler->is_panic = true;
    fprintf(stderr, "[line %zu][column %zu] Error ", token->line, token->column);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, "at end of file");
    } else if (token->type == TOKEN_EOL) {
        fprintf(stderr, "at end of line");
    } else if (token->type != TOKEN_ERROR) {
        fprintf(stderr, "at '%.*s'", (int) token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    compiler->is_error = true;
}


static void compiler_advance(Compiler *compiler)
{
    compiler->previous = compiler->current;

    while (true) {
        compiler->current = lexer_scan_token(compiler->lexer);
        if (compiler->current.type != TOKEN_ERROR) {
            break;
        }

        compiler_handle_error_at_current(compiler, compiler->current.start);
    }
}

static void compiler_consume(Compiler *compiler, TokenType type, const char *message)
{
    if (compiler->current.type == type) {
        compiler_advance(compiler);
        return;
    }

    compiler_handle_error_at_current(compiler, message);
}


static ParseRule parse_rules[] = {
        [TOKEN_EOF] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_EOL] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_ERROR] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_LEFT_PAREN] = { .prefix = compiler_parse_grouping, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_RIGHT_PAREN] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_LEFT_BRACE] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_RIGHT_BRACE] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_RIGHT_BRACKET] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_LEFT_BRACKET] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_DOT] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_COMMA] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_COLON] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_PLUS] = { .prefix = NULL, .infix = compiler_parse_binary, .precedence = PREC_TERM },
        [TOKEN_PLUS_ASSIGN] = { .prefix = NULL, .infix = NULL, .precedence = PREC_TERM },
        [TOKEN_MINUS] = { .prefix = compiler_parse_unary, .infix = compiler_parse_binary, .precedence = PREC_TERM },
        [TOKEN_MINUS_ASSIGN] = { .prefix = NULL, .infix = NULL, .precedence = PREC_TERM },
        [TOKEN_RIGHT_ARROW] = { .prefix = NULL, .infix = NULL, .precedence = PREC_TERM },
        [TOKEN_STAR] = { .prefix = NULL, .infix = compiler_parse_binary, .precedence = PREC_FACTOR },
        [TOKEN_STAR_ASSIGN] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_SLASH] = { .prefix = NULL, .infix = compiler_parse_binary, .precedence = PREC_FACTOR },
        [TOKEN_SLASH_ASSIGN] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_AT] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_NOT] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_NOT_EQUAL] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_ASSIGN] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_EQUAL] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_GREATER] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_GREATER_EQUAL] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_LESS] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_LESS_EQUAL] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_DIV] = { .prefix = NULL, .infix = compiler_parse_binary, .precedence = PREC_FACTOR },
        [TOKEN_PIPE] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_OR] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_AND] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_KEY_PACKAGE] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_KEY_IMPORT] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_KEY_STRUCT] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_KEY_SELF] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_KEY_LET] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_KEY_NIL] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_KEY_FN] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_KEY_RETURN] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_KEY_IF] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_KEY_ELSE] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_KEY_WHILE] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_KEY_FOR] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_KEY_TRUE] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_KEY_FALSE] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_KEY_MATCH] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_KEY_WITH] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_KEY_DUMP] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_IDENTIFIER] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_LIT_STRING] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_LIT_REAL] = { .prefix = compiler_parse_real, .infix = NULL, .precedence = PREC_NONE },
        [TOKEN_LIT_INT] = { .prefix = NULL, .infix = NULL, .precedence = PREC_NONE },
};


static ParseRule *get_parse_rule(TokenType type)
{
    assert(COUNT_TOKENS == 55);
    return &parse_rules[type];
}


static ASTNode *compiler_parse_precedence(Compiler *compiler, Precedence precedence) // TODO: add support for multiline expressions
{
    compiler_advance(compiler);
    ParseFnPrefix prefix_rule = get_parse_rule(compiler->previous.type)->prefix;
    if (prefix_rule == NULL) {
        compiler_handle_error_at_previous(compiler, "Expected expression.");
        return NULL;
    }

    ASTNode *first = prefix_rule(compiler);

    while (precedence <= get_parse_rule(compiler->current.type)->precedence) {
        compiler_advance(compiler);
        ParseFnInfix infix_rule = get_parse_rule(compiler->previous.type)->infix;
        first = infix_rule(compiler, first);
    }

    return first;
}

static ASTNode *compiler_parse_expression(Compiler *compiler)
{
    return compiler_parse_precedence(compiler, PREC_ASSIGNMENT);
}

static ASTNode *compiler_parse_grouping(Compiler *compiler)
{
    ASTNode *child = compiler_parse_expression(compiler);
    compiler_consume(compiler, TOKEN_RIGHT_PAREN, "Expected ')' after expression.");
    return make_ast_node_grouping(child);
}

static ASTNode *compiler_parse_binary(Compiler *compiler, ASTNode *first)
{
    TokenType operator_type = compiler->previous.type;
    ParseRule *rule = get_parse_rule(operator_type);
    ASTNode *second = compiler_parse_precedence(compiler, (Precedence) (rule->precedence + 1));

    ASTOperator ast_operator;
    switch (operator_type) {
        case TOKEN_PLUS:
            ast_operator = OTOR_PLUS;
            break;
        case TOKEN_MINUS:
            ast_operator = OTOR_MINUS;
            break;
        case TOKEN_STAR:
            ast_operator = OTOR_STAR;
            break;
        case TOKEN_SLASH:
            ast_operator = OTOR_SLASH;
            break;
        case TOKEN_DIV:
            ast_operator = OTOR_DIV;
            break;
        default:
            assert(false); // TODO: Handle unreachable code in a better way
            return NULL; // Unreachable
    }

    return make_ast_node_binary(first, second, ast_operator);
}

static ASTNode *compiler_parse_unary(Compiler *compiler)
{
    TokenType operator_type = compiler->previous.type;
    ASTNode *child = compiler_parse_precedence(compiler, PREC_UNARY);

    ASTOperator ast_operator;
    switch (operator_type) {
        case TOKEN_MINUS:
            ast_operator = OTOR_MINUS;
            break;
        default:
            assert(false); // TODO: Handle unreachable code in a better way
            return NULL; // Unreachable
    }

    return make_ast_node_unary(child, ast_operator);
}

static ASTNode *compiler_parse_real(Compiler *compiler)
{
    SkReal real = strtod(compiler->previous.start, NULL);
    ASTNode *node = make_ast_node_value(make_value_real(real));
    return node;
}


bool compiler_generate_ast(Compiler *compiler)
{
    compiler_advance(compiler);
    ASTNode *ast = compiler_parse_expression(compiler);
    compiler_consume(compiler, TOKEN_EOF, "Expected end of expression.");

    if (ast != NULL) {
        ast_node_print(ast, true);
        ast_node_free(ast);
    }

    return !compiler->is_error;
}
