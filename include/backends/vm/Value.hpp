#pragma once 
#include <string>
#include "RuntimeException.hpp"

class Object;
std::string objectToString(Object* obj);

enum ValueType {
    INT,
    FLOAT,
    DOUBLE,
    BOOL,
    OBJECT
};

struct Value {
    ValueType type;
    union {
        int integerVal;
        float floatVal;
        double doubleVal;
        bool boolVal;
        Object* object;
    } as;
};

Value createInt(int value);
Value createFloat(float value);
Value createDouble(double value);
Value createBool(bool value);
Value createObject(Object* obj);

bool isInt(Value val);
bool isFloat(Value val);
bool isDouble(Value val);
bool isBool(Value val);
bool isObject(Value val);

int asInt(Value val);
float asFloat(Value val);
double asDouble(Value val);
bool asBool(Value val);
Object* asObject(Value val);

std::string valueToString(Value val);
bool valuesEqual(Value a, Value b);
