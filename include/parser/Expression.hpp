#pragma once

#include <memory>
#include <string>
#include "../lexer/Token.hpp"

class Expression {
    public:
        virtual ~Expression() = default;
};

class LiteralExpression: public Expression {
    public:
        Token value;     
        LiteralExpression(Token value): value(value) {};
        
};

class IdentifierExpression: public Expression {
    public:
        std::string name;
        IdentifierExpression(std::string name) : name(name) {};
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
};

class UnaryExpression: public Expression {
    public:
        Token operator_;
        std::unique_ptr<Expression> operand;

        UnaryExpression(
            Token operator_,
            std::unique_ptr<Expression> operand
        ) : operator_(operator_), operand(std::move(operand)) {}
};