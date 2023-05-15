#include <stdio.h>

#include "chunk.h"
#include "common.h"
#include "debug.h"
#include "vm.h"
#include "compiler.h"


VM vm;


static void resetStack(){
    vm.stackTop = vm.stack;
    // Q: equivlent?
    // A: no, vm.stack is already the pointer to the first element, so &vm.stack would create another redirection, which is wrong
    // vm.stackTop = &vm.stack;
}


void initVM() {
    resetStack();
}

void freeVM() {

}

static InterpretResult run(){
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
// ?: does this `double` break the abstraction for Value type?
// I would think so, the better way is to use `Value` for type instead of double
#define BINDARY_OP(op) \
    do { \
        double b = pop(); \
        double a = pop(); \
        push(a op b); \
    } while (false)

    for(;;) {
#ifdef DEBUG_TRACE_EXECUTION
        // print constants stack
        printf("          ");
        for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");

        // disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code)/sizeof(uint8_t));
        // m:                                                             ^- a divide is wrong
        // basic unit for pointer is byte. so this is actually right, since sizeof(uint8_t) == 1, 
        // the following line just implicitly imply this
        disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif
        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: 
                {
                    Value constant = READ_CONSTANT();
                    push(constant);
                    break;
                }
            case OP_ADD: BINDARY_OP(+);break;
            case OP_SUBTRACT: BINDARY_OP(-);break;
            case OP_MULTIPLY: BINDARY_OP(*);break;
            case OP_DIVIDE: BINDARY_OP(/);break;
            case OP_NEGATE:
                {
                    push(-pop()); 
                    break;
                }
            case OP_RETURN: 
                {
                    printValue(pop());
                    printf("\n");
                    return INTERPRET_OK;
                } 
        }
    }
#undef READ_BYTE
#undef READ_CONSTANT
#undef BINDARY_OP
}

InterpretResult interpret(const char* source) {
    Chunk chunk;
    initChunk(&chunk);

    if (!compile(source, &chunk)) {
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;

    InterpretResult result = run();

    freeChunk(&chunk);

    return result;
}


void push(Value value){
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop(){
    vm.stackTop--;
    // this can underflow, if not track properly
    return *vm.stackTop;
}