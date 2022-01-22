#include "compiler.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "utils.h"


void ast_node_free(ASTNode *node)
{
    switch (node->node_type) {
        case AST_NODE_VALUE:
            break;
        case AST_NODE_UNARY:
            ast_node_free((ASTNode *) node->as.node_unary.child);
            break;
        case AST_NODE_BINARY:
            ast_node_free((ASTNode *) node->as.node_binary.first);
            ast_node_free((ASTNode *) node->as.node_binary.second);
            break;
        case AST_NODE_GROUPING:
            ast_node_free((ASTNode *) node->as.node_grouping.child);
            break;
    }

    free(node);
}

static void ast_print_invalid();

static void ast_node_print_value(ASTNodeValue value);
static void ast_node_print_unary(ASTNodeUnary unary);
static void ast_node_print_binary(ASTNodeBinary binary);
static void ast_node_print_grouping(ASTNodeGrouping grouping);


static void ast_print_invalid()
{
    printf("INVALID");
}


static void ast_node_print_value(ASTNodeValue value)
{
    print_value(value.value);
}

static void ast_node_print_unary(ASTNodeUnary unary)
{
    switch (unary.operator) {
        case OTOR_MINUS:
            printf("-");
            break;
        default:
            ast_print_invalid();
            break;
    }
    printf(" ");

    ast_node_print((ASTNode *) unary.child, false);
}

static void ast_node_print_binary(ASTNodeBinary binary)
{
    switch (binary.operator) {
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
    }
    printf(" ");

    ast_node_print((ASTNode *) binary.first, false);
    ast_node_print((ASTNode *) binary.second, false);
}

static void ast_node_print_grouping(ASTNodeGrouping grouping)
{
    printf("gr ");
    ast_node_print((ASTNode *) grouping.child, false);
}


void ast_node_print(ASTNode *node, bool end_line)
{
    printf("(");

    switch (node->value_type) {
        case VAL_REAL:
            printf("REAL");
            break;
    }
    printf(" ");

    switch (node->node_type) {
        case AST_NODE_VALUE:
            ast_node_print_value(node->as.node_value);
            break;
        case AST_NODE_UNARY:
            ast_node_print_unary(node->as.node_unary);
            break;
        case AST_NODE_BINARY:
            ast_node_print_binary(node->as.node_binary);
            break;
        case AST_NODE_GROUPING:
            ast_node_print_grouping(node->as.node_grouping);
            break;
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


static ASTNode *make_ast_node_value(Value value);
static ASTNode *make_ast_node_unary(ASTNode *child, ValueType type, ASTOperator operator);
static ASTNode *make_ast_node_binary(ASTNode *first, ASTNode *second, ValueType type, ASTOperator operator);
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


static ASTNode *make_ast_node_value(Value value)
{
    ASTNode *node = SKARD_ALLOCATE(ASTNode);
    node->node_type = AST_NODE_VALUE;
    node->value_type = value.type;
    node->as.node_value = (ASTNodeValue) { .value = value };
    return node;
}

static ASTNode *make_ast_node_unary(ASTNode *child, ValueType type, ASTOperator operator)
{
    ASTNode *node = SKARD_ALLOCATE(ASTNode);
    node->node_type = AST_NODE_UNARY;
    node->value_type = type;
    node->as.node_unary = (ASTNodeUnary) { .child = (struct ASTNode *) child, .operator = operator };
    return node;
}

static ASTNode *make_ast_node_binary(ASTNode *first, ASTNode *second, ValueType type, ASTOperator operator)
{
    ASTNode *node = SKARD_ALLOCATE(ASTNode);
    node->node_type = AST_NODE_BINARY;
    node->value_type = type;
    node->as.node_binary = (ASTNodeBinary) { .first = (struct ASTNode *) first, .second = (struct ASTNode *) second, .operator = operator };
    return node;
}

static ASTNode *make_ast_node_grouping(ASTNode *child)
{
    ASTNode *node = SKARD_ALLOCATE(ASTNode);
    node->node_type = AST_NODE_GROUPING;
    node->value_type = child->value_type;
    node->as.node_grouping = (ASTNodeGrouping) { .child = (struct ASTNode *) child };
    return node;
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


static ASTNode *compiler_parse_precedence(Compiler *compiler, Precedence precedence)
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

    switch (operator_type) {
        case TOKEN_PLUS:
            if (first->value_type == VAL_REAL && second->value_type == VAL_REAL) {
                return make_ast_node_binary(first, second, VAL_REAL, OTOR_PLUS);
            }

            break;
        case TOKEN_MINUS:
            if (first->value_type == VAL_REAL && second->value_type == VAL_REAL) {
                return make_ast_node_binary(first, second, VAL_REAL, OTOR_MINUS);
            }

            break;
        case TOKEN_STAR:
            if (first->value_type == VAL_REAL && second->value_type == VAL_REAL) {
                return make_ast_node_binary(first, second, VAL_REAL, OTOR_STAR);
            }

            break;
        case TOKEN_SLASH:
            if (first->value_type == VAL_REAL && second->value_type == VAL_REAL) {
                return make_ast_node_binary(first, second, VAL_REAL, OTOR_SLASH);
            }

            break;
        default:
            break; // Unreachable
    }

    // compiler_handle_error(compiler, ) TODO: Here we need to handle type error
    return NULL;
}

static ASTNode *compiler_parse_unary(Compiler *compiler)
{
    TokenType operator_type = compiler->previous.type;
    ASTNode *child = compiler_parse_precedence(compiler, PREC_UNARY);

    switch (operator_type) {
        case TOKEN_MINUS:
            return make_ast_node_unary(child, child->value_type, OTOR_MINUS);
        default:
            return NULL; // Unreachable
    }
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

    ast_node_print(ast, true);
    ast_node_free(ast);

    return !compiler->is_error;
}
