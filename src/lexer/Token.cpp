#include "../../include/lexer/Token.hpp"

std::string Token::toString() const {
    return tokenTypeToString(this->type) + " (" + value + ") at " + 
        std::to_string(line) + ":" + std::to_string(column);
}