#ifndef SKARD_CHUNK_H
#define SKARD_CHUNK_H

#include <stdlib.h>
#include <stdint.h>

#include "value.h"

#define SKARD_MAX_CHUNK_CONSTANTS 16777216

typedef enum {
    OP_RETURN,
    OP_DUMP,
    OP_CONSTANT,
    OP_CONSTANT_LONG,
} OpCode;

typedef struct {
    size_t lines_count;
    size_t lines_capacity;
    size_t *lines;
    size_t columns_count;
    size_t columns_capacity;
    size_t *columns;
} DebugInfo;

void debugInfoInit(DebugInfo *debugInfo);
void debugInfoFree(DebugInfo *debugInfo);
void debugInfoAddLine(DebugInfo *debugInfo, size_t line);
void debugInfoAddColumn(DebugInfo *debugInfo, size_t column);
void debugInfoAdd(DebugInfo *debugInfo, size_t line, size_t column);
size_t debugInfoReadLine(DebugInfo *debugInfo, size_t offset);
size_t debugInfoReadColumn(DebugInfo *debugInfo, size_t offset);

typedef struct {
    size_t count;
    size_t capacity;
    uint8_t *code;
    ValueArray constants;
    DebugInfo debug_info;
} Chunk;

void chunkInit(Chunk *chunk);
void chunkFree(Chunk *chunk);
void chunkAddOpByte(Chunk *chunk, uint8_t byte, size_t line, size_t column);
void chunkAddOpConstant(Chunk *chunk, Value constant, size_t line, size_t column);

#endif //SKARD_CHUNK_H
