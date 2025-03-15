#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

typedef enum {
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_RETURN, // so this is an uint8_t type
} OpCode;

// all the line NO. info in the source code
typedef struct {
    int count;
    int capacity;
    // stored in (times_of_occurrence, line_number)
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
    // program binary code
    uint8_t* code; // uint8_t[]
    // line info
    RLE_LineEncoding line_encodings;
    // like .rodata section, preserves instruction constant value
    // todo: add example here
    // @type {Value[]}
    ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
int addConstant(Chunk* chunk, Value value);
int chunkGetLine(Chunk* chunk, int index);

#endif