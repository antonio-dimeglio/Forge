#pragma once

#include <memory>
#include <string>
#include "../lexer/Token.hpp"

class BytecodeCompiler;

class Expression {
    public:
        virtual ~Expression() = default;
        virtual void accept(BytecodeCompiler& compiler) const = 0;
        virtual std::string toString(int indent = 0) const = 0;
};

class LiteralExpression: public Expression {
    public:
        Token value;
        LiteralExpression(Token value): value(value) {};
        void accept(BytecodeCompiler& compiler) const override;
        std::string toString(int indent = 0) const override;
};

class IdentifierExpression: public Expression {
    public:
        std::string name;
        IdentifierExpression(std::string name) : name(name) {};
        void accept(BytecodeCompiler& compiler) const override;
        std::string toString(int indent = 0) const override;
};

class BinaryExpression: public Expression {
    public:
        std::unique_ptr<Expression> left; 
        Token operator_;
        std::unique_ptr<Expression> right;

        BinaryExpression(
            std::unique_ptr<Expression> left,
            Token operator_,
            std::unique_ptr<Expression> right
        ) : left(std::move(left)), operator_(operator_), right(std::move(right)) {};
        void accept(BytecodeCompiler& compiler) const override;
        std::string toString(int indent = 0) const override;
};

class UnaryExpression: public Expression {
    public:
        Token operator_;
        std::unique_ptr<Expression> operand;

        UnaryExpression(
            Token operator_,
            std::unique_ptr<Expression> operand
        ) : operator_(operator_), operand(std::move(operand)) {}
        void accept(BytecodeCompiler& compiler) const override;
        std::string toString(int indent = 0) const override;
};