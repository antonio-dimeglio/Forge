#pragma once 
#include "Instruction.hpp"
#include <vector>
#include <memory>
#include "../../parser/Expression.hpp"

class BytecodeCompiler {
    public:
        struct CompiledProgram {
            std::vector<Instruction> instructions;
            std::vector<Value> constants;
        };

        CompiledProgram compile(std::unique_ptr<Expression> ast);
};