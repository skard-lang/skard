#include "value.h"

#include <stdio.h>
#include <assert.h>

#include "utils.h"


SkardType make_skard_type_simple(TypeKind type)
{
    SkardType skard_type;
    skard_type.type = type;
    return skard_type;
}

SkardType make_skard_type_unknown(void)
{
    return make_skard_type_simple(TYPE_UNKNOWN);
}

SkardType make_skard_type_invalid()
{
    return make_skard_type_simple(TYPE_INVALID);
}

SkardType make_skard_type_real(void)
{
    return make_skard_type_simple(TYPE_REAL);
}

SkardType make_skard_type_int(void)
{
    return make_skard_type_simple(TYPE_INT);
}

SkardType copy_skard_type(SkardType *source)
{
    SkardType destination;
    destination.type = source->type;
    return destination;
}


bool is_skard_type_of_kind(SkardType *skard_type, TypeKind type)
{
    return skard_type->type == type;
}

bool is_skard_type_unknown(SkardType *skard_type)
{
    return skard_type->type == TYPE_UNKNOWN;
}

bool is_skard_type_invalid(SkardType *skard_type)
{
    return skard_type->type == TYPE_INVALID;
}


void skard_type_print(SkardType *skard_type)
{
    switch (skard_type->type) {
        case TYPE_UNKNOWN:
        case TYPE_REAL:
        case TYPE_INT:
        case TYPE_INVALID:
            printf("%s", skard_type_translate(skard_type));
    }
}


const char *skard_type_translate(SkardType *skard_type)
{
    switch (skard_type->type) {
        case TYPE_UNKNOWN:
            return "*Unknown";
        case TYPE_INVALID:
            return "*Invalid";
        case TYPE_REAL:
            return "Real";
        case TYPE_INT:
            return "Int";
    }

    assert(false);
    return NULL;
}


Value make_value_real(SkReal real)
{
    return (Value) { .type = TYPE_REAL, .as.sk_real = real };
}


void print_value(Value value)
{
    switch (value.type) {
        case TYPE_REAL:
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
