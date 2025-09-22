#include "../../../include/backends/vm/Heap.hpp"
#include <cstring>

Heap::Heap() {
    
}

Heap::~Heap() {
    Object* current = objects;
    while (current != nullptr) {
        Object* next = current->next;

        if (current->type == STRING) {
            StringObject* str = static_cast<StringObject*>(current);
            delete[] str->chars;
        } 
        // Functions have types that manage themselves

        delete current;
        current = next;
    }
}

template<typename T>
T* Heap::allocateObject(ObjectType type) {
    T* object = new T();

    object->type = type;
    object->isMarked = false;

    object->next = objects;
    objects = object;

    bytesAllocated += sizeof(T);

    if (bytesAllocated > nextGC) {
        collect();
        nextGC = bytesAllocated * 2; 
    }

    return object;
}

StringObject* Heap::allocateString(const std::string& str) {
    StringObject* obj = allocateObject<StringObject>(STRING);
    obj->length = str.length();
    obj->chars = new char[str.length() + 1];
    std::strcpy(obj->chars, str.c_str());
    return obj;
}

FunctionObject* Heap::allocateFunction(const std::string& name, int paramCount) {
    FunctionObject* obj = allocateObject<FunctionObject>(FUNCTION);
    obj->name = name;
    obj->parameterCount = paramCount;
    return obj;
}

void Heap::collect() {
    // TODO: Implement mark-and-sweep garbage collection
    // For now, just a stub
}

void Heap::markObject(Object* obj) {
    if (obj == nullptr || obj->isMarked) return;

    obj->isMarked = true;

    if (obj->type == FUNCTION) {
        FunctionObject* func = static_cast<FunctionObject*>(obj);
        for (const auto& constant : func->constants) {
            if (isObject(constant)) {
                markObject(asObject(constant));
            }
        }
    }
}

void Heap::sweep() {
    Object** current = &objects;
    while (*current != nullptr) {
        if (!(*current)->isMarked) {
            Object* unreached = *current;
            *current = unreached->next;

            if (unreached->type == STRING) {
                StringObject* str = static_cast<StringObject*>(unreached);
                delete[] str->chars;
            }

            bytesAllocated -= sizeof(*unreached);
            delete unreached;
        } else {
            (*current)->isMarked = false;
            current = &(*current)->next;
        }
    }
}

template StringObject* Heap::allocateObject<StringObject>(ObjectType type);
template FunctionObject* Heap::allocateObject<FunctionObject>(ObjectType type);