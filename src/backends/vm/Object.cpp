#include "../../../include/backends/vm/Object.hpp"

bool isString(Value val) {
    return val.type == ValueType::OBJECT && val.as.object->type == ObjectType::STRING;
}

bool isFunction(Value val) {
    return val.type == ValueType::OBJECT && val.as.object->type == ObjectType::FUNCTION;
}

bool isArray(Value val) {
    return val.type == ValueType::OBJECT && val.as.object->type == ObjectType::ARRAY;
}

StringObject* asString(Value val) {
    return static_cast<StringObject*>(val.as.object);
}

FunctionObject* asFunction(Value val) {
    return static_cast<FunctionObject*>(val.as.object);
}

ArrayObject* asArray(Value val) {
    return static_cast<ArrayObject*>(val.as.object);
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
        case ARRAY: {
            ArrayObject* arr = static_cast<ArrayObject*>(obj);
            return "<array length=" + std::to_string(arr->length) + ">";
        }
        default:
            throw RuntimeException("Missing toString for this type.");
    }
}