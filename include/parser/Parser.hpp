#pragma once 
#include "Expression.hpp"
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

        std::unique_ptr<Expression> expression();
        std::unique_ptr<Expression> equality();
        std::unique_ptr<Expression> comparison();
        std::unique_ptr<Expression> term();
        std::unique_ptr<Expression> factor();
        std::unique_ptr<Expression> unary();
        std::unique_ptr<Expression> primary();

        void skipNewLines();
    public:
        Parser(const std::vector<Token>& tokens) : tokens(tokens) {};
        std::unique_ptr<Expression> generateAST();
        void reset(const std::vector<Token>& tokens);
};