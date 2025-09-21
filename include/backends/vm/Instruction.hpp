#pragma once 
#include "OPCode.hpp"
#include <string>
#include <variant>

struct TypedValue {
    enum Type { INT, FLOAT, DOUBLE, STRING, BOOL } type;
    union {
        int i;
        float f;
        double d;
        size_t string_id;
        bool b;
    } value;
};

inline const char* TypeToString(TypedValue::Type t) {
    switch (t) {
        case TypedValue::Type::INT: return "INT";
        case TypedValue::Type::FLOAT: return "FLOAT";
        case TypedValue::Type::DOUBLE: return "DOUBLE";
        case TypedValue::Type::STRING: return "STRING";
        case TypedValue::Type::BOOL: return "BOOL";
    };
    return "UNKNOWN";
}

struct Instruction {
    OPCode opcode; 
    int operand;
};