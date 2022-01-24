#ifndef SKARD_COMPILER_H
#define SKARD_COMPILER_H

#include <stdbool.h>

#include "lexer.h"
#include "chunk.h"
#include "value.h"

typedef enum {
    OTOR_PLUS,
    OTOR_MINUS,
    OTOR_STAR,
    OTOR_SLASH,
    OTOR_DIV,
    COUNT_OTORS,
} ASTOperator;

const char *ast_operator_translate(ASTOperator operator);

typedef struct {
    Value value;
} ASTExpressionValue;

typedef struct {
    struct ASTNode *child;
    ASTOperator operator;
} ASTExpressionUnary;

typedef struct {
    struct ASTNode *first;
    struct ASTNode *second;
    ASTOperator operator;
} ASTExpressionBinary;

typedef struct {
    struct ASTNode *child;
} ASTExpressionGrouping;

typedef enum {
    AST_EXPR_VALUE,
    AST_EXPR_UNARY,
    AST_EXPR_BINARY,
    AST_EXPR_GROUPING,
    COUNT_AST_EXPRS,
} ASTExpressionKind;

typedef struct {
    ASTExpressionKind kind;
    SkardType type;
    union {
        ASTExpressionValue node_value;
        ASTExpressionUnary node_unary;
        ASTExpressionBinary node_binary;
        ASTExpressionGrouping node_grouping;
    } as;
} ASTNodeExpression;

typedef enum {
    AST_NODE_EXPRESSION,
    COUNT_AST_NODES,
} ASTNodeKind;

typedef struct {
    ASTNodeKind kind;
    union {
        ASTNodeExpression node_expression;
    } as;
    size_t line;
    size_t column;
} ASTNode;

void ast_node_free(ASTNode *node);

void ast_node_print(ASTNode *node, bool end_line);

typedef struct {
    Lexer *lexer;
    Token current;
    Token previous;
    bool is_error;
    bool is_panic;
} Compiler;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT, // = += -= *= /=
    PREC_OR, // or
    PREC_AND, // and
    PREC_EQUALITY, // == !=
    PREC_COMPARISON, // < > <= >=
    PREC_TERM, // + -
    PREC_FACTOR, // * /
    PREC_UNARY, // ! -
    PREC_CALL, // ()
    PREC_PRIMARY
} Precedence;

typedef ASTNode *(*ParseFnPrefix)(Compiler *);
typedef ASTNode *(*ParseFnInfix)(Compiler *, ASTNode *);

typedef struct {
    ParseFnPrefix prefix;
    ParseFnInfix infix;
    Precedence precedence;
} ParseRule;

typedef SkardType (*InferFnUnary)(Compiler *, SkardType *);
typedef SkardType (*InferFnBinary)(Compiler *, SkardType *, SkardType *);

typedef struct {
    InferFnUnary unary;
    InferFnBinary binary;
} InferRule;

void compiler_init(Compiler *compiler);

bool compiler_compile_file(Compiler *compiler, const char *filename, Chunk *chunk);
bool compiler_compile_source(Compiler *compiler, const char *source, Chunk *chunk);
bool compiler_generate_ast(Compiler *compiler);
bool compiler_generate_bytecode(Compiler *compiler, ASTNode *node);

#endif //SKARD_COMPILER_H
