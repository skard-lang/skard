#include <stdio.h>

#include "skard.h"

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;

    Chunk c;
    chunkInit(&c);

    Value a = { .type = VAL_REAL, .as.sk_real = 0.42 };
    chunkAddOpConstant(&c, a, 0, 0);
    chunkAddOpByte(&c, OP_DUMP, 1, 0);
    chunkAddOpByte(&c, OP_RETURN, 2, 0);

    disassembleChunk(&c, "TEST");

    chunkFree(&c);

    return 0;
}
