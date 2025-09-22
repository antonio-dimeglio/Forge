#pragma once 
#include "Object.hpp"

class Heap {
    private:
        Object* objects = nullptr;
        size_t bytesAllocated = 0;
        size_t nextGC = 1024;

    public:
        Heap();  
        ~Heap(); 

        StringObject* allocateString(const std::string& str);
        FunctionObject* allocateFunction(const std::string& name, int paramCount);

        void collect();
        void markObject(Object* obj);

        template<typename T>
        T* allocateObject(ObjectType type);
        void sweep();
};