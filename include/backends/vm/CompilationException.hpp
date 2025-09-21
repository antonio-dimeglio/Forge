#pragma once
#include <string>
#include <exception>

class CompilationException : public std::exception {
private:
    std::string message;

public:
    CompilationException(const std::string& msg, int line, int column) {
        message = msg + " at line " + std::to_string(line) + ", column " + std::to_string(column);
    }

    const char* what() const noexcept override {
        return message.c_str();
    }
};