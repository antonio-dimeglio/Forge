#pragma once 
#include <string> 
#include <vector> 
#include "Token.hpp"
#include "Operator.hpp"

class Tokenizer { 
    private:
        std::string source;
        size_t idx; 
        int line;
        int column; 

        char current(); 
        char peek(int offset = 1);
        void advance();
        bool isAtEnd();
        
        bool isDigit(char c);
        bool isAlpha(char c);
        bool isAlphaNumeric(char c);
        bool isWhitespace(char c);
        TokenType getKeywordType(const std::string& text);

        Token scanNumber();
        Token scanIdentifier();
        Token scanString(char startingChar = '"');
        Token scanOperator(char ch);

        void skipWhitespace();

        void errorToken(const std::string& message);
    public:
        Tokenizer(const std::string& source) : 
            source(source), idx(0), line(1), column(1) {};
        std::vector<Token> tokenize();
        void reset(const std::string& source);
};