#pragma once 
#include <string> 
#include "LexerException.hpp"

#define TOKEN_TYPES(X) \
    /* Literals */ \
    X(NUMBER, "NUMBER") \
    X(STRING, "STRING") \
    X(TRUE, "TRUE") \
    X(FALSE, "FALSE") \
    \
    /* Operators */ \
    X(PLUS, "PLUS") \
    X(MINUS, "MINUS") \
    X(MULT, "MULT") \
    X(DIV, "DIV") \
    X(ASSIGN, "ASSIGN") \
    X(INFER_ASSIGN, "INFER_ASSIGN") \
    X(EQUAL_EQUAL, "EQUAL_EQUAL") \
    X(NOT_EQUAL, "NOT_EQUAL") \
    X(LESS, "LESS") \
    X(GREATER, "GREATER") \
    X(NOT, "NOT") \
    X(LOGIC_AND, "LOGIC_AND") \
    X(BITWISE_AND, "BITWISE_AND") \
    X(LOGIC_OR, "LOGIC_OR") \
    X(BITWISE_OR, "BITWISE_OR") \
    X(BITWISE_XOR, "BITWISE_XOR") \
    X(GEQ, "GEQ") \
    X(LEQ, "LEQ") \
    X(ARROW, "ARROW") \
    X(PLUS_EQ, "PLUS_EQ") \
    X(MINUS_EQ, "MINUS_EQ") \
    X(MULT_EQ, "MULT_EQ") \
    X(DIV_EQ, "DIV_EQ") \
    \
    /* Keywords */ \
    X(IF, "IF") \
    X(ELSE, "ELSE") \
    X(WHILE, "WHILE") \
    X(FOR, "FOR") \
    X(DEF, "DEF") \
    X(RETURN, "RETURN") \
    X(INT, "INT") \
    X(STR, "STR") \
    X(BOOL, "BOOL") \
    X(FLOAT, "FLOAT") \
    X(DOUBLE, "DOUBLE") \
    X(CLASS, "CLASS") \
    X(SELF, "SELF") \
    X(UNIQUE, "UNIQUE") \
    X(SHARED, "SHARED") \
    X(WEAK, "WEAK") \
    X(MAYBE, "MAYBE") \
    \
    /* Punctuation */ \
    X(LPAREN, "LPAREN") \
    X(RPAREN, "RPAREN") \
    X(LBRACE, "LBRACE") \
    X(RBRACE, "RBRACE") \
    X(LSQUARE, "LSQUARE") \
    X(RSQUARE, "RSQUARE") \
    X(LANGLE, "LANGLE") \
    X(RANGLE, "RANGLE") \
    X(COMMA, "COMMA") \
    X(COLON, "COLON") \
    X(DOT, "DOT") \
    \
    /* Special */ \
    X(IDENTIFIER, "IDENTIFIER") \
    X(NEWLINE, "NEWLINE") \
    X(END_OF_FILE, "END_OF_FILE") \
    X(COMMENT, "COMMENT") \
    X(EXTERN, "EXTERN") \
    \
    /* Memory */ \
    X(RAW_PTR, "RAW_PTR") \
    X(IMM_REF, "IMM_REF") \
    X(MUT_REF, "MUT_REF") \
    X(MOVE, "MOVE") \
    X(NULL_, "NULL") \
    X(DEFER, "DEFER") \
    X(NEW, "NEW") \


enum class TokenType {
    #define X(name, str) name, 
    TOKEN_TYPES(X)
    #undef X
};

inline std::string tokenTypeToString(TokenType tt) {
    static const char* names[] = {
        #define X(name, str) str,
        TOKEN_TYPES(X)
        #undef X
    };
    return names[static_cast<int>(tt)];
}