#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "value.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objectType) (type*)allocateObject(sizeof(type), objectType)

static Obj* allocateObject(int size, ObjType type) {
    Obj* object = (Obj*)reallocate(NULL, 0, size);
    //                                   ^ the size would be greater than Obj, so it's ok
    object->type = type;
    object->next = vm.objects;
    //             ^ need `extern vm` here
    vm.objects = object;
    return object;
}

static ObjString* allocateString(char* chars, int length) {
    ObjString* string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    //                               ^ as u can see here, Obj* can be automatically converted to ObjString. this is
    //                               polymorphism done in c
    string->length = length;
    string->chars = chars;
    return string;
}

// convert c string to ObjString
ObjString* takeString(char* chars, int length) {
    return allocateString(chars, length);
}

// convert c string to ObjString, create a new copy
ObjString* copyString(const char* chars, int length) {
    char* heapChars = ALLOCATE(char, length + 1);
    memcpy(heapChars, chars, length);
    heapChars[length] = '\0';
    return allocateString(heapChars, length);
}

void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
    }
}