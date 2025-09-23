#include <gtest/gtest.h>
#include "../../include/lexer/TokenType.hpp"
#include "../../include/lexer/Token.hpp"

// TokenType Tests
TEST(TokenTypeTest, BasicTokenToString) {
    EXPECT_EQ(tokenTypeToString(TokenType::ASSIGN), "ASSIGN");
    EXPECT_EQ(tokenTypeToString(TokenType::PLUS), "PLUS");
    EXPECT_EQ(tokenTypeToString(TokenType::IF), "IF");
    EXPECT_EQ(tokenTypeToString(TokenType::IDENTIFIER), "IDENTIFIER");
}

TEST(TokenTypeTest, AllTokensConvertWithoutException) {
    std::vector<TokenType> allTokens = {
        TokenType::NUMBER, TokenType::STRING, TokenType::TRUE, TokenType::FALSE,
        TokenType::PLUS, TokenType::MINUS, TokenType::MULT, TokenType::DIV,
        TokenType::ASSIGN, TokenType::EQUAL_EQUAL, TokenType::NOT_EQUAL,
        TokenType::LESS, TokenType::GREATER,
        TokenType::IF, TokenType::ELSE, TokenType::WHILE, TokenType::FOR,
        TokenType::DEF, TokenType::RETURN, TokenType::INT, TokenType::STR,
        TokenType::BOOL, TokenType::FLOAT, TokenType::DOUBLE,
        TokenType::LPAREN, TokenType::RPAREN, TokenType::LBRACE, TokenType::RBRACE,
        TokenType::COMMA, TokenType::COLON,
        TokenType::IDENTIFIER, TokenType::NEWLINE, TokenType::END_OF_FILE
    };

    for(auto token : allTokens) {
        std::string result = tokenTypeToString(token);
        EXPECT_FALSE(result.empty()) << "Token converted to empty string";
    }
}

// Token Tests
TEST(TokenTest, ConstructorWithValue) {
    Token token(TokenType::PLUS, "+", 1, 5);
    EXPECT_EQ(token.getType(), TokenType::PLUS);
    EXPECT_EQ(token.getValue(), "+");
    EXPECT_EQ(token.getLine(), 1);
    EXPECT_EQ(token.getColumn(), 5);
}

TEST(TokenTest, ConstructorWithoutValue) {
    Token token(TokenType::IF, 2, 10);
    EXPECT_EQ(token.getType(), TokenType::IF);
    EXPECT_EQ(token.getValue(), "");
    EXPECT_EQ(token.getLine(), 2);
    EXPECT_EQ(token.getColumn(), 10);
}

TEST(TokenTest, ToStringContainsKeyInfo) {
    Token token(TokenType::IDENTIFIER, "myVar", 3, 7);
    std::string result = token.toString();

    EXPECT_NE(result.find("IDENTIFIER"), std::string::npos);
    EXPECT_NE(result.find("myVar"), std::string::npos);
    EXPECT_NE(result.find("3"), std::string::npos);
    EXPECT_NE(result.find("7"), std::string::npos);
}