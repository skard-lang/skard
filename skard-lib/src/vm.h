#ifndef SKARD_VM_H
#define SKARD_VM_H

#include "chunk.h"

#define SKARD_VM_STACK_MIN_SIZE 256

typedef struct {
    size_t capacity;
    Value *stack;
    Value *stack_top;
} VMStack;

void vm_stack_init(VMStack *stack);
void vm_stack_free(VMStack *stack);

void vm_stack_push(VMStack *stack, Value value);
Value vm_stack_pop(VMStack *stack);

typedef enum {
    INTERPRETER_OK,
    INTERPRETER_NOK_RUNTIME,
} InterpreterResult;

typedef struct {
    Chunk *chunk;
    uint8_t *ip;
    VMStack stack;
} SkardVM;

void vm_init(SkardVM *vm);
void vm_free(SkardVM *vm);

InterpreterResult vm_run(SkardVM *vm, Chunk *chunk);

#endif //SKARD_VM_H
