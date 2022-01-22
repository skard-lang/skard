#include "value.h"

#include <stdio.h>

#include "utils.h"


Value make_value_real(SkReal real)
{
    return (Value) { .type = VAL_REAL, .as.sk_real = real };
}


void print_value(Value value)
{
    switch (value.type) {
        case VAL_REAL:
            printf("%lf", value.as.sk_real);
            break;
        default:
            printf("UNKNOWN TYPE");
    }
}


void value_array_init(ValueArray *array) {
    array->count = 0;
    array->capacity = 0;
    array->values = NULL;
}

void value_array_free(ValueArray *array) {
    SKARD_FREE_ARRAY(Value, array->values);
    value_array_init(array);
}

void value_array_add(ValueArray *array, Value value) {
    if (array->capacity < array->count + 1) {
        array->capacity = SKARD_GROW_CAPACITY(array->capacity);
        array->values = SKARD_GROW_ARRAY(Value, array->values, array->capacity);
    }
    array->values[array->count] = value;
    array->count++;
}
