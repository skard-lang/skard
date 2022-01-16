#include "utils.h"

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
