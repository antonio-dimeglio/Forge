#pragma once

#include "Token.hpp"
#include <vector>
#include <unordered_map>

struct CompoundOp {
    char nextChar;      // What character to look for next
    TokenType tokenType; // What token to create
    std::string text;   // The full operator text
};

struct OperatorInfo {
    TokenType singleCharToken;           // For single character ('+' -> PLUS)
    std::string singleCharText;          // The text for single char
    std::vector<CompoundOp> compounds;   // Possible compounds ('+=' -> PLUS_EQ)
};

static const std::unordered_map<char, OperatorInfo> operatorTable = {
    {'+', {TokenType::PLUS, "+", {
        {'=', TokenType::PLUS_EQ, "+="}
    }}},
    {'-', {TokenType::MINUS, "-", {
        {'>', TokenType::ARROW, "->"},
        {'=', TokenType::MINUS_EQ, "-="}
    }}},
    {'*', {TokenType::MULT, "*", {
        {'=', TokenType::MULT_EQ, "*="}
    }}},
    {'/', {TokenType::DIV, "/", {
        {'=', TokenType::DIV_EQ, "/="},
        {'/', TokenType::COMMENT, "//"}  
    }}},
    {'=', {TokenType::ASSIGN, "=", {
        {'=', TokenType::EQUAL_EQUAL, "=="},
    }}},
    {':', {TokenType::COLON, ":", {
        {'=', TokenType::INFER_ASSIGN, ":="},
    }}},
    {'!', {TokenType::NOT, "!", {
        {'=', TokenType::NOT_EQUAL, "!="}
    }}},
    {'>', {TokenType::GREATER, ">", {
        {'=', TokenType::GEQ, ">="}
    }}},
    {'<', {TokenType::LESS, "<", {
        {'=', TokenType::LEQ, "<="}
    }}},
    {'&', {TokenType::BITWISE_AND, "&", {
        {'&', TokenType::LOGIC_AND, "&&"}
    }}},
    {'|', {TokenType::BITWISE_OR, "|", {
        {'|', TokenType::LOGIC_OR, "||"}
    }}},
    {'^', {TokenType::BITWISE_XOR, "^", {}}},  // No compounds for XOR
    // Simple punctuation (no compounds)
    {'[', {TokenType::LSQUARE, "[", {}}},
    {']', {TokenType::RSQUARE, "]", {}}},
    {'(', {TokenType::LPAREN, "(", {}}},
    {')', {TokenType::RPAREN, ")", {}}},
    {'{', {TokenType::LBRACE, "{", {}}},
    {'}', {TokenType::RBRACE, "}", {}}},
    {',', {TokenType::COMMA, ",", {}}}
};
