#include "vm.h"

#include <stdio.h>
#include <stdbool.h>

#include "utils.h"
#include "debug.h"

void vm_stack_init(VMStack *stack)
{
    stack->capacity = 0;
    stack->stack = NULL;
    stack->stack_top = NULL;
}

void vm_stack_free(VMStack *stack)
{
    SKARD_FREE_ARRAY(Value, stack->stack);
    vm_stack_init(stack);
}

void vm_stack_push(VMStack *stack, Value value)
{
    size_t offset = stack->stack == stack->stack_top ? 0 : stack->stack_top - stack->stack;
    if (offset == stack->capacity) {
        stack->capacity = stack->capacity < SKARD_VM_STACK_MIN_SIZE ? SKARD_VM_STACK_MIN_SIZE : 2 * stack->capacity;
        stack->stack = SKARD_GROW_ARRAY(Value, stack->stack, stack->capacity);
        stack->stack_top = stack->stack + offset;
    }

    *stack->stack_top = value;
    stack->stack_top++;
}

Value vm_stack_pop(VMStack *stack)
{
    stack->stack_top--;
    return *stack->stack_top;
}


void vm_init(SkardVM *vm)
{
    vm_stack_init(&vm->stack);
}

void vm_free(SkardVM *vm)
{
    vm_stack_free(&vm->stack);
}

static void vm_debug_print_stack(SkardVM *vm)
{
    printf("{ ");
    size_t offset = vm->stack.stack == vm->stack.stack_top ? 0 : vm->stack.stack_top - vm->stack.stack;
    for (size_t i = 0; i < offset; i++) {
        printf("[ ");
        print_value(vm->stack.stack[i]);
        printf(" ]");
    }
    printf(" }");
}

static void vm_debug_trace(SkardVM *vm)
{
    vm_debug_print_stack(vm);
    printf("\n");
    disassemble_instruction(vm->chunk, vm->ip - vm->chunk->code);
    printf("\n");
}

static InterpreterResult vm_loop(SkardVM *vm)
{
#define SKARD_READ_BYTE() (*vm->ip++)
#define SKARD_READ_CONSTANT() (vm->chunk->constants.values[SKARD_READ_BYTE()])
#define SKARD_READ_CONSTANT_LONG() \
    (vm->chunk->constants.values[(vm->ip += 3, (vm->ip[-3]) | (vm->ip[-2]) << 8 | (vm->ip[-1]) << 16)])

    while (true) {

#ifdef SKARD_DEBUG_TRACE
        vm_debug_trace(vm);
#endif

        switch (SKARD_READ_BYTE()) {
            case OP_RETURN:
                return INTERPRETER_OK;
            case OP_DUMP:
                print_value(vm_stack_pop(&vm->stack));
                printf("\n");
                break;
            case OP_CONSTANT:
                vm_stack_push(&vm->stack, SKARD_READ_CONSTANT());
                break;
            case OP_CONSTANT_LONG:
                vm_stack_push(&vm->stack, SKARD_READ_CONSTANT_LONG());
                break;
            default:
                return INTERPRETER_NOK_RUNTIME;
        }
    }

#undef SKARD_READ_BYTE
#undef SKARD_READ_CONSTANT
#undef SKARD_READ_CONSTANT_LONG
}

InterpreterResult vm_run(SkardVM *vm, Chunk *chunk)
{
    vm->chunk = chunk;
    vm->ip = chunk->code;

    return vm_loop(vm);
}
