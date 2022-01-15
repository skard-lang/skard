#ifndef SKARD_UTILS_H
#define SKARD_UTILS_H

#include <stdlib.h>

void *reallocate(void *pointer, size_t newSize);
void *allocate(size_t newSize);

#define SKARD_GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define SKARD_GROW_ARRAY(type, pointer, newCapacity) \
    (type *) reallocate(pointer, sizeof(type) * (newCapacity))

#define SKARD_FREE_ARRAY(type, pointer) \
    (type *) reallocate(pointer, 0)

#endif //SKARD_UTILS_H
