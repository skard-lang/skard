#include <stdio.h>

#include "skard.h"

int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    SkardVM vm;
    vm_init(&vm);

    Chunk c;
    chunk_init(&c);

    Value a = { .type = TYPE_REAL, .as.sk_real = 0.42 };
    chunk_write_op_constant(&c, a, 0, 0);
    chunk_write_byte(&c, OP_DUMP, 1, 0);
    chunk_write_byte(&c, OP_RETURN, 2, 0);

    disassemble_chunk(&c, "TEST");

    printf("RUNNING...\n");
    vm_run(&vm, &c);

    chunk_free(&c);

    vm_free(&vm);

    printf("Test tokenize...\n");
    Compiler compiler;
    Chunk chunk;
    compiler_init(&compiler);
    compiler_compile_file(&compiler, "examples/basic/00_dump.sk", &chunk);

    return 0;
}
