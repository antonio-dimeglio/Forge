#include "../../../include/backends/vm/Object.hpp"

bool isString(Value val) {
    return val.type == ValueType::OBJECT && val.as.object->type == ObjectType::STRING;
}

bool isFunction(Value val) {
    return val.type == ValueType::OBJECT && val.as.object->type == ObjectType::FUNCTION;
}

StringObject* asString(Value val) {
    return static_cast<StringObject*>(val.as.object);
}

FunctionObject* asFunction(Value val) {
    return static_cast<FunctionObject*>(val.as.object);
}

std::string objectToString(Object* obj) {
    switch (obj->type) {
        case STRING: {
            StringObject* str = static_cast<StringObject*>(obj);
            return std::string(str->chars, str->length);
        }
        case FUNCTION: {
            FunctionObject* func = static_cast<FunctionObject*>(obj);
            return "<function " + func->name + ">";
        }
        default:
            throw RuntimeException("Missing toString for this type.");
    }
}