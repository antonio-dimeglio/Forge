#pragma once

#include <memory>
#include <string>
#include <vector>
#include "../lexer/Token.hpp"

class BytecodeCompiler;

class Expression {
    public:
        virtual ~Expression() = default;
        template<typename Visitor>
        auto accept(Visitor& visitor) const -> decltype(visitor.visit(*this))  { return visitor.visit(*this); }
        virtual std::string toString(int indent = 0) const = 0;
};

class LiteralExpression: public Expression {
    public:
        Token value;
        LiteralExpression(Token value): value(value) {};
        std::string toString(int indent = 0) const override;
};

class ArrayLiteralExpression: public Expression {
    public:
        std::vector<std::unique_ptr<Expression>> arrayValues;
            ArrayLiteralExpression(
                std::vector<std::unique_ptr<Expression>> values
            ):  arrayValues(std::move(values)) {};
        std::string toString(int indent = 0) const override;
};

class IndexAccessExpression: public Expression {
    public:
        std::unique_ptr<Expression> array;
        std::unique_ptr<Expression> index;
        IndexAccessExpression(
                std::unique_ptr<Expression> array,
                std::unique_ptr<Expression> index
            ):  array(std::move(array)), index(std::move(index)) {};
        std::string toString(int indent = 0) const override;
};

class MemberAccessExpression: public Expression {
    public:
        std::unique_ptr<Expression> object;  
        std::string memberName;              
        std::vector<std::unique_ptr<Expression>> arguments;
        bool isMethodCall; 

        MemberAccessExpression(
            std::unique_ptr<Expression> object,
            std::string memberName,
            std::vector<std::unique_ptr<Expression>> arguments,
            bool isMethodCall
        ): object(std::move(object)), memberName(memberName),
            arguments(std::move(arguments)), isMethodCall(isMethodCall) {}

        std::string toString(int indent = 0) const override;
};

class IdentifierExpression: public Expression {
    public:
        std::string name;
        IdentifierExpression(std::string name) : name(name) {};
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
        std::string toString(int indent = 0) const override;
};

class FunctionCall : public Expression {
    public:
        std::string functionName;
        std::vector<std::unique_ptr<Expression>> arguments;  

        FunctionCall(std::string name, std::vector<std::unique_ptr<Expression>> args = {})
            : functionName(name), arguments(std::move(args)) {}

        std::string toString(int indent = 0) const override;
};