#include "error.h"

#include <stdlib.h>
#include <stdio.h>

void error_not_enough_memory(void) {
    fprintf(stderr, "Not enough memory");
    exit(1);
}

void error_too_many_constants_in_chunk(void) {
    fprintf(stderr, "Too many constants in one chunk");
    exit(1);
}
