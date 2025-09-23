#pragma once 

#include <memory>
#include <string>
#include <vector>
#include "Expression.hpp"

class BytecodeCompiler;

class Statement {
    public:
        virtual ~Statement() = default;
        virtual void accept(BytecodeCompiler& compiler) const = 0;
        virtual std::string toString(int indent = 0) const = 0;
};

class Program : public Statement {
    public:
        std::vector<std::unique_ptr<Statement>> statements;

        Program(std::vector<std::unique_ptr<Statement>> statements)
            : statements(std::move(statements)) {}
        void accept(BytecodeCompiler& compiler) const override;
        std::string toString(int indent = 0) const override;
};


class ExpressionStatement : public Statement {
    public:
        std::unique_ptr<Expression> expression;

        ExpressionStatement(std::unique_ptr<Expression> expr)
            : expression(std::move(expr)) {}
        void accept(BytecodeCompiler& compiler) const override;
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
        void accept(BytecodeCompiler& compiler) const override;
        std::string toString(int indent = 0) const override;
};

class Assignment: public Statement {
    public:
        Token variable;
        std::unique_ptr<Expression> expr;
        Assignment(
            Token variable,
            std::unique_ptr<Expression> expr
        ) : variable(variable), expr(std::move(expr)) {}
        void accept(BytecodeCompiler& compiler) const override;
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
        void accept(BytecodeCompiler& compiler) const override;
        std::string toString(int indent = 0) const override;
};

class BlockStatement : public Statement {
public:
    std::vector<std::unique_ptr<Statement>> statements;

    explicit BlockStatement(std::vector<std::unique_ptr<Statement>> stmts)
        : statements(std::move(stmts)) {}

    void accept(BytecodeCompiler& compiler) const override;

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
        void accept(BytecodeCompiler& compiler) const override;
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
        void accept(BytecodeCompiler& compiler) const override;
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
        std::vector<StatementParameter> parameters;
        std::unique_ptr<BlockStatement> body;
        FunctionDefinition(
            Token functionName, Token functionReturnType,
            std::vector<StatementParameter> parameters,
            std::unique_ptr<BlockStatement> body
        ) : functionName(functionName), functionReturnType(functionReturnType),
            parameters(parameters), body(std::move(body)) {}
        void accept(BytecodeCompiler& compiler) const override;
        std::string toString(int indent = 0) const override;
};

class ReturnStatement : public Statement {
    public:
        std::unique_ptr<Expression> returnValue;
        
        ReturnStatement(
            std::unique_ptr<Expression> returnValue
        ) : returnValue(std::move(returnValue)) {};
        void accept(BytecodeCompiler& compiler) const override;
        std::string toString(int indent = 0) const override;
};