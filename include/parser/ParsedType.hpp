#pragma once
#include "../lexer/Token.hpp"
#include <vector>

class Parser; 

struct ParsedType {
    Token primaryType;
    std::vector<Token> typeParameters; 
    int nestingLevel;

    bool isSimpleType() const { return typeParameters.empty(); }
    std::string toString() const;
};