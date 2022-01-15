#include "error.h"

#include <stdlib.h>
#include <stdio.h>

void errorNotEnoughMemory(void) {
    fprintf(stderr, "Not enough memory");
    exit(1);
}

void errorTooManyConstantsInChunk(void) {
    fprintf(stderr, "Too many constants in one chunk");
    exit(1);
}
