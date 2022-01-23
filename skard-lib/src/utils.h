#ifndef SKARD_UTILS_H
#define SKARD_UTILS_H

#include <stdlib.h>

void *reallocate(void *pointer, size_t new_size);
void *allocate(size_t new_size);

#define SKARD_GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define SKARD_GROW_ARRAY(type, pointer, new_capacity) \
    (type *) reallocate(pointer, sizeof(type) * (new_capacity))

#define SKARD_FREE_ARRAY(type, pointer) \
    (type *) reallocate(pointer, 0)

#define SKARD_ALLOCATE(type) \
    (type *) allocate(sizeof(type))

char *read_file(const char *filename);

#endif //SKARD_UTILS_H
