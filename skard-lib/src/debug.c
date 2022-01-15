#include "debug.h"

#include <stdio.h>

void disassembleChunk(Chunk *chunk, const char *name) {
    printf("DISASSEMBLING CHUNK: %s\n", name);
    printf("OFFSET |  LINE  | COLUMN |      OPCODE      | OPERAND | VALUE\n");
    printf("-------------------------------------------------------------\n");

    size_t offset = 0;
    while (offset < chunk->count) {
        offset = disassembleInstruction(chunk, offset);
        printf("\n");
    }
    printf("-------------------------------------------------------------\n");
}

static void printInstructionName(const char *name) {
    printf("%-16s | ", name);
}

static void printValue(Value value) {
    switch (value.type) {
        case VAL_REAL:
            printf("%lf", value.as.sk_real);
            break;
        default:
            printf("UNKNOWN");
    }
}

static size_t unknownInstruction(size_t offset) {
    printInstructionName("UNKNOWN");
    return offset + 1;
}

static size_t simpleInstruction(const char *name, size_t offset) {
    printInstructionName(name);
    return offset + 1;
}

static size_t constantInstruction(const char *name, size_t offset, Chunk *chunk) {
    printInstructionName(name);
    size_t index = chunk->code[offset + 1];
    printf("%07zu | ", index);
    printValue(chunk->constants.values[index]);
    return offset + 2;
}

static size_t constantLongInstruction(const char *name, size_t offset, Chunk *chunk) {
    printInstructionName(name);
    size_t index = chunk->code[offset + 1] | (chunk->code[offset + 2] << 8) | (chunk->code[offset + 3] << 16);
    printf("%07zu | ", index);
    printValue(chunk->constants.values[index]);
    return offset + 4;
}

size_t disassembleInstruction(Chunk *chunk, size_t offset) {
    printf("%06zu | ", offset);
    size_t line = debugInfoReadLine(&chunk->debug_info, offset);
    size_t column = debugInfoReadColumn(&chunk->debug_info, offset);
    printf("%06zu | ", line);
    printf("%06zu | ", column);

    uint8_t byte = chunk->code[offset];
    switch (byte) {
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        case OP_DUMP:
            return simpleInstruction("OP_DUMP", offset);
        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", offset, chunk);
        case OP_CONSTANT_LONG:
            return constantLongInstruction("OP_CONSTANT_LONG", offset, chunk);
        default:
            return unknownInstruction(offset);
    }
}
