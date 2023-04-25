#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

typedef enum {
    OP_CONSTANT,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NEGATE,
    OP_RETURN, // so this is an uint8_t type
} OpCode;

typedef struct {
    int count;
    int capacity;
    int* encodings;
} RLE_LineEncoding;

void initEncoding(RLE_LineEncoding* encoding);
void freeEncoding(RLE_LineEncoding* encoding);
void writeLine(RLE_LineEncoding* encoding, int line);
int getEncodingLine(RLE_LineEncoding* encoding, int index);

// code instructions in binary format, 
typedef struct {
    int count;
    int capacity;
    uint8_t* code; // uint8_t[]
    RLE_LineEncoding line_encodings;
    ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
int addConstant(Chunk* chunk, Value value);
int chunkGetLine(Chunk* chunk, int index);

#endif