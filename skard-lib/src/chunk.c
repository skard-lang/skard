#include "chunk.h"

#include "utils.h"
#include "error.h"

void debug_info_init(DebugInfo *debug_info)
{
    debug_info->lines_count = 0;
    debug_info->lines_capacity = 0;
    debug_info->lines = NULL;
    debug_info->columns_count = 0;
    debug_info->columns_capacity = 0;
    debug_info->columns = NULL;
}

void debug_info_free(DebugInfo *debug_info)
{
    SKARD_FREE_ARRAY(size_t, debug_info->lines);
    SKARD_FREE_ARRAY(size_t, debug_info->columns);
    debug_info_init(debug_info);
}

void debug_info_add_line(DebugInfo *debug_info, size_t line)
{
    if (debug_info->lines_capacity != 0 && debug_info->lines[debug_info->lines_count - 2] == line) {
        debug_info->lines[debug_info->lines_count - 1]++;
        return;
    }

    if (debug_info->lines_capacity < debug_info->lines_capacity + 2) {
        debug_info->lines_capacity = SKARD_GROW_CAPACITY(debug_info->lines_capacity);
        debug_info->lines = SKARD_GROW_ARRAY(size_t, debug_info->lines, debug_info->lines_capacity);
    }
    debug_info->lines[debug_info->lines_count] = line;
    debug_info->lines[debug_info->lines_count + 1] = 1;
    debug_info->lines_count += 2;
}

void debug_info_add_column(DebugInfo *debug_info, size_t column)
{
    if (debug_info->columns_capacity < debug_info->columns_count + 1) {
        debug_info->columns_capacity = SKARD_GROW_CAPACITY(debug_info->columns_capacity);
        debug_info->columns = SKARD_GROW_ARRAY(size_t, debug_info->columns, debug_info->columns_capacity);
    }
    debug_info->columns[debug_info->columns_count] = column;
    debug_info->columns_count++;
}

void debug_info_add(DebugInfo *debug_info, size_t line, size_t column)
{
    debug_info_add_line(debug_info, line);
    debug_info_add_column(debug_info, column);
}

size_t debug_info_read_line(DebugInfo *debug_info, size_t offset)
{
    size_t pos = 0;
    while (pos * 2 < debug_info->lines_count) {
        if (offset >= debug_info->lines[pos * 2 + 1]) {
            offset -= debug_info->lines[pos * 2 + 1];
        } else {
            return debug_info->lines[pos * 2];
        }

        pos++;
    }

    return 0;
}

size_t debug_info_read_column(DebugInfo *debug_info, size_t offset)
{
    return debug_info->columns[offset];
}


void chunk_init(Chunk *chunk)
{
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    value_array_init(&chunk->constants);
    debug_info_init(&chunk->debug_info);
}

void chunk_free(Chunk *chunk)
{
    debug_info_free(&chunk->debug_info);
    value_array_free(&chunk->constants);
    SKARD_FREE_ARRAY(uint8_t, chunk->code);
    chunk_init(chunk);
}

void chunk_write_byte(Chunk *chunk, uint8_t byte, size_t line, size_t column)
{
    if (chunk->capacity < chunk->count + 1) {
        chunk->capacity = SKARD_GROW_CAPACITY(chunk->capacity);
        chunk->code = SKARD_GROW_ARRAY(uint8_t, chunk->code, chunk->capacity);
    }
    chunk->code[chunk->count] = byte;
    chunk->count++;
    debug_info_add(&chunk->debug_info, line, column);
}

static size_t chunk_add_constant(Chunk *chunk, Value constant)
{
    value_array_add(&chunk->constants, constant);
    return chunk->constants.count - 1;
}

void chunk_write_op_constant(Chunk *chunk, Value constant, size_t line, size_t column)
{
    size_t index = chunk_add_constant(chunk, constant);
    if (index >= SKARD_MAX_CHUNK_CONSTANTS) {
        error_too_many_constants_in_chunk();
    }

    if (index <= UINT8_MAX) {
        chunk_write_byte(chunk, OP_CONSTANT, line, column);
        chunk_write_byte(chunk, index, line, column);
        return;
    }
    chunk_write_byte(chunk, OP_CONSTANT_LONG, line, column);
    chunk_write_byte(chunk, index & 0xFF, line, column);
    chunk_write_byte(chunk, (index >> 8) & 0xFF, line, column);
    chunk_write_byte(chunk, (index >> 16) & 0xFF, line, column);
}
