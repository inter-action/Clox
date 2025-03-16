#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)

#define IS_STRING(value) isObjType(value, OBJ_STRING)

// @type {ObjString*}
#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
// @type {char*}
#define AS_CSTRING(value) (((ObjString*)AS_OBJ(value))->chars)

typedef enum {
    OBJ_STRING,
} ObjType;

// todo: why not using typedef here?
// c: https://www.delftstack.com/howto/c/struct-and-typedef-struct-in-c/
//    https://stackoverflow.com/questions/1675351/typedef-struct-vs-struct-definitions
// C specifies that struct fields are arranged in memory in the order that they are declared.
struct Obj {
    ObjType type;
    struct Obj* next;
};

struct ObjString {
    // > This is designed to enable a clever pattern:
    // > You can take a pointer to a struct and safely
    // > convert it to a pointer to its first field and back.
    Obj obj;
    //  ^ , in c, due to ObjString's layout is superset of Obj
    //  that you can safely convert it to Obj or ObjString
    int length;
    char* chars;
};

ObjString* takeString(char* chars, int length);
ObjString* copyString(const char* chars, int length);
void printObject(Value value);

static inline bool isObjType(Value value, ObjType type) {
    // c:          ^ can't put this inside a macro defination,
    // cause `value` would be evaluate twice
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
    //            ^ evaluate       ^ evaluate
}

#endif