#pragma once 
#include "Expression.hpp"
#include "Statement.hpp"
#include "../lexer/Token.hpp"
#include <vector>
#include <memory>

class Parser {
    private:
        std::vector<Token> tokens; 
        size_t idx = 0;

        Token current(); 
        Token peek(int offset = 1);
        Token advance();
        bool isAtEnd();

        std::unique_ptr<Expression> parseExpression();
        std::unique_ptr<Expression> parseLogicalOr();
        std::unique_ptr<Expression> parseLogicalAnd();
        std::unique_ptr<Expression> parseBitwiseOr();
        std::unique_ptr<Expression> parseBitwiseXor();
        std::unique_ptr<Expression> parseBitwiseAnd();
        std::unique_ptr<Expression> parseEquality();
        std::unique_ptr<Expression> parseComparison();
        std::unique_ptr<Expression> parseTerm();
        std::unique_ptr<Expression> parseFactor();
        std::unique_ptr<Expression> parseUnary();
        std::unique_ptr<Expression> parsePrimary();

        std::unique_ptr<Statement> parseExpressionStatement();
        std::unique_ptr<Statement> parseStatement();
        std::unique_ptr<Statement> parseVariableDeclaration();
        std::unique_ptr<Statement> parseAssignment();
        std::unique_ptr<Statement> parseIfStatement();
        std::unique_ptr<Statement> parseWhileStatement();
        std::unique_ptr<Statement> parseFunctionDefinition();
        std::unique_ptr<Statement> parseReturnStatement();
        std::unique_ptr<BlockStatement> parseBlockStatement();

        void skipNewLines();
    public:
        Parser(const std::vector<Token>& tokens) : tokens(tokens) {};
        std::unique_ptr<Statement> parseProgram();
        void reset(const std::vector<Token>& tokens);
};