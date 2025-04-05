#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "table.h"
#include "value.h"

#define STACK_MAX 256

typedef struct {
    Chunk* chunk;           // program instructions
    uint8_t* ip;            // program instruction pointer
    Value stack[STACK_MAX]; // static allocated Value stack
    Value* stackTop;        // Value stack pointer
    Table globals;
    Table strings;
    Obj* objects;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;

// todo: why use extern here?
// object.c depends on this `vm` variable which is declared in vm.c
// basically export this `vm` variable declared in the c file
extern VM vm;

void initVM();
void freeVM();
InterpretResult interpret(const char* source);
void push(Value value);
Value pop();

#endif