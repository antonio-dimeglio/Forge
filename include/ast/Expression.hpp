#pragma once

#include <memory>
#include <string>
#include <vector>
#include "../lexer/Token.hpp"
#include "../backend/errors/ErrorTypes.hpp"

namespace forge::ast {

class Expression {
    public:
        virtual ~Expression() = default;
        template<typename Visitor>
        auto accept(Visitor& visitor) const -> decltype(visitor.visit(*this))  { return visitor.visit(*this); }
        virtual std::string toString(int indent = 0) const = 0;

        // Source location for error reporting
        forge::errors::SourceLocation getLocation() const { return location_; }

    protected:
        forge::errors::SourceLocation location_;

    public:
        Expression(forge::errors::SourceLocation location) : location_(location) {}
};

class LiteralExpression: public Expression {
    public:
        Token value;
        LiteralExpression(Token value, forge::errors::SourceLocation location)
            : Expression(location), value(value) {}
        std::string toString(int indent = 0) const override;
};

class ArrayLiteralExpression: public Expression {
    public:
        std::vector<std::unique_ptr<Expression>> arrayValues;
        ArrayLiteralExpression(
            std::vector<std::unique_ptr<Expression>> values,
            forge::errors::SourceLocation location
        ) : Expression(location), arrayValues(std::move(values)) {}
        std::string toString(int indent = 0) const override;
};

class IndexAccessExpression: public Expression {
    public:
        std::unique_ptr<Expression> array;
        std::unique_ptr<Expression> index;
        IndexAccessExpression(
            std::unique_ptr<Expression> array,
            std::unique_ptr<Expression> index,
            forge::errors::SourceLocation location
        ) : Expression(location), array(std::move(array)), index(std::move(index)) {}
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
            bool isMethodCall,
            forge::errors::SourceLocation location
        ) : Expression(location), object(std::move(object)), memberName(memberName),
            arguments(std::move(arguments)), isMethodCall(isMethodCall) {}

        std::string toString(int indent = 0) const override;
};

class IdentifierExpression: public Expression {
    public:
        std::string name;
        IdentifierExpression(std::string name, forge::errors::SourceLocation location)
            : Expression(location), name(name) {}
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
            std::unique_ptr<Expression> right,
            forge::errors::SourceLocation location
        ) : Expression(location), left(std::move(left)), operator_(operator_), right(std::move(right)) {}
        std::string toString(int indent = 0) const override;
};

class UnaryExpression: public Expression {
    public:
        Token operator_;
        std::unique_ptr<Expression> operand;

        UnaryExpression(
            Token operator_,
            std::unique_ptr<Expression> operand,
            forge::errors::SourceLocation location
        ) : Expression(location), operator_(operator_), operand(std::move(operand)) {}
        std::string toString(int indent = 0) const override;
};

class FunctionCall : public Expression {
    public:
        std::string functionName;
        std::vector<Token> typeArguments;
        std::vector<std::unique_ptr<Expression>> arguments;

        FunctionCall(
            std::string name,
            std::vector<Token> typeArgs,
            std::vector<std::unique_ptr<Expression>> args,
            forge::errors::SourceLocation location
        ) : Expression(location), functionName(name), typeArguments(typeArgs), arguments(std::move(args)) {}

        std::string toString(int indent = 0) const override;
};

class ObjectInstantiation : public Expression {
    public:
        Token className;
        std::vector<std::unique_ptr<Expression>> arguments;

        ObjectInstantiation(
            Token className,
            std::vector<std::unique_ptr<Expression>> arguments,
            forge::errors::SourceLocation location
        ) : Expression(location), className(className), arguments(std::move(arguments)) {}

        std::string toString(int indent = 0) const override;
};

class GenericInstantiation : public Expression {
    public:
        Token className;
        std::vector<Token> typeArguments;  // [int], [string, int]
        std::vector<std::unique_ptr<Expression>> arguments;

        GenericInstantiation(
            Token className,
            std::vector<Token> typeArguments,
            std::vector<std::unique_ptr<Expression>> arguments,
            forge::errors::SourceLocation location
        ) : Expression(location), className(className), typeArguments(std::move(typeArguments)),
            arguments(std::move(arguments)) {}

        std::string toString(int indent = 0) const override;
};

class MoveExpression : public Expression {
    public:
        Token moveToken;
        std::unique_ptr<Expression> operand;

        MoveExpression(
            Token moveToken,
            std::unique_ptr<Expression> operand,
            forge::errors::SourceLocation location
        ) : Expression(location), moveToken(moveToken), operand(std::move(operand)) {}

        std::string toString(int indent = 0) const override;
};

class NewExpression : public Expression {
    public:
        std::unique_ptr<Expression> valueExpression;

        NewExpression(
            std::unique_ptr<Expression> valueExpression,
            forge::errors::SourceLocation location
        ) : Expression(location), valueExpression(std::move(valueExpression)) {}

        std::string toString(int indent = 0) const override;
};

class OptionalExpression : public Expression {
    public:
        Token type;
        std::unique_ptr<Expression> value;

        OptionalExpression(
            Token type,
            std::unique_ptr<Expression> value,
            forge::errors::SourceLocation location
        ) : Expression(location), type(type), value(std::move(value)) {}

        std::string toString(int indent = 0) const override;
};

} // namespace forge::ast