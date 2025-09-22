#include "../../../include/backends/vm/Value.hpp"

Value createInt(int value) {
    Value val;
    val.type = ValueType::INT;
    val.as.integerVal = value;
    return val;
}

Value createFloat(float value) {
    Value val;
    val.type = ValueType::FLOAT;
    val.as.floatVal = value;
    return val;
}

Value createDouble(double value) {
    Value val;
    val.type = ValueType::DOUBLE;
    val.as.doubleVal = value;
    return val;
}

Value createBool(bool value) {
    Value val;
    val.type = ValueType::BOOL;
    val.as.boolVal = value;
    return val;
}

Value createObject(Object* value) {
    Value val;
    val.type = ValueType::OBJECT;
    val.as.object = value;
    return val;
}

bool isInt(Value val) { return val.type == ValueType::INT; }
bool isFloat(Value val) { return val.type == ValueType::FLOAT; }
bool isDouble(Value val) { return val.type == ValueType::DOUBLE; }
bool isBool(Value val) { return val.type == ValueType::BOOL; }
bool isObject(Value val) { return val.type == ValueType::OBJECT; }

int asInt(Value val) {
    return val.as.integerVal;
}

float asFloat(Value val) {
    return val.as.floatVal;
}

double asDouble(Value val) {
    return val.as.doubleVal;
}

bool asBool(Value val) {
    return val.as.boolVal;
}

Object* asObject(Value val) {
    return val.as.object;
}

std::string valueToString(Value val) {
    switch (val.type) {
        case INT:
            return std::to_string(val.as.integerVal);
        case FLOAT:
            return std::to_string(val.as.floatVal);
        case DOUBLE:
            return std::to_string(val.as.doubleVal);
        case BOOL:
            return val.as.boolVal ? "true" : "false";
        case OBJECT:
            return objectToString(val.as.object);
        default:
            return "unknown";
    }
}

bool valuesEqual(Value a, Value b) {
    if (a.type != b.type) { return false; }

    switch (a.type) {
        case INT:
            return a.as.integerVal == b.as.integerVal;
        case FLOAT:
            return a.as.floatVal == b.as.floatVal;
        case DOUBLE:
            return a.as.doubleVal == b.as.doubleVal;
        case BOOL:
            return a.as.boolVal == b.as.boolVal;
        case OBJECT:
            if (a.as.object == b.as.object) return true; 
            // TODO FIX
            return false;
        default:
            throw RuntimeException("Cannot evaluate equality.");
    }
}