#pragma once 
#include <cstddef>
#include <vector>
#include <string>
#include "Instruction.hpp"
#include "Value.hpp"

enum ObjectType {
    STRING,
    FUNCTION
};

struct Object {
    ObjectType type;
    bool isMarked;
    Object* next;
};

struct StringObject : Object {
    size_t length;
    char* chars;
};

struct FunctionObject : Object {
    std::string name;
    int parameterCount; 
    std::vector<Instruction> instructions;
    std::vector<Value> constants;
};

bool isString(Value val);
bool isFunction(Value val);

StringObject* asString(Value val);
FunctionObject* asFunction(Value val);

std::string objectToString(Object* obj);