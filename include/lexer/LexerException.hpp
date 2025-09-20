#pragma once 
#include <exception>

class InvalidTokenTypeException : public std::exception {
    public: 
        const char* what() const noexcept override {
            return "Unsupported TokenType, did you forget to implement it?";
        }
};

class InvalidSyntaxException : public std::exception {
private:
    std::string message;

public:
    InvalidSyntaxException(const std::string& msg, int line, int column) {
        message = msg + " at line " + std::to_string(line) + ", column " + std::to_string(column);
    }

    const char* what() const noexcept override {
        return message.c_str();
    }
};