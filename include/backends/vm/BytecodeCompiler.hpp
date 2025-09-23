#pragma once
#include "Instruction.hpp"
#include "Value.hpp"
#include <vector>
#include <unordered_map>
#include "../../parser/Expression.hpp"
#include "../../parser/Statement.hpp"
#include "../../lexer/Token.hpp"
#include "RuntimeException.hpp"

class VirtualMachine;


class BytecodeCompiler {
    public:
        struct CompiledProgram {
            std::vector<Instruction> instructions;
            std::vector<Value> constants;
        };
        struct VariableInfo {
            int slot;
            ValueType type;
        };

        struct CompiledFunction {
            std::vector<Instruction> instructions;
            std::vector<Value> constants;
            int parameterCount;
            ValueType returnType; 
            std::string functionName;    
        };

        explicit BytecodeCompiler(VirtualMachine& vm);
        CompiledProgram compile(std::unique_ptr<Statement> ast);
        ValueType compileExpression(const Expression& node);
        ValueType compileLiteral(const LiteralExpression& node);
        ValueType compileBinary(const BinaryExpression& node);
        ValueType compileUnary(const UnaryExpression& node);
        ValueType compileIdentifier(const IdentifierExpression& node);
        ValueType compileFunctionCall(const FunctionCall& node);

        void compileVariableDeclaration(const VariableDeclaration& node);
        void compileAssignment(const Assignment& node);
        void compileIfStatement(const IfStatement& node);
        void compileWhileStatement(const WhileStatement& node);
        void compileFunctionDefinition(const FunctionDefinition &node);
        void compileReturnStatement(const ReturnStatement& node);
        void compileBlockStatement(const BlockStatement& node);
        void compileProgram(const Program& node);

    private:
        VirtualMachine& vm;
        std::vector<Instruction> instructions;
        std::vector<Value> constants;
        ValueType currentExpressionType;
        std::vector<std::unordered_map<std::string, VariableInfo>> scopeStack;
        std::unordered_map<std::string, int> functionTable;
        int nextSlot = 0;
        int globalSlot = 0;

        Value inferType(const Token& token);
        int addConstant(Value value);
        void emit(OPCode opcode, int operand = 0);
        OPCode getOpCode(TokenType op, ValueType type, bool isUnary = false);

        void enterScope();
        void exitScope();
        VariableInfo* lookupVariable(const std::string& name);
        void declareVariable(const std::string& name, const VariableInfo& info);
};