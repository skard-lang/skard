#include "chunk.h"

#include "utils.h"
#include "error.h"

void debugInfoInit(DebugInfo *debugInfo) {
    debugInfo->lines_count = 0;
    debugInfo->lines_capacity = 0;
    debugInfo->lines = NULL;
    debugInfo->columns_count = 0;
    debugInfo->columns_capacity = 0;
    debugInfo->columns = NULL;
}

void debugInfoFree(DebugInfo *debugInfo) {
    SKARD_FREE_ARRAY(size_t, debugInfo->lines);
    SKARD_FREE_ARRAY(size_t, debugInfo->columns);
    debugInfoInit(debugInfo);
}

void debugInfoAddLine(DebugInfo *debugInfo, size_t line) {
    if (debugInfo->lines_capacity != 0 && debugInfo->lines[debugInfo->lines_count - 2] == line) {
        debugInfo->lines[debugInfo->lines_count - 1]++;
        return;
    }

    if (debugInfo->lines_capacity < debugInfo->lines_capacity + 2) {
        debugInfo->lines_capacity = SKARD_GROW_CAPACITY(debugInfo->lines_capacity);
        debugInfo->lines = SKARD_GROW_ARRAY(size_t, debugInfo->lines, debugInfo->lines_capacity);
    }
    debugInfo->lines[debugInfo->lines_count] = line;
    debugInfo->lines[debugInfo->lines_count + 1] = 1;
    debugInfo->lines_count += 2;
}

void debugInfoAddColumn(DebugInfo *debugInfo, size_t column) {
    if (debugInfo->columns_capacity < debugInfo->columns_count + 1) {
        debugInfo->columns_capacity = SKARD_GROW_CAPACITY(debugInfo->columns_capacity);
        debugInfo->columns = SKARD_GROW_ARRAY(size_t, debugInfo->columns, debugInfo->columns_capacity);
    }
    debugInfo->columns[debugInfo->columns_count] = column;
    debugInfo->columns_count++;
}

void debugInfoAdd(DebugInfo *debugInfo, size_t line, size_t column) {
    debugInfoAddLine(debugInfo, line);
    debugInfoAddColumn(debugInfo, column);
}

size_t debugInfoReadLine(DebugInfo *debugInfo, size_t offset) {
    size_t pos = 0;
    while (pos * 2 < debugInfo->lines_count) {
        if (offset >= debugInfo->lines[pos * 2 + 1]) {
            offset -= debugInfo->lines[pos * 2 + 1];
        } else {
            return debugInfo->lines[pos * 2];
        }

        pos++;
    }

    return 0;
}

size_t debugInfoReadColumn(DebugInfo *debugInfo, size_t offset) {
    return debugInfo->columns[offset];
}


void chunkInit(Chunk *chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    valueArrayInit(&chunk->constants);
    debugInfoInit(&chunk->debug_info);
}

void chunkFree(Chunk *chunk) {
    debugInfoFree(&chunk->debug_info);
    valueArrayFree(&chunk->constants);
    SKARD_FREE_ARRAY(uint8_t, chunk->code);
    chunkInit(chunk);
}

void chunkAddOpByte(Chunk *chunk, uint8_t byte, size_t line, size_t column) {
    if (chunk->capacity < chunk->count + 1) {
        chunk->capacity = SKARD_GROW_CAPACITY(chunk->capacity);
        chunk->code = SKARD_GROW_ARRAY(uint8_t, chunk->code, chunk->capacity);
    }
    chunk->code[chunk->count] = byte;
    chunk->count++;
    debugInfoAdd(&chunk->debug_info, line, column);
}

static size_t chunkAddConstant(Chunk *chunk, Value constant) {
    valueArrayAdd(&chunk->constants, constant);
    return chunk->constants.count - 1;
}

void chunkAddOpConstant(Chunk *chunk, Value constant, size_t line, size_t column) {
    size_t index = chunkAddConstant(chunk, constant);
    if (index >= SKARD_MAX_CHUNK_CONSTANTS) {
        errorTooManyConstantsInChunk();
    }

    if (index <= UINT8_MAX) {
        chunkAddOpByte(chunk, OP_CONSTANT, line, column);
        chunkAddOpByte(chunk, index, line, column);
        return;
    }
    chunkAddOpByte(chunk, OP_CONSTANT_LONG, line, column);
    chunkAddOpByte(chunk, index & 0xFF, line, column);
    chunkAddOpByte(chunk, (index >> 8) & 0xFF, line, column);
    chunkAddOpByte(chunk, (index >> 16) & 0xFF, line, column);
}
