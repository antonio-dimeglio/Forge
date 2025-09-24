#include <gtest/gtest.h>
#include "../../include/parser/Parser.hpp"
#include "../../include/parser/Statement.hpp"
#include "../../include/lexer/Tokenizer.hpp"
#include "../../include/parser/ParserException.hpp"
#include <memory>
#include <vector>

// Use the external helper functions from test_parser.cpp
extern std::unique_ptr<Statement> parseStatement(const std::string& input);
extern std::unique_ptr<Expression> parseExpression(const std::string& input);

class ClassParsingTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// ============= CLASS PARSING TESTS =============

TEST_F(ClassParsingTest, BasicClassDefinition) {
    // Note: This test will need ClassDefinition AST node to be implemented first
    std::string input = "class Person { }";

    // For now, expect parsing to succeed when ClassDefinition is implemented
    // This is a placeholder test structure
    EXPECT_NO_THROW({
        auto stmt = parseStatement(input);
        // When ClassDefinition is implemented, add proper assertions here
    });
}

TEST_F(ClassParsingTest, GenericClassDefinition) {
    // Note: This test will need generic support in ClassDefinition
    std::string input = "class Array[T] { }";

    EXPECT_NO_THROW({
        auto stmt = parseStatement(input);
        // When generic ClassDefinition is implemented, add proper assertions here
    });
}

TEST_F(ClassParsingTest, MultipleGenerics) {
    // Note: This test will need multiple generic parameter support
    std::string input = "class Map[K, V] { }";

    EXPECT_NO_THROW({
        auto stmt = parseStatement(input);
        // When multiple generic parameters are implemented, add assertions here
    });
}

TEST_F(ClassParsingTest, ObjectInstantiation) {
    // Note: This might be parsed as FunctionCall initially
    std::string input = "Person(\"Alice\", 25)";

    auto expr = parseExpression(input);
    ASSERT_NE(expr, nullptr);

    // Should be parsed as FunctionCall for now
    auto funcCallPtr = dynamic_cast<const FunctionCall*>(expr.get());
    EXPECT_NE(funcCallPtr, nullptr);
    if (funcCallPtr) {
        EXPECT_EQ(funcCallPtr->functionName, "Person");
        EXPECT_EQ(funcCallPtr->arguments.size(), 2);
    }
}

TEST_F(ClassParsingTest, GenericInstantiation) {
    // Note: This will need special parsing for generic instantiation
    std::string input = "Array[int](10)";

    // For now, this might not parse correctly until generic instantiation is implemented
    EXPECT_NO_THROW({
        auto expr = parseExpression(input);
        // When GenericInstantiation is implemented, add proper assertions here
    });
}

TEST_F(ClassParsingTest, FieldAccess) {
    // Note: This should work with existing BinaryExpression or MemberAccess
    std::string input = "person.name";

    auto expr = parseExpression(input);
    ASSERT_NE(expr, nullptr);

    // This might be parsed as a different expression type initially
    // TODO: Check what expression type is used for member access
}

TEST_F(ClassParsingTest, MethodCall) {
    // Note: This should work with existing parsing for member access + function call
    std::string input = "person.greet()";

    auto expr = parseExpression(input);
    ASSERT_NE(expr, nullptr);

    // This might be parsed as a function call on a member access
    // TODO: Check what expression structure is used for method calls
}

TEST_F(ClassParsingTest, ChainedCalls) {
    // Note: This tests chained method calls
    std::string input = "arr.get(0).toString()";

    EXPECT_NO_THROW({
        auto expr = parseExpression(input);
        ASSERT_NE(expr, nullptr);
        // TODO: Check chained method call structure when implemented
    });
}