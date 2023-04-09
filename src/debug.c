#include <stdio.h>
#include "chunk.h"
#include "debug.h"


void disassembleChunk(Chunk* chunk, char* name){
    printf("== %s ==\n", name);

    for (int offset = 0; offset < chunk->count; ) {
        int next = disassembleInstruction(chunk, offset);
        offset = next;
    }
}

static int simpleInstruction(char* name, int offset){
    printf("%s\n", name);
    
    return offset + 1;
}

int disassembleInstruction(Chunk* chunk, int offset) {
    printf("%04d ", offset);

    uint8_t opcode = chunk->code[offset];

    switch (opcode) {
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        default:
            printf("Unknow opcode %d\n", opcode);
            // todo: why not panic here?
            return opcode + 1;
    }
    

    return 0;
}

