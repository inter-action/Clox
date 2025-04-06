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
typedef void (*ParseFn)(bool canAssign);

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

static bool check(TokenType type) {
    return parser.current.type == type;
}

static bool match(TokenType type) {
    if (!check(type))
        return false;
    advance();
    return true;
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
static void statement();
static void declaration();
static void parsePrecedence(Precedence precedence);
static ParseRule* getRule(TokenType tokenType);

static uint8_t identifierConstant(Token* name) {
    return makeConstant(OBJ_VAL(copyString(name->start, name->length)));
}

static uint8_t parseVariable(const char* errorMessage) {
    consume(TOKEN_IDENTIFIER, errorMessage);
    return identifierConstant(&parser.previous);
}

static void defineVariable(uint8_t global) {
    emitBytes(OP_DEFINE_GLOBAL, global);
}

static void binary(bool canAssign) {
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

static void literal(bool canAssign) {
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

static void varDeclaration() {
    uint8_t global = parseVariable("Expect variable name.");

    if (match(TOKEN_EQUAL)) {
        expression();
    } else {
        // `var a;` will turns into `var a = nil;`
        emitByte(OP_NIL);
    }

    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

    defineVariable(global);
}

static void expressionStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");
    // todo: why discard result here?
    emitByte(OP_POP);
}

static void printStatement() {
    expression();
    consume(TOKEN_SEMICOLON, "Expect ';' after value.");
    emitByte(OP_PRINT);
}

static void synchronize() {
    parser.panicMode = false;

    while (parser.current.type != TOKEN_EOF) {
        // these are all points where parser can try recovery from error
        if (parser.previous.type == TOKEN_SEMICOLON)
            return;

        switch (parser.current.type) {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;

            default:; // do nothing. skipping...
        }

        advance();
    }
}

static void declaration() {
    if (match(TOKEN_VAR)) {
        varDeclaration();
    } else {
        statement();
    }

    if (parser.panicMode)
        synchronize();
}

static void statement() {
    if (match(TOKEN_PRINT)) {
        printStatement();
    } else {
        expressionStatement();
    }
}

static void grouping(bool canAssign) {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void number(bool canAssign) {
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

static void string(bool canAssign) {
    // no beginning & ending quote ", or ending \0
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1, parser.previous.length - 2)));
}

static void namedVariable(Token name, bool canAssign) {
    uint8_t arg = identifierConstant(&name);

    if (canAssign && match(TOKEN_EQUAL)) {
        // var a = 3;
        //         ^
        // now match expression, this case 3
        expression();
        emitBytes(OP_SET_GLOBAL, arg);
    } else {
        emitBytes(OP_GET_GLOBAL, arg);
    }
}

static void variable(bool canAssign) {
    namedVariable(parser.previous, canAssign);
}

static void unary(bool canAssign) {
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
    [TOKEN_IDENTIFIER]       = {variable, NULL, PREC_NONE},
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

    bool canAssign = precedence <= PREC_ASSIGNMENT;
    prefixRule(canAssign);

    // - if we are at 10 + b * c
    //                         ^ , variable c has lower precedence
    //                         than any other rule, so it'll just
    //                         stop at while loop
    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();
        //  - 10 + b * c
        //         ^
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        //  - 10 + b * c
        //       ^ binary
        infixRule(canAssign); // no need, but c type system require this
    }

    // say we're parsing
    // a*b=c;
    // ^ canAssign == true, we're in frame A
    // a*b=c;
    //   ^ canAssign == false, compilation result [a,b,*], we're in frame B
    // a*b=c;
    //    ^ step out of binary function (frame B), we're in frame A,
    //    so canAssign == true, and we also exit while loop due to
    //    `=` sign has precedence of PREC_NONE
    if (canAssign && match(TOKEN_EQUAL)) {
        error("Invalid assigment target.");
    }
}

static ParseRule* getRule(TokenType type) {
    return &rules[type];
}

// for example,
// when paring a + b * c, it'll generate instruction roughly
// like  :[ a b c * +]
// when paring a * b + c, the output would then be
// output:[ a b * c +]
bool compile(const char* source, Chunk* chunk) {
    initScanner(source);
    compilingChunk = chunk;

    parser.hadError = false;
    parser.panicMode = false;

    //  - 10 + b * c
    advance();
    //  - 10 + b * c
    //     ^

    while (!match(TOKEN_EOF)) {
        declaration();
    }

    endCompiler();
    return !parser.hadError;
}