#pragma once
#include "Instruction.hpp"
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
            std::vector<TypedValue> constants;
        };
        struct VariableInfo {
            int slot;
            TypedValue::Type type;
        };

        explicit BytecodeCompiler(VirtualMachine& vm);
        CompiledProgram compile(std::unique_ptr<Statement> ast);
        TypedValue::Type compileExpression(const Expression& node);
        TypedValue::Type compileLiteral(const LiteralExpression& node);
        TypedValue::Type compileBinary(const BinaryExpression& node);
        TypedValue::Type compileUnary(const UnaryExpression& node);
        TypedValue::Type compileIdentifier(const IdentifierExpression& node);
        TypedValue::Type compileFunctionCall(const FunctionCall& node);

        void compileVariableDeclaration(const VariableDeclaration& node);
        void compileAssignment(const Assignment& node);
        void compileIfStatement(const IfStatement& node);
        void compileBlockStatement(const BlockStatement& node);
        void compileProgram(const Program& node);

    private:
        VirtualMachine& vm;
        std::vector<Instruction> instructions;
        std::vector<TypedValue> constants;
        TypedValue::Type currentExpressionType;
        std::vector<std::unordered_map<std::string, VariableInfo>> scopeStack;
        int nextSlot = 0;

        TypedValue inferType(const Token& token);
        int addConstant(TypedValue value);
        void emit(OPCode opcode, int operand = 0);
        OPCode getOpCode(TokenType op, TypedValue::Type type, bool isUnary = false);

        void enterScope();
        void exitScope();
        VariableInfo* lookupVariable(const std::string& name);
        void declareVariable(const std::string& name, const VariableInfo& info);
};