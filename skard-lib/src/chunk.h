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
    COUNT_OPS
} OpCode;

typedef struct {
    size_t lines_count;
    size_t lines_capacity;
    size_t *lines;
    size_t columns_count;
    size_t columns_capacity;
    size_t *columns;
} DebugInfo;

void debug_info_init(DebugInfo *debug_info);
void debug_info_free(DebugInfo *debug_info);
void debug_info_add_line(DebugInfo *debug_info, size_t line);
void debug_info_add_column(DebugInfo *debug_info, size_t column);
void debug_info_add(DebugInfo *debug_info, size_t line, size_t column);
size_t debug_info_read_line(DebugInfo *debug_info, size_t offset);
size_t debug_info_read_column(DebugInfo *debug_info, size_t offset);

typedef struct {
    size_t count;
    size_t capacity;
    uint8_t *code;
    ValueArray constants;
    DebugInfo debug_info;
} Chunk;

void chunk_init(Chunk *chunk);
void chunk_free(Chunk *chunk);
void chunk_write_byte(Chunk *chunk, uint8_t byte, size_t line, size_t column);
void chunk_write_op_constant(Chunk *chunk, Value constant, size_t line, size_t column);

#endif //SKARD_CHUNK_H
