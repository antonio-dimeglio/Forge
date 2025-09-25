#pragma once 

#include <memory>
#include <string>
#include <vector>
#include "Expression.hpp"

class BytecodeCompiler;

class Statement {
    public:
        virtual ~Statement() = default;
        template<typename Visitor>
        auto accept(Visitor& visitor) const -> decltype(visitor.visit(*this)) { return visitor.visit(*this); }
        virtual std::string toString(int indent = 0) const = 0;
};

class Program : public Statement {
    public:
        std::vector<std::unique_ptr<Statement>> statements;

        Program(std::vector<std::unique_ptr<Statement>> statements)
            : statements(std::move(statements)) {}
        std::string toString(int indent = 0) const override;
};


class ExpressionStatement : public Statement {
    public:
        std::unique_ptr<Expression> expression;

        ExpressionStatement(std::unique_ptr<Expression> expr)
            : expression(std::move(expr)) {}
        std::string toString(int indent = 0) const override;
};


class VariableDeclaration: public Statement {
    public:
        Token variable;
        Token type;
        std::unique_ptr<Expression> expr; 
        VariableDeclaration (
            Token variable,
            Token type,
            std::unique_ptr<Expression> expr
        ) : variable(variable), type(type), expr(std::move(expr)) {}
        std::string toString(int indent = 0) const override;
};

class Assignment: public Statement {
    public:
        std::unique_ptr<Expression> lvalue;  // Left-hand side expression (*ptr, arr[i], obj.field, etc.)
        std::unique_ptr<Expression> rvalue;  // Right-hand side expression
        Assignment(
            std::unique_ptr<Expression> lvalue,
            std::unique_ptr<Expression> rvalue
        ) : lvalue(std::move(lvalue)), rvalue(std::move(rvalue)) {}
        std::string toString(int indent = 0) const override;
};


class IndexAssignment: public Statement {
    public:
        std::unique_ptr<Expression> lvalue;  // The array[index] expression
        std::unique_ptr<Expression> rvalue;  // The value to assign
        IndexAssignment(
            std::unique_ptr<Expression> lvalue,
            std::unique_ptr<Expression> rvalue
        ) : lvalue(std::move(lvalue)), rvalue(std::move(rvalue)) {}
        std::string toString(int indent = 0) const override;
};

class BlockStatement : public Statement {
public:
    std::vector<std::unique_ptr<Statement>> statements;

    explicit BlockStatement(std::vector<std::unique_ptr<Statement>> stmts)
        : statements(std::move(stmts)) {}

    std::string toString(int indent = 0) const override;
};

class IfStatement: public Statement {
    public:
        std::unique_ptr<Expression> condition;
        std::unique_ptr<BlockStatement> thenBlock;
        std::unique_ptr<BlockStatement> elseBlock;
        IfStatement(
            std::unique_ptr<Expression> condition,
            std::unique_ptr<BlockStatement> thenBlock,
            std::unique_ptr<BlockStatement> elseBlock
        ) : condition(std::move(condition)),
            thenBlock(std::move(thenBlock)),
            elseBlock(std::move(elseBlock)) {}
        std::string toString(int indent = 0) const override;
};

class WhileStatement: public Statement {
    public:
        std::unique_ptr<Expression> condition;
        std::unique_ptr<BlockStatement> body;
        WhileStatement(
            std::unique_ptr<Expression> condition,
            std::unique_ptr<BlockStatement> body
        ) : condition(std::move(condition)),
            body(std::move(body)) {}
        std::string toString(int indent = 0) const override;
};

struct StatementParameter {
    Token name;
    Token type; 
};

class FunctionDefinition : public Statement {
    public:
        Token functionName;
        Token functionReturnType;
        std::vector<Token> typeParameters;
        std::vector<StatementParameter> parameters;
        std::unique_ptr<BlockStatement> body;
        FunctionDefinition(
            Token functionName, Token functionReturnType,
            std::vector<Token> typeParameters,
            std::vector<StatementParameter> parameters,
            std::unique_ptr<BlockStatement> body
        ) : functionName(functionName), functionReturnType(functionReturnType),
            typeParameters(typeParameters), parameters(parameters), body(std::move(body)) {}
        std::string toString(int indent = 0) const override;
};

class ReturnStatement : public Statement {
    public:
        std::unique_ptr<Expression> returnValue;
        
        ReturnStatement(
            std::unique_ptr<Expression> returnValue
        ) : returnValue(std::move(returnValue)) {};
        std::string toString(int indent = 0) const override;
};


struct FieldDefinition {
    Token name;
    Token type;    
};

struct MethodDefinition {
    Token methodName;
    Token returnType;
    std::vector<StatementParameter> parameters;
    std::unique_ptr<BlockStatement> body;

    MethodDefinition(Token methodName, Token returnType,
                     std::vector<StatementParameter> parameters,
                     std::unique_ptr<BlockStatement> body)
        : methodName(methodName), returnType(returnType),
          parameters(std::move(parameters)), body(std::move(body)) {}
};

class ClassDefinition : public Statement {
    public:
        Token className;
        std::vector<Token> genericParameters;
        std::vector<FieldDefinition> fields;
        std::vector<MethodDefinition> methods;

        ClassDefinition(Token className,
                        std::vector<Token> genericParameters,
                        std::vector<FieldDefinition> fields,
                        std::vector<MethodDefinition> methods)
            : className(className), genericParameters(std::move(genericParameters)),
            fields(std::move(fields)), methods(std::move(methods)) {}

        std::string toString(int indent = 0) const override;
};

class DeferStatement : public Statement {
    public:
        std::unique_ptr<Expression> expression;

        DeferStatement(std::unique_ptr<Expression> expression)
            : expression(std::move(expression)) {}

        
        std::string toString(int indent = 0) const override;
};

class ExternStatement : public Statement {
    public: 
        Token functionName;
        std::vector<StatementParameter> parameters; 
        Token returnType;

        ExternStatement(Token functionName,
                        std::vector<StatementParameter> parameters,
                        Token returnType) 
            : functionName(functionName), parameters(std::move(parameters)),
                returnType(returnType) {}

        std::string toString(int indent = 0) const override;
};