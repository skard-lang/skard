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
    OTOR_SLASH
} ASTOperator;

typedef struct {
    Value value;
} ASTNodeValue;

typedef struct {
    struct ASTNode *child;
    ASTOperator operator;
} ASTNodeUnary;

typedef struct {
    struct ASTNode *first;
    struct ASTNode *second;
    ASTOperator operator;
} ASTNodeBinary;

typedef struct {
    struct ASTNode *child;
} ASTNodeGrouping;

typedef enum {
    AST_NODE_VALUE,
    AST_NODE_UNARY,
    AST_NODE_BINARY,
    AST_NODE_GROUPING,
} ASTNodeType;

typedef struct {
    ASTNodeType node_type;
    ValueType value_type;
    union {
        ASTNodeValue node_value;
        ASTNodeUnary node_unary;
        ASTNodeBinary node_binary;
        ASTNodeGrouping node_grouping;
    } as;
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

void compiler_init(Compiler *compiler);

bool compiler_compile_file(Compiler *compiler, const char *filename, Chunk *chunk);
bool compiler_compile_source(Compiler *compiler, const char *source, Chunk *chunk);
bool compiler_generate_ast(Compiler *compiler);

#endif //SKARD_COMPILER_H
