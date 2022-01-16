#include "debug.h"

#include <stdio.h>

void disassemble_chunk(Chunk *chunk, const char *name)
{
    printf("DISASSEMBLING CHUNK: %s\n", name);
    printf("OFFSET |  LINE  | COLUMN |      OPCODE      | OPERAND | VALUE\n");
    printf("-------------------------------------------------------------\n");

    size_t offset = 0;
    while (offset < chunk->count) {
        offset = disassemble_instruction(chunk, offset);
        printf("\n");
    }
    printf("-------------------------------------------------------------\n");
}

static void print_instruction_name(const char *name)
{
    printf("%-16s | ", name);
}

static size_t disassemble_unknown_instruction(size_t offset)
{
    print_instruction_name("UNKNOWN");
    return offset + 1;
}

static size_t disassemble_simple_instruction(const char *name, size_t offset)
{
    print_instruction_name(name);
    return offset + 1;
}

static size_t disassemble_constant_instruction(const char *name, size_t offset, Chunk *chunk)
{
    print_instruction_name(name);
    size_t index = chunk->code[offset + 1];
    printf("%07zu | ", index);
    print_value(chunk->constants.values[index]);
    return offset + 2;
}

static size_t disassemble_constant_long_instruction(const char *name, size_t offset, Chunk *chunk)
{
    print_instruction_name(name);
    size_t index = chunk->code[offset + 1] | (chunk->code[offset + 2] << 8) | (chunk->code[offset + 3] << 16);
    printf("%07zu | ", index);
    print_value(chunk->constants.values[index]);
    return offset + 4;
}

size_t disassemble_instruction(Chunk *chunk, size_t offset)
{
    printf("%06zu | ", offset);
    size_t line = debug_info_read_line(&chunk->debug_info, offset);
    size_t column = debug_info_read_column(&chunk->debug_info, offset);
    printf("%06zu | ", line);
    printf("%06zu | ", column);

    uint8_t byte = chunk->code[offset];
    switch (byte) {
        case OP_RETURN:
            return disassemble_simple_instruction("OP_RETURN", offset);
        case OP_DUMP:
            return disassemble_simple_instruction("OP_DUMP", offset);
        case OP_CONSTANT:
            return disassemble_constant_instruction("OP_CONSTANT", offset, chunk);
        case OP_CONSTANT_LONG:
            return disassemble_constant_long_instruction("OP_CONSTANT_LONG", offset, chunk);
        default:
            return disassemble_unknown_instruction(offset);
    }
}
