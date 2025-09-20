#pragma once 
#include <string> 
#include "LexerException.hpp"

enum class TokenType {
    // Literals
    NUMBER, STRING, TRUE, FALSE,

    // Operators 
    PLUS, MINUS, MULT, DIV, ASSIGN,
    EQUAL_EQUAL, NOT_EQUAL, LESS, GREATER, 

    // Keywords
    IF, ELSE, WHILE, FOR, DEF, RETURN,
    INT, STR, BOOL, FLOAT, DOUBLE,

    // Punctuation
    LPAREN, RPAREN, LBRACE, RBRACE, COMMA, COLON,

    // Special
    IDENTIFIER, NEWLINE, END_OF_FILE
};

inline std::string tokenTypeToString(TokenType tt) {
    switch (tt)
    {   
        case TokenType::NUMBER:
            return "NUMBER";
        case TokenType::STRING:
            return "STRING";
        case TokenType::TRUE:
            return "TRUE";
        case TokenType::FALSE:
            return "FALSE";

        case TokenType::PLUS:
            return "PLUS";
        case TokenType::MINUS:
            return "MINUS";
        case TokenType::MULT:
            return "MULT";
        case TokenType::DIV:
            return "DIV";
        case TokenType::ASSIGN:
            return "ASSIGN";
        case TokenType::EQUAL_EQUAL:
            return "EQUAL_EQUAL";
        case TokenType::NOT_EQUAL:
            return "NOT_EQUAL";
        case TokenType::LESS:
            return "LESS";
        case TokenType::GREATER:
            return "GREATER";

        case TokenType::IF:
            return "IF";
        case TokenType::ELSE:
            return "ELSE";
        case TokenType::WHILE:
            return "WHILE";
        case TokenType::FOR:
            return "FOR";
        case TokenType::DEF:
            return "DEF";
        case TokenType::RETURN:
            return "RETURN";
        case TokenType::INT:
            return "INT";
        case TokenType::STR:
            return "STR";
        case TokenType::BOOL:
            return "BOOL";
        case TokenType::FLOAT:
            return "FLOAT";
        case TokenType::DOUBLE:
            return "DOUBLE";

        case TokenType::LPAREN:
            return "LPAREN";
        case TokenType::RPAREN:
            return "RPAREN";
        case TokenType::LBRACE:
            return "LBRACE";
        case TokenType::RBRACE:
            return "RBRACE";
        case TokenType::COMMA:
            return "COMMA";
        case TokenType::COLON:
            return "COLON";

        case TokenType::IDENTIFIER:
            return "IDENTIFIER";
        case TokenType::NEWLINE:
            return "NEWLINE";
        case TokenType::END_OF_FILE:
            return "END_OF_FILE";           
        default:
            throw InvalidTokenTypeException();
        }
}