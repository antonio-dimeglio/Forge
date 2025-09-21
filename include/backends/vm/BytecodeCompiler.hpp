#pragma once 
#include "Instruction.hpp"
#include <vector>
#include "../../parser/Expression.hpp"
#include "../../lexer/Token.hpp"
#include "RuntimeException.hpp"

class BytecodeCompiler {
    public:
        struct CompiledProgram {
            std::vector<Instruction> instructions;
            std::vector<TypedValue> constants;
            std::vector<std::string> strings;
        };

        
        CompiledProgram compile(std::unique_ptr<Expression> ast);
        TypedValue::Type compileExpression(const Expression& node);
        TypedValue::Type compileLiteral(const LiteralExpression& node);
        TypedValue::Type compileBinary(const BinaryExpression& node);
        TypedValue::Type compileUnary(const UnaryExpression& node);
        TypedValue::Type compileIdentifier(const IdentifierExpression& node);
    private:
        std::vector<Instruction> instructions;
        std::vector<TypedValue> constants;
        std::vector<std::string> tempStringPool;
        TypedValue::Type currentExpressionType;

        TypedValue inferType(const Token& token);
        int addConstant(TypedValue value);
        void emit(OPCode opcode, int operand = 0);
        OPCode getOpCode(TokenType op, TypedValue::Type type, bool isUnary = false);
};