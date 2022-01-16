#include "error.h"

#include <stdlib.h>
#include <stdio.h>

void error_not_enough_memory(void)
{
    fprintf(stderr, "Not enough memory\n");
    exit(1);
}

void error_too_many_constants_in_chunk(void)
{
    fprintf(stderr, "Too many constants in one chunk\n");
    exit(1);
}

void error_could_not_open_file(const char *filename)
{
    fprintf(stderr, "Could not open file \"%s\"\n", filename);
    exit(1);
}

void error_not_enough_memory_to_read(const char *filename)
{
    fprintf(stderr, "Not enough memory to read file \"%s\"\n", filename);
    exit(1);
}

void error_could_not_read(const char *filename)
{
    fprintf(stderr, "Could not read file \"%s\"\n", filename);
    exit(1);
}