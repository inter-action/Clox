/**
 * as u can see this compiler doesn't compose AST, it directly translate
 * AST node into byte codes. this is the reason that the book call it
 * `one time pass compiler`
 */
#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "object.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct {
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

// the lower the higher of the enum value would be
typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT, // =
    PREC_OR,         // or
    PREC_AND,        // and
    PREC_EQUALITY,   // == !=
    PREC_COMPARISON, // <> <= >=
    PREC_TERM,       // + -
    PREC_FACTOR,     // * /
    PREC_UNARY,      // ! -
    PREC_CALL,       // . ()
    PREC_PRIMARY
} Precedence;

// clang function pointer
typedef void (*ParseFn)();

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

Parser parser;
Chunk* compilingChunk;

static Chunk* currentChunk() {
    return compilingChunk;
}

static void errorAt(Token* token, const char* message) {
    if (parser.panicMode)
        return;
    parser.panicMode = true;
    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, "at end");
    } else if (token->type == TOKEN_ERROR) {
        // Nothing
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

static void errorAtCurrent(const char* message) {
    errorAt(&parser.current, message);
}

static void error(const char* message) {
    errorAt(&parser.previous, message);
}

static void advance() {
    parser.previous = parser.current;

    while (true) {
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR)
            break;

        errorAtCurrent(parser.current.start);
    }
}

static void consume(TokenType type, const char* message) {
    if (parser.current.type == type) {
        advance();
        return;
    }

    errorAtCurrent(message);
}

static void emitByte(uint8_t byte) {
    writeChunk(currentChunk(), byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

static void emitReturn() {
    emitByte(OP_RETURN);
}

static uint8_t makeConstant(Value value) {
    int constant = addConstant(currentChunk(), value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }
    return (uint8_t)constant;
}

// for types that not able to fit in one Byte
static void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void endCompiler() {
    emitReturn();
#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError) {
        disassembleChunk(currentChunk(), "code");
    }
#endif
}

// empty declarations
static void expression();
static void parsePrecedence(Precedence precedence);
static ParseRule* getRule(TokenType tokenType);

static void binary() {
    //  - 10 + b * c
    //       ^ previous
    TokenType operatorType = parser.previous.type;
    ParseRule* rule = getRule(operatorType);
    //  - 10 + b * c
    //       ^ precedence == PREC_NONE, so +1 gives PREC_FACTOR
    parsePrecedence((Precedence)(rule->precedence + 1)); // force left associate for expression

    switch (operatorType) {
        case TOKEN_BANG_EQUAL:
            emitBytes(OP_EQUAL, OP_NOT);
            break;
        case TOKEN_EQUAL_EQUAL:
            emitByte(OP_EQUAL);
            break;
        case TOKEN_GREATER:
            emitByte(OP_GREATER);
            break;
        case TOKEN_GREATER_EQUAL:
            emitBytes(OP_LESS, OP_NOT);
            break;
        case TOKEN_LESS:
            emitByte(OP_LESS);
            break;
        case TOKEN_LESS_EQUAL:
            emitBytes(OP_GREATER, OP_NOT);
            break;
        case TOKEN_PLUS:
            emitByte(OP_ADD);
            break;
        case TOKEN_MINUS:
            emitByte(OP_SUBTRACT);
            break;
        case TOKEN_STAR:
            emitByte(OP_MULTIPLY);
            break;
        case TOKEN_SLASH:
            emitByte(OP_DIVIDE);
            break;
        default:
            return; // Unreachable
    }
}

static void literal() {
    switch (parser.previous.type) {
        case TOKEN_FALSE:
            emitByte(OP_FALSE);
            break;
        case TOKEN_NIL:
            emitByte(OP_NIL);
            break;
        case TOKEN_TRUE:
            emitByte(OP_TRUE);
            break;
        default:
            return; // Unreachable
    }
}

static void expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

static void grouping() {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void number() {
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

static void string() {
    // no beginning & ending quote ", or ending \0
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1, parser.previous.length - 2)));
}

static void unary() {
    TokenType operatorType = parser.previous.type;

    // Compile the operand
    parsePrecedence(PREC_UNARY);

    // Emit the operator instruction
    switch (operatorType) {
        case TOKEN_BANG:
            emitByte(OP_NOT);
            break;
        case TOKEN_MINUS:
            emitByte(OP_NEGATE);
            break;
        default:
            return; // Unreachable
    }
}

// C99's array literal
// c: so this works like a hashmap in some sense
// clang-format off
// :!column -t -s =
// then add = back with vim column mode
ParseRule rules[] = {
    // because value for this enum also starts with zero
    [TOKEN_LEFT_PAREN]       = {grouping, NULL, PREC_NONE},
    //                                    ^ infix rule
    //                          ^ prefix rule
    [TOKEN_RIGHT_PAREN]      = {NULL, NULL, PREC_NONE},
    [TOKEN_LEFT_BRACE]       = {NULL, NULL, PREC_NONE},
    [TOKEN_RIGHT_BRACE]      = {NULL, NULL, PREC_NONE},
    [TOKEN_COMMA]            = {NULL, NULL, PREC_NONE},
    [TOKEN_DOT]              = {NULL, NULL, PREC_NONE},
    [TOKEN_MINUS]            = {unary, binary, PREC_TERM},
    [TOKEN_PLUS]             = {NULL, binary, PREC_TERM},
    [TOKEN_SEMICOLON]        = {NULL, NULL, PREC_NONE},
    [TOKEN_SLASH]            = {NULL, binary, PREC_FACTOR},
    [TOKEN_STAR]             = {NULL, binary, PREC_FACTOR},
    [TOKEN_BANG]             = {unary, NULL, PREC_NONE},
    // !=
    [TOKEN_BANG_EQUAL]       = {NULL, binary, PREC_EQUALITY},
    [TOKEN_EQUAL]            = {NULL, NULL, PREC_NONE},
    [TOKEN_EQUAL_EQUAL]      = {NULL, binary, PREC_EQUALITY},
    [TOKEN_GREATER]          = {NULL, binary, PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL]    = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS]             = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS_EQUAL]       = {NULL, binary, PREC_COMPARISON},
    [TOKEN_IDENTIFIER]       = {NULL, NULL, PREC_NONE},
    [TOKEN_STRING]           = {string, NULL, PREC_NONE},
    [TOKEN_NUMBER]           = {number, NULL, PREC_NONE},
    [TOKEN_AND]              = {NULL, NULL, PREC_NONE},
    [TOKEN_CLASS]            = {NULL, NULL, PREC_NONE},
    [TOKEN_ELSE]             = {NULL, NULL, PREC_NONE},
    [TOKEN_FALSE]            = {literal, NULL, PREC_NONE},
    [TOKEN_FOR]              = {NULL, NULL, PREC_NONE},
    [TOKEN_FUN]              = {NULL, NULL, PREC_NONE},
    [TOKEN_IF]               = {NULL, NULL, PREC_NONE},
    [TOKEN_NIL]              = {literal, NULL, PREC_NONE},
    [TOKEN_OR]               = {NULL, NULL, PREC_NONE},
    [TOKEN_PRINT]            = {NULL, NULL, PREC_NONE},
    [TOKEN_RETURN]           = {NULL, NULL, PREC_NONE},
    [TOKEN_SUPER]            = {NULL, NULL, PREC_NONE},
    [TOKEN_THIS]             = {NULL, NULL, PREC_NONE},
    [TOKEN_TRUE]             = {literal, NULL, PREC_NONE},
    [TOKEN_VAR]              = {NULL, NULL, PREC_NONE},
    [TOKEN_WHILE]            = {NULL, NULL, PREC_NONE},
    [TOKEN_ERROR]            = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF]              = {NULL, NULL, PREC_NONE},
};
// clang-format on

// parse to operator with precedence higher than <precedence>
// @example
//  - 10 + b * c
//  - after parsing, the code instruction would look like this:
//  - [(load 10), (load b), (load c), (op *), (op +)]
//  - when vm execute:
//  - mem stack [10, b, c]
//  - current op: *
//  - mem stack [10], poped [b, c]
//  - mem stack [10, (b*c)]
//  - current op: +
//  - mem stack [], poped [10, (b*c)]
//  - mem stack [10 + b*c]
static void parsePrecedence(Precedence precedence) {
    advance();
    //  - 10 + b * c
    //  -    ^ match number infix rule
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == NULL) {
        error("Expect expression");
        return;
    }

    prefixRule();

    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();
        //  - 10 + b * c
        //         ^
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        //  - 10 + b * c
        //       ^ binary
        infixRule();
    }
}

static ParseRule* getRule(TokenType type) {
    return &rules[type];
}

bool compile(const char* source, Chunk* chunk) {
    initScanner(source);
    compilingChunk = chunk;

    parser.hadError = false;
    parser.panicMode = false;

    //  - 10 + b * c
    advance();
    //  - 10 + b * c
    //     ^
    expression();
    consume(TOKEN_EOF, "Expected end of expression.");

    endCompiler();
    return !parser.hadError;
}