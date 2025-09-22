#pragma once 

enum class OPCode {
    // Type-specific arithmetic
    ADD_INT, ADD_FLOAT, ADD_DOUBLE, ADD_STRING,
    SUB_INT, SUB_FLOAT, SUB_DOUBLE,
    MULT_INT, MULT_FLOAT, MULT_DOUBLE,
    DIV_INT, DIV_FLOAT, DIV_DOUBLE,
    NEG_INT, NEG_FLOAT, NEG_DOUBLE,

    NOT_BOOL,

    // Type-specific loads
    LOAD_INT, LOAD_FLOAT, LOAD_DOUBLE, LOAD_STRING, LOAD_BOOL,
    STORE_LOCAL, LOAD_LOCAL,

    // Type conversions (explicit)
    INT_TO_DOUBLE, FLOAT_TO_DOUBLE,

    // Comparisons (return bool)
    EQ_INT, EQ_DOUBLE, LT_INT, LT_DOUBLE, GT_INT, GT_DOUBLE,

    // Control flow
    JUMP_IF_FALSE, JUMP,

    HALT
};

const char* OpCodeToString(OPCode op);