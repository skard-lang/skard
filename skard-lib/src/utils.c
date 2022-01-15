#include "utils.h"

#include "error.h"

void *reallocate(void *pointer, size_t newSize) {
    if (newSize == 0) {
        if (pointer != NULL) {
            free(pointer);
        }
        return NULL;
    }

    void *result = realloc(pointer, newSize);
    if (result == NULL) {
        errorNotEnoughMemory();
    }

    return result;
}

void *allocate(size_t newSize) {
    void *result = malloc(newSize);
    if (result == NULL) {
        errorNotEnoughMemory();
    }

    return result;
}
