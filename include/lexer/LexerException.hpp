#pragma once 
#include <exception>

class InvalidTokenTypeException : public std::exception {
    public: 
        const char* what() const noexcept override {
            return "Unsupported TokenType, did you forget to implement it?";
        }
};