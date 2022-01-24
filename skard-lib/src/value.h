#ifndef SKARD_VALUE_H
#define SKARD_VALUE_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

typedef double SkReal;
typedef int64_t SkInt;

typedef enum {
    TYPE_UNKNOWN,
    TYPE_INVALID,
    TYPE_REAL,
    TYPE_INT,
    COUNT_TYPES,
} TypeKind;

typedef struct {
    TypeKind kind;
} SkardType;

SkardType make_skard_type_simple(TypeKind type);
SkardType make_skard_type_unknown(void);
SkardType make_skard_type_invalid();
SkardType make_skard_type_real(void);
SkardType make_skard_type_int(void);
SkardType copy_skard_type(SkardType *source);

bool is_skard_type_of_kind(SkardType *skard_type, TypeKind type);
bool is_skard_type_unknown(SkardType *skard_type);
bool is_skard_type_invalid(SkardType *skard_type);

void skard_type_print(SkardType *skard_type);

const char *skard_type_translate(SkardType *skard_type);

typedef struct {
    TypeKind type;
    union {
        SkReal sk_real;
        SkInt sk_int;
    } as;
} Value;

Value make_value_real(SkReal sk_real);
Value make_value_int(SkInt sk_int);

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
