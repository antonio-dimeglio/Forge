#pragma once
#include "../lexer/Token.hpp"
#include <vector>

class Parser; 

enum class SmartPointerType {
    None,
    Unique,
    Shared,
    Weak
};

struct ParsedType {
    Token primaryType;
    std::vector<Token> typeParameters;
    int nestingLevel;
    bool isPointer;
    bool isReference;
    bool isMutReference;
    bool isOptional;
    SmartPointerType smartPointerType;

    bool isSimpleType() const { return typeParameters.empty(); }
    bool isSmartPointer() const { return smartPointerType != SmartPointerType::None; }
    std::string toString() const;
};