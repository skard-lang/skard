#include "value.h"

#include <stdio.h>
#include <assert.h>

#include "utils.h"


SkardType make_skard_type_simple(TypeKind type)
{
    SkardType skard_type;
    skard_type.kind = type;
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
    destination.kind = source->kind;
    return destination;
}


bool is_skard_type_of_kind(SkardType *skard_type, TypeKind type)
{
    return skard_type->kind == type;
}

bool is_skard_type_unknown(SkardType *skard_type)
{
    return skard_type->kind == TYPE_UNKNOWN;
}

bool is_skard_type_invalid(SkardType *skard_type)
{
    return skard_type->kind == TYPE_INVALID;
}


void skard_type_print(SkardType *skard_type)
{
    assert((COUNT_TYPES == 4) && "Exhaustive types handling");
    switch (skard_type->kind) {
        case TYPE_UNKNOWN:
        case TYPE_REAL:
        case TYPE_INT:
        case TYPE_INVALID:
            printf("%s", skard_type_translate(skard_type));
        default:
            break;
    }
}


const char *skard_type_translate(SkardType *skard_type)
{
    assert((COUNT_TYPES == 4) && "Exhaustive types handling");
    switch (skard_type->kind) {
        case TYPE_UNKNOWN:
            return "*Unknown";
        case TYPE_INVALID:
            return "*Invalid";
        case TYPE_REAL:
            return "Real";
        case TYPE_INT:
            return "Int";
        default:
            break;
    }

    return NULL;
}


Value make_value_real(SkReal real)
{
    return (Value) { .type = TYPE_REAL, .as.sk_real = real };
}


void print_value(Value value)
{
    assert((COUNT_TYPES == 4) && "Exhaustive types handling");
    switch (value.type) {
        case TYPE_REAL:
            printf("%lf", value.as.sk_real);
            break;
        default:
            printf("UNKNOWN TYPE");
            break;
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
