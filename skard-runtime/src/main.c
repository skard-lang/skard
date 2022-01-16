#include <stdio.h>

#include "skard.h"

int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    Chunk c;
    chunk_init(&c);

    Value a = { .type = VAL_REAL, .as.sk_real = 0.42 };
    chunk_write_op_constant(&c, a, 0, 0);
    chunk_write_byte(&c, OP_DUMP, 1, 0);
    chunk_write_byte(&c, OP_RETURN, 2, 0);

    disassemble_chunk(&c, "TEST");

    chunk_free(&c);

    return 0;
}
