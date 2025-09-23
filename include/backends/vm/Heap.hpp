#pragma once 
#include "Object.hpp"
#include <vector>
#include <stack>

class VirtualMachine;
class CallFrame;

class Heap {
    private:
        Object* objects = nullptr;
        size_t bytesAllocated = 0;
        size_t nextGC = 1024;
        VirtualMachine* vm = nullptr;
    public:
        Heap();  
        ~Heap(); 

        void setVM(VirtualMachine* virtualMachine) { vm = virtualMachine; }

        StringObject* allocateString(const std::string& str);
        FunctionObject* allocateFunction(const std::string& name, int paramCount);

        void collect();
        void markVMRoots();
        void markValueArray(const std::vector<Value>& values);
        void markObject(Object* obj);
        void markCallStack(const std::stack<CallFrame>& callStack);
        

        template<typename T>
        T* allocateObject(ObjectType type);
        void sweep();
};