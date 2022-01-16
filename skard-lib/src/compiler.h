#ifndef SKARD_COMPILER_H
#define SKARD_COMPILER_H

#include <stdbool.h>

#include "lexer.h"
#include "chunk.h"

typedef enum {
    CONSTANT
} ASTNodeType;

typedef struct {
    ASTNodeType type;
} ASTNode;

typedef struct {
    Token current;
} Compiler;

bool compiler_compile_file(Compiler *compiler, const char *filename, Chunk *chunk);

#endif //SKARD_COMPILER_H
