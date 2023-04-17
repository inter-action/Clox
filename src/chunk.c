#include <stdio.h>
#include <stdlib.h>
#include "chunk.h"
#include "memory.h"
#include "value.h"


void initChunk(Chunk* chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    initValueArray(&chunk->constants);

    RLE_LineEncoding line_encodings;
    chunk->line_encodings = line_encodings;
    initEncoding(&chunk->line_encodings);
}

void freeChunk(Chunk* chunk) {
    // why do use FREE_ARRAY here when we can simply call free?
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    freeValueArray(&chunk->constants);
    freeEncoding(&chunk->line_encodings);
    initChunk(chunk);
}

void writeChunk(Chunk* chunk, uint8_t byte, int line){
    if (chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->count++;
    writeLine(&chunk->line_encodings, line);
}


void* reallocate(void* pointer, size_t oldSize, size_t newSize){
    if (newSize == 0) {
        free(pointer);
        return NULL;
    }

    void* result = realloc(pointer, newSize);
    if (result == NULL) exit(1);
    return result;
}

int addConstant(Chunk* chunk, Value value) {
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count - 1;
}

// return -1 if it holds no valid encodings
int chunkGetLine(Chunk* chunk, int index) {
    return getEncodingLine(&chunk->line_encodings, index);
}

// start RLE


void initEncoding(RLE_LineEncoding* encoding) {
    encoding->count = 0;
    encoding->capacity = 0;
    encoding->encodings = NULL;

}
void freeEncoding(RLE_LineEncoding* encoding) {
    FREE_ARRAY(int, encoding->encodings, encoding->capacity);
    initEncoding(encoding);
}

void writeLine(RLE_LineEncoding* encoding, int line) {
    if (encoding->encodings != NULL && line == encoding->encodings[encoding->count - 1]) {
        encoding->encodings[encoding->count-2]++;
    } else {
        if (encoding->capacity < encoding->count + 1) {
            int oldCapacity = encoding->capacity;
            encoding->capacity = GROW_CAPACITY(oldCapacity);
            encoding->encodings = GROW_ARRAY(int, encoding->encodings, oldCapacity, encoding->capacity);
        }

        encoding->encodings[encoding->count] = 1;
        encoding->encodings[encoding->count + 1] = line;
        encoding->count += 2;
    }
}

int getEncodingLine(RLE_LineEncoding* encoding, int index) {
    if (encoding->count == 0) return -1;

    int cursor = 0;
    int counter = 0;
    while (true) {
        counter += encoding->encodings[cursor];
        int line = encoding->encodings[cursor + 1];
        cursor += 2;

        if((index + 1) <= counter || cursor >= encoding->count) {
            break;
        }
    }

    return encoding->encodings[cursor - 1];
}

