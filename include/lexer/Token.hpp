#pragma once 
#include <string> 
#include "TokenType.hpp"

class Token {
    private:
        TokenType type;
        std::string value;
        int line;
        int column;

    public:
        Token(TokenType type, std::string value, int line, int column ) :
            type(type), value(value), line(line), column(column) {};
        Token(TokenType type, int line, int column) :
            type(type), value(""), line(line), column(column) {};
        
        std::string toString () const;
        
        TokenType getType() const { return type; }
        std::string getValue() const { return value; }
        int getLine() const { return line; }
        int getColumn() const { return column; }
};