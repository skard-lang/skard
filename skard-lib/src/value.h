#ifndef SKARD_VALUE_H
#define SKARD_VALUE_H

#include <stdlib.h>

typedef double SkReal;

typedef enum {
    VAL_REAL,
} ValueType;

typedef struct {
    ValueType type;
    union {
        SkReal sk_real;
    } as;
} Value;

typedef struct {
    size_t count;
    size_t capacity;
    Value *values;
} ValueArray;

void valueArrayInit(ValueArray *array);
void valueArrayFree(ValueArray *array);
void valueArrayAdd(ValueArray *array, Value value);


#endif //SKARD_VALUE_H
