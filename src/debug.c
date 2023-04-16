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

// m: so static function don't need to be added in header files
static int constantInstruction(char* name, Chunk* chunk, int offset){
    uint8_t index = chunk->code[offset + 1];
    Value value = chunk->constants.values[index];

    printf("%-16s %4d '", name, index);
    printValue(value);
    printf("'\n");
    
    return offset + 2;
}

static int simpleInstruction(char* name, int offset){
    printf("%s\n", name);
    
    return offset + 1;
}

int disassembleInstruction(Chunk* chunk, int offset) {
    printf("%04d ", offset);

    if (offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
        printf("   | ");
    } else {
        printf("%4d ", chunk->lines[offset]);
    }

    uint8_t opcode = chunk->code[offset];

    switch (opcode) {
        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", chunk, offset);
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        default:
            printf("Unknow opcode %d\n", opcode);
            // todo: why not panic here?
            return opcode + 1;
    }
    

    return 0;
}
