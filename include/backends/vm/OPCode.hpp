#pragma once 
// TODO: Unimplemented OPs
// ADD_STRING
enum class OPCode {
    // Type-specific arithmetic
    ADD_INT, ADD_FLOAT, ADD_DOUBLE, ADD_STRING,
    SUB_INT, SUB_FLOAT, SUB_DOUBLE,
    MULT_INT, MULT_FLOAT, MULT_DOUBLE,
    DIV_INT, DIV_FLOAT, DIV_DOUBLE,
    NEG_INT, NEG_FLOAT, NEG_DOUBLE,
    BITWISE_AND_INT, BITWISE_OR_INT, BITWISE_XOR_INT, 
    BITWISE_AND_BOOL, BITWISE_OR_BOOL, BITWISE_XOR_BOOL,
    

    NOT_BOOL,

    // Type-specific loads
    LOAD_INT, LOAD_FLOAT, LOAD_DOUBLE, LOAD_STRING, LOAD_BOOL,
    STORE_LOCAL, LOAD_LOCAL,  STORE_GLOBAL, LOAD_GLOBAL,

    // Type conversions (explicit)
    INT_TO_DOUBLE, FLOAT_TO_DOUBLE,

    // Comparisons (return bool)
    EQ_INT, EQ_DOUBLE, EQ_FLOAT, EQ_BOOL, EQ_STRING,
    LT_INT, LT_DOUBLE, LT_FLOAT,
    GT_INT, GT_DOUBLE, GT_FLOAT,
    GEQ_INT, GEQ_DOUBLE, GEQ_FLOAT, 
    LEQ_INT, LEQ_DOUBLE, LEQ_FLOAT,

    // Control flow
    JUMP_IF_FALSE, JUMP,

    // Function calls
    CALL, RETURN,

    // Array operations
    ARRAY_NEW,          // Create new empty array
    ARRAY_GET,          // Get element at index: arr[i]
    ARRAY_SET,          // Set element at index: arr[i] = value
    ARRAY_LENGTH,       // Get array length: arr.length
    ARRAY_PUSH,         // Push element to end of array: arr.push(value)
    ARRAY_POP,          // Pop element from end of array: arr.pop()

    HALT
};

const char* OpCodeToString(OPCode op);