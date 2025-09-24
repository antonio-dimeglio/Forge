#include <gtest/gtest.h>
#include "../../include/lexer/Tokenizer.hpp"
#include "../../include/lexer/LexerException.hpp"

class ClassTokenTest : public ::testing::Test {
protected:
    Tokenizer tokenizer{""}; // Empty string initially

    void SetUp() override {
        // Called before each test
    }

    std::vector<Token> tokenize(const std::string& source) {
        tokenizer.reset(source);
        return tokenizer.tokenize();
    }

    void expectToken(const Token& token, TokenType expectedType, const std::string& expectedValue = "") {
        EXPECT_EQ(token.getType(), expectedType);
        if (!expectedValue.empty()) {
            EXPECT_EQ(token.getValue(), expectedValue);
        }
    }
};

// ============= CLASS KEYWORD TESTS =============

TEST_F(ClassTokenTest, ClassKeyword) {
    auto tokens = tokenize("class");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::CLASS, "class");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(ClassTokenTest, DefKeyword) {
    auto tokens = tokenize("def");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::DEF, "def");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(ClassTokenTest, SelfKeyword) {
    auto tokens = tokenize("self");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::SELF, "self");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(ClassTokenTest, InitMethod) {
    auto tokens = tokenize("__init__");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::IDENTIFIER, "__init__"); // Special identifier
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(ClassTokenTest, GenericBrackets) {
    auto tokens = tokenize("[T]");
    ASSERT_EQ(tokens.size(), 4);
    expectToken(tokens[0], TokenType::LSQUARE, "[");
    expectToken(tokens[1], TokenType::IDENTIFIER, "T");
    expectToken(tokens[2], TokenType::RSQUARE, "]");
    expectToken(tokens[3], TokenType::END_OF_FILE);
}

TEST_F(ClassTokenTest, BasicClassDefinition) {
    std::string input = "class Person {";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 4);
    expectToken(tokens[0], TokenType::CLASS, "class");
    expectToken(tokens[1], TokenType::IDENTIFIER, "Person");
    expectToken(tokens[2], TokenType::LBRACE, "{");
    expectToken(tokens[3], TokenType::END_OF_FILE);
}

TEST_F(ClassTokenTest, GenericClassDefinition) {
    std::string input = "class Array[T] {";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 7);
    expectToken(tokens[0], TokenType::CLASS, "class");
    expectToken(tokens[1], TokenType::IDENTIFIER, "Array");
    expectToken(tokens[2], TokenType::LSQUARE, "[");
    expectToken(tokens[3], TokenType::IDENTIFIER, "T");
    expectToken(tokens[4], TokenType::RSQUARE, "]");
    expectToken(tokens[5], TokenType::LBRACE, "{");
    expectToken(tokens[6], TokenType::END_OF_FILE);
}

TEST_F(ClassTokenTest, MultipleGenerics) {
    std::string input = "class Map[K, V] {";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 9);
    expectToken(tokens[0], TokenType::CLASS, "class");
    expectToken(tokens[1], TokenType::IDENTIFIER, "Map");
    expectToken(tokens[2], TokenType::LSQUARE, "[");
    expectToken(tokens[3], TokenType::IDENTIFIER, "K");
    expectToken(tokens[4], TokenType::COMMA, ",");
    expectToken(tokens[5], TokenType::IDENTIFIER, "V");
    expectToken(tokens[6], TokenType::RSQUARE, "]");
    expectToken(tokens[7], TokenType::LBRACE, "{");
    expectToken(tokens[8], TokenType::END_OF_FILE);
}

TEST_F(ClassTokenTest, ObjectInstantiation) {
    std::string input = "person: Person = Person(\"Alice\", 25)";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 11);
    expectToken(tokens[0], TokenType::IDENTIFIER, "person");
    expectToken(tokens[1], TokenType::COLON, ":");
    expectToken(tokens[2], TokenType::IDENTIFIER, "Person");
    expectToken(tokens[3], TokenType::ASSIGN, "=");
    expectToken(tokens[4], TokenType::IDENTIFIER, "Person");
    expectToken(tokens[5], TokenType::LPAREN, "(");
    expectToken(tokens[6], TokenType::STRING, "Alice");
    expectToken(tokens[7], TokenType::COMMA, ",");
    expectToken(tokens[8], TokenType::NUMBER, "25");
    expectToken(tokens[9], TokenType::RPAREN, ")");
    expectToken(tokens[10], TokenType::END_OF_FILE);
}

TEST_F(ClassTokenTest, GenericInstantiation) {
    std::string input = "arr: Array[int] = Array[int](10)";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 15);
    expectToken(tokens[0], TokenType::IDENTIFIER, "arr");
    expectToken(tokens[1], TokenType::COLON, ":");
    expectToken(tokens[2], TokenType::IDENTIFIER, "Array");
    expectToken(tokens[3], TokenType::LSQUARE, "[");
    expectToken(tokens[4], TokenType::INT, "int");
    expectToken(tokens[5], TokenType::RSQUARE, "]");
    expectToken(tokens[6], TokenType::ASSIGN, "=");
    expectToken(tokens[7], TokenType::IDENTIFIER, "Array");
    expectToken(tokens[8], TokenType::LSQUARE, "[");
    expectToken(tokens[9], TokenType::INT, "int");
    expectToken(tokens[10], TokenType::RSQUARE, "]");
    expectToken(tokens[11], TokenType::LPAREN, "(");
    expectToken(tokens[12], TokenType::NUMBER, "10");
    expectToken(tokens[13], TokenType::RPAREN, ")");
    expectToken(tokens[14], TokenType::END_OF_FILE);
}

TEST_F(ClassTokenTest, MethodCall) {
    std::string input = "person.greet()";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 6);
    expectToken(tokens[0], TokenType::IDENTIFIER, "person");
    expectToken(tokens[1], TokenType::DOT, ".");
    expectToken(tokens[2], TokenType::IDENTIFIER, "greet");
    expectToken(tokens[3], TokenType::LPAREN, "(");
    expectToken(tokens[4], TokenType::RPAREN, ")");
    expectToken(tokens[5], TokenType::END_OF_FILE);
}

TEST_F(ClassTokenTest, FieldAccess) {
    std::string input = "person.name";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 4);
    expectToken(tokens[0], TokenType::IDENTIFIER, "person");
    expectToken(tokens[1], TokenType::DOT, ".");
    expectToken(tokens[2], TokenType::IDENTIFIER, "name");
    expectToken(tokens[3], TokenType::END_OF_FILE);
}