#include "utils.h"

#include <stdio.h>

#include "error.h"

void *reallocate(void *pointer, size_t new_size) {
    if (new_size == 0) {
        if (pointer != NULL) {
            free(pointer);
        }
        return NULL;
    }

    void *result = realloc(pointer, new_size);
    if (result == NULL) {
        error_not_enough_memory();
    }

    return result;
}

void *allocate(size_t new_size) {
    void *result = malloc(new_size);
    if (result == NULL) {
        error_not_enough_memory();
    }

    return result;
}


char *read_file(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        error_could_not_open_file(filename);
    }

    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    char *buffer = (char *) malloc(file_size + 1);
    if (buffer == NULL) {
        error_not_enough_memory_to_read(filename);
        return NULL;
    }

    size_t read_bytes = fread(buffer, sizeof(char), file_size, file);
    if (read_bytes < file_size) {
        error_could_not_read(filename);
    }
    buffer[read_bytes] = '\0';

    fclose(file);
    return buffer;
}
