#include <gtest/gtest.h>
#include "../../include/lexer/Lexer.hpp"

class ClassTokenTest : public ::testing::Test {
protected:
    void tokenizeAndCheck(const std::string& input, const std::vector<TokenType>& expectedTypes) {
        Lexer lexer(input);
        auto tokens = lexer.tokenize();

        ASSERT_EQ(tokens.size(), expectedTypes.size() + 1); // +1 for EOF

        for (size_t i = 0; i < expectedTypes.size(); ++i) {
            EXPECT_EQ(tokens[i].getType(), expectedTypes[i])
                << "Token " << i << " expected " << static_cast<int>(expectedTypes[i])
                << " but got " << static_cast<int>(tokens[i].getType());
        }

        EXPECT_EQ(tokens.back().getType(), TokenType::END_OF_FILE);
    }
};

TEST_F(ClassTokenTest, ClassKeyword) {
    tokenizeAndCheck("class", {TokenType::CLASS});
}

TEST_F(ClassTokenTest, DefKeyword) {
    tokenizeAndCheck("def", {TokenType::DEF});
}

TEST_F(ClassTokenTest, SelfKeyword) {
    tokenizeAndCheck("self", {TokenType::SELF});
}

TEST_F(ClassTokenTest, InitMethod) {
    tokenizeAndCheck("__init__", {TokenType::IDENTIFIER}); // Special identifier
}

TEST_F(ClassTokenTest, GenericBrackets) {
    tokenizeAndCheck("[T]", {
        TokenType::LEFT_BRACKET,
        TokenType::IDENTIFIER,
        TokenType::RIGHT_BRACKET
    });
}

TEST_F(ClassTokenTest, BasicClassDefinition) {
    std::string input = R"(
class Person:
    name: string
    age: int

    def __init__(name: string, age: int):
        self.name = name
)";

    std::vector<TokenType> expected = {
        TokenType::CLASS,
        TokenType::IDENTIFIER,     // Person
        TokenType::COLON,
        TokenType::NEWLINE,
        TokenType::INDENT,
        TokenType::IDENTIFIER,     // name
        TokenType::COLON,
        TokenType::IDENTIFIER,     // string
        TokenType::NEWLINE,
        TokenType::IDENTIFIER,     // age
        TokenType::COLON,
        TokenType::IDENTIFIER,     // int
        TokenType::NEWLINE,
        TokenType::NEWLINE,
        TokenType::DEF,
        TokenType::IDENTIFIER,     // __init__
        TokenType::LEFT_PAREN,
        TokenType::IDENTIFIER,     // name
        TokenType::COLON,
        TokenType::IDENTIFIER,     // string
        TokenType::COMMA,
        TokenType::IDENTIFIER,     // age
        TokenType::COLON,
        TokenType::IDENTIFIER,     // int
        TokenType::RIGHT_PAREN,
        TokenType::COLON,
        TokenType::NEWLINE,
        TokenType::INDENT,
        TokenType::SELF,
        TokenType::DOT,
        TokenType::IDENTIFIER,     // name
        TokenType::ASSIGN,
        TokenType::IDENTIFIER,     // name
        TokenType::NEWLINE,
        TokenType::DEDENT,
        TokenType::DEDENT
    };

    tokenizeAndCheck(input, expected);
}

TEST_F(ClassTokenTest, GenericClassDefinition) {
    std::string input = "class Array[T]:";

    std::vector<TokenType> expected = {
        TokenType::CLASS,
        TokenType::IDENTIFIER,     // Array
        TokenType::LEFT_BRACKET,
        TokenType::IDENTIFIER,     // T
        TokenType::RIGHT_BRACKET,
        TokenType::COLON
    };

    tokenizeAndCheck(input, expected);
}

TEST_F(ClassTokenTest, MultipleGenerics) {
    std::string input = "class Map[K, V]:";

    std::vector<TokenType> expected = {
        TokenType::CLASS,
        TokenType::IDENTIFIER,     // Map
        TokenType::LEFT_BRACKET,
        TokenType::IDENTIFIER,     // K
        TokenType::COMMA,
        TokenType::IDENTIFIER,     // V
        TokenType::RIGHT_BRACKET,
        TokenType::COLON
    };

    tokenizeAndCheck(input, expected);
}

TEST_F(ClassTokenTest, ObjectInstantiation) {
    std::string input = "person: Person = Person(\"Alice\", 25)";

    std::vector<TokenType> expected = {
        TokenType::IDENTIFIER,     // person
        TokenType::COLON,
        TokenType::IDENTIFIER,     // Person
        TokenType::ASSIGN,
        TokenType::IDENTIFIER,     // Person
        TokenType::LEFT_PAREN,
        TokenType::STRING,         // "Alice"
        TokenType::COMMA,
        TokenType::NUMBER,         // 25
        TokenType::RIGHT_PAREN
    };

    tokenizeAndCheck(input, expected);
}

TEST_F(ClassTokenTest, GenericInstantiation) {
    std::string input = "arr: Array[int] = Array[int](10)";

    std::vector<TokenType> expected = {
        TokenType::IDENTIFIER,     // arr
        TokenType::COLON,
        TokenType::IDENTIFIER,     // Array
        TokenType::LEFT_BRACKET,
        TokenType::IDENTIFIER,     // int
        TokenType::RIGHT_BRACKET,
        TokenType::ASSIGN,
        TokenType::IDENTIFIER,     // Array
        TokenType::LEFT_BRACKET,
        TokenType::IDENTIFIER,     // int
        TokenType::RIGHT_BRACKET,
        TokenType::LEFT_PAREN,
        TokenType::NUMBER,         // 10
        TokenType::RIGHT_PAREN
    };

    tokenizeAndCheck(input, expected);
}

TEST_F(ClassTokenTest, MethodCall) {
    std::string input = "person.greet()";

    std::vector<TokenType> expected = {
        TokenType::IDENTIFIER,     // person
        TokenType::DOT,
        TokenType::IDENTIFIER,     // greet
        TokenType::LEFT_PAREN,
        TokenType::RIGHT_PAREN
    };

    tokenizeAndCheck(input, expected);
}

TEST_F(ClassTokenTest, FieldAccess) {
    std::string input = "person.name";

    std::vector<TokenType> expected = {
        TokenType::IDENTIFIER,     // person
        TokenType::DOT,
        TokenType::IDENTIFIER      // name
    };

    tokenizeAndCheck(input, expected);
}