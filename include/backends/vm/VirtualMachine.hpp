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
        std::stack<CallFrame> callStack;

        void pushValue(Value value);
        Value peekValue();

        void runtimeError(const std::string& message);

    public:
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
};