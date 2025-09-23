#pragma once
#include "OPCode.hpp"
#include "Instruction.hpp"
#include "Value.hpp"
#include "Heap.hpp"
#include <vector>
#include <memory>
#include <unordered_map>
#include <stack>
#include "../../parser/Expression.hpp"

struct CallFrame {
    std::vector<Instruction> instructions;
    std::vector<Value> constants;
    std::vector<Value> locals;
    size_t returnAddress;
    size_t stackBase;  // Where this frame's stack starts
};

class VirtualMachine {
    private:
        std::vector<Value> stack;
        std::vector<Value> constants;
        std::vector<Instruction> instructions;
        Heap heap;
        size_t ip = 0;
        std::vector<Value> locals;
        std::vector<Value> globals;
        std::stack<CallFrame> callStack;

        void pushValue(Value value);
        Value peekValue();

        void runtimeError(const std::string& message);

    public:
        VirtualMachine();  // Constructor
        Value popValue();
        void loadProgram(const std::vector<Instruction>& instructions,
                const std::vector<Value>& constants);
        void run();
        void dumpStack();
        void printInstructions();

        Value getLocal(size_t idx);
        void reset();

        // Heap access for object allocation
        Heap& getHeap() { return heap; }

        // GC root access for heap
        const std::vector<Value>& getStack() const { return stack; }
        const std::vector<Value>& getGlobals() const { return globals; }
        const std::vector<Value>& getConstants() const { return constants; }
        const std::vector<Value>& getLocals() const { return locals; }
        const std::stack<CallFrame>& getCallStack() const { return callStack; }
};