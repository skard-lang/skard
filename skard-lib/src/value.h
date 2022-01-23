#ifndef SKARD_VALUE_H
#define SKARD_VALUE_H

#include <stdlib.h>

typedef double SkReal;

typedef enum {
    VAL_UNKNOWN,
    VAL_REAL,
} ValueType;

typedef struct {
    ValueType type;
} SkardType;

SkardType make_skard_type_unknown(void);
SkardType make_skard_type_simple(ValueType type);

typedef struct {
    ValueType type;
    union {
        SkReal sk_real;
    } as;
} Value;

Value make_value_real(SkReal real);

void print_value(Value value);

typedef struct {
    size_t count;
    size_t capacity;
    Value *values;
} ValueArray;

void value_array_init(ValueArray *array);
void value_array_free(ValueArray *array);
void value_array_add(ValueArray *array, Value value);


#endif //SKARD_VALUE_H
