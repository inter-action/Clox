#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"

typedef enum {
    OP_RETURN, // so this is an uint8_t type
} OpCode;


// code instructions in binary format, 
typedef struct {
    int count;
    int capacity;
    uint8_t* code; // uint8_t[]
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte);

#endif