#pragma once 
#include <exception>
#include <string>

class ParsingException : public std::exception {
private:
    std::string message;

public:
    ParsingException(const std::string& msg, int line, int column) {
        message = msg + " at line " + std::to_string(line) + ", column " + std::to_string(column);
    }

    const char* what() const noexcept override {
        return message.c_str();
    }
};