#include <gtest/gtest.h>
#include "../../include/parser/Parser.hpp"
#include "../../include/lexer/Tokenizer.hpp"
#include <memory>
#include <vector>

// Helper function to parse expression from string
std::unique_ptr<Expression> parseExpression(const std::string& input) {
    Tokenizer tokenizer(input);
    std::vector<Token> tokens = tokenizer.tokenize();
    Parser parser(tokens);
    return parser.generateAST();
}

// Helper function to get tokens for manual testing
std::vector<Token> getTokens(const std::string& input) {
    Tokenizer tokenizer(input);
    return tokenizer.tokenize();
}

class ParserTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// ===== BASIC LITERAL TESTS =====

TEST_F(ParserTest, ParseNumberLiteral) {
    auto expr = parseExpression("42");

    ASSERT_NE(expr, nullptr);
    auto literal = dynamic_cast<LiteralExpression*>(expr.get());
    ASSERT_NE(literal, nullptr);
    EXPECT_EQ(literal->value.getType(), TokenType::NUMBER);
    EXPECT_EQ(literal->value.getValue(), "42");
}

TEST_F(ParserTest, ParseFloatLiteral) {
    auto expr = parseExpression("3.14");

    ASSERT_NE(expr, nullptr);
    auto literal = dynamic_cast<LiteralExpression*>(expr.get());
    ASSERT_NE(literal, nullptr);
    EXPECT_EQ(literal->value.getType(), TokenType::NUMBER);
    EXPECT_EQ(literal->value.getValue(), "3.14");
}

TEST_F(ParserTest, ParseStringLiteral) {
    auto expr = parseExpression("\"hello world\"");

    ASSERT_NE(expr, nullptr);
    auto literal = dynamic_cast<LiteralExpression*>(expr.get());
    ASSERT_NE(literal, nullptr);
    EXPECT_EQ(literal->value.getType(), TokenType::STRING);
    EXPECT_EQ(literal->value.getValue(), "hello world");
}

TEST_F(ParserTest, ParseEmptyString) {
    auto expr = parseExpression("\"\"");

    ASSERT_NE(expr, nullptr);
    auto literal = dynamic_cast<LiteralExpression*>(expr.get());
    ASSERT_NE(literal, nullptr);
    EXPECT_EQ(literal->value.getType(), TokenType::STRING);
    EXPECT_EQ(literal->value.getValue(), "");
}

TEST_F(ParserTest, ParseTrueLiteral) {
    auto expr = parseExpression("true");

    ASSERT_NE(expr, nullptr);
    auto literal = dynamic_cast<LiteralExpression*>(expr.get());
    ASSERT_NE(literal, nullptr);
    EXPECT_EQ(literal->value.getType(), TokenType::TRUE);
    EXPECT_EQ(literal->value.getValue(), "true");
}

TEST_F(ParserTest, ParseFalseLiteral) {
    auto expr = parseExpression("false");

    ASSERT_NE(expr, nullptr);
    auto literal = dynamic_cast<LiteralExpression*>(expr.get());
    ASSERT_NE(literal, nullptr);
    EXPECT_EQ(literal->value.getType(), TokenType::FALSE);
    EXPECT_EQ(literal->value.getValue(), "false");
}

// ===== IDENTIFIER TESTS =====

TEST_F(ParserTest, ParseSimpleIdentifier) {
    auto expr = parseExpression("myVar");

    ASSERT_NE(expr, nullptr);
    auto identifier = dynamic_cast<IdentifierExpression*>(expr.get());
    ASSERT_NE(identifier, nullptr);
    EXPECT_EQ(identifier->name, "myVar");
}

TEST_F(ParserTest, ParseIdentifierWithUnderscore) {
    auto expr = parseExpression("_private_var");

    ASSERT_NE(expr, nullptr);
    auto identifier = dynamic_cast<IdentifierExpression*>(expr.get());
    ASSERT_NE(identifier, nullptr);
    EXPECT_EQ(identifier->name, "_private_var");
}

TEST_F(ParserTest, ParseIdentifierWithNumbers) {
    auto expr = parseExpression("var123");

    ASSERT_NE(expr, nullptr);
    auto identifier = dynamic_cast<IdentifierExpression*>(expr.get());
    ASSERT_NE(identifier, nullptr);
    EXPECT_EQ(identifier->name, "var123");
}

// ===== UNARY EXPRESSION TESTS =====

TEST_F(ParserTest, ParseNegativeNumber) {
    auto expr = parseExpression("-42");

    ASSERT_NE(expr, nullptr);
    auto unary = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_NE(unary, nullptr);
    EXPECT_EQ(unary->operator_.getType(), TokenType::MINUS);

    auto operand = dynamic_cast<LiteralExpression*>(unary->operand.get());
    ASSERT_NE(operand, nullptr);
    EXPECT_EQ(operand->value.getValue(), "42");
}

TEST_F(ParserTest, ParseNotExpression) {
    auto expr = parseExpression("!true");

    ASSERT_NE(expr, nullptr);
    auto unary = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_NE(unary, nullptr);
    EXPECT_EQ(unary->operator_.getType(), TokenType::NOT);

    auto operand = dynamic_cast<LiteralExpression*>(unary->operand.get());
    ASSERT_NE(operand, nullptr);
    EXPECT_EQ(operand->value.getType(), TokenType::TRUE);
}

TEST_F(ParserTest, ParseDoubleNegative) {
    auto expr = parseExpression("--5");

    ASSERT_NE(expr, nullptr);
    auto outer_unary = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_NE(outer_unary, nullptr);
    EXPECT_EQ(outer_unary->operator_.getType(), TokenType::MINUS);

    auto inner_unary = dynamic_cast<UnaryExpression*>(outer_unary->operand.get());
    ASSERT_NE(inner_unary, nullptr);
    EXPECT_EQ(inner_unary->operator_.getType(), TokenType::MINUS);

    auto literal = dynamic_cast<LiteralExpression*>(inner_unary->operand.get());
    ASSERT_NE(literal, nullptr);
    EXPECT_EQ(literal->value.getValue(), "5");
}

TEST_F(ParserTest, ParseDoubleNot) {
    auto expr = parseExpression("!!flag");

    ASSERT_NE(expr, nullptr);
    auto outer_unary = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_NE(outer_unary, nullptr);
    EXPECT_EQ(outer_unary->operator_.getType(), TokenType::NOT);

    auto inner_unary = dynamic_cast<UnaryExpression*>(outer_unary->operand.get());
    ASSERT_NE(inner_unary, nullptr);
    EXPECT_EQ(inner_unary->operator_.getType(), TokenType::NOT);

    auto identifier = dynamic_cast<IdentifierExpression*>(inner_unary->operand.get());
    ASSERT_NE(identifier, nullptr);
    EXPECT_EQ(identifier->name, "flag");
}

TEST_F(ParserTest, ParseMixedUnaryOperators) {
    auto expr = parseExpression("-!true");

    ASSERT_NE(expr, nullptr);
    auto outer_unary = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_NE(outer_unary, nullptr);
    EXPECT_EQ(outer_unary->operator_.getType(), TokenType::MINUS);

    auto inner_unary = dynamic_cast<UnaryExpression*>(outer_unary->operand.get());
    ASSERT_NE(inner_unary, nullptr);
    EXPECT_EQ(inner_unary->operator_.getType(), TokenType::NOT);

    auto literal = dynamic_cast<LiteralExpression*>(inner_unary->operand.get());
    ASSERT_NE(literal, nullptr);
    EXPECT_EQ(literal->value.getType(), TokenType::TRUE);
}

// ===== BINARY EXPRESSION TESTS =====

TEST_F(ParserTest, ParseSimpleAddition) {
    auto expr = parseExpression("2 + 3");

    ASSERT_NE(expr, nullptr);
    auto binary = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary, nullptr);
    EXPECT_EQ(binary->operator_.getType(), TokenType::PLUS);

    auto left = dynamic_cast<LiteralExpression*>(binary->left.get());
    ASSERT_NE(left, nullptr);
    EXPECT_EQ(left->value.getValue(), "2");

    auto right = dynamic_cast<LiteralExpression*>(binary->right.get());
    ASSERT_NE(right, nullptr);
    EXPECT_EQ(right->value.getValue(), "3");
}

TEST_F(ParserTest, ParseSimpleSubtraction) {
    auto expr = parseExpression("10 - 5");

    ASSERT_NE(expr, nullptr);
    auto binary = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary, nullptr);
    EXPECT_EQ(binary->operator_.getType(), TokenType::MINUS);
}

TEST_F(ParserTest, ParseSimpleMultiplication) {
    auto expr = parseExpression("3 * 4");

    ASSERT_NE(expr, nullptr);
    auto binary = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary, nullptr);
    EXPECT_EQ(binary->operator_.getType(), TokenType::MULT);
}

TEST_F(ParserTest, ParseSimpleDivision) {
    auto expr = parseExpression("8 / 2");

    ASSERT_NE(expr, nullptr);
    auto binary = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary, nullptr);
    EXPECT_EQ(binary->operator_.getType(), TokenType::DIV);
}

// ===== OPERATOR PRECEDENCE TESTS =====

TEST_F(ParserTest, ParseMultiplicationPrecedence) {
    // 2 + 3 * 4 should be parsed as 2 + (3 * 4)
    auto expr = parseExpression("2 + 3 * 4");

    ASSERT_NE(expr, nullptr);
    auto addition = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(addition, nullptr);
    EXPECT_EQ(addition->operator_.getType(), TokenType::PLUS);

    // Left side should be literal 2
    auto left = dynamic_cast<LiteralExpression*>(addition->left.get());
    ASSERT_NE(left, nullptr);
    EXPECT_EQ(left->value.getValue(), "2");

    // Right side should be multiplication 3 * 4
    auto multiplication = dynamic_cast<BinaryExpression*>(addition->right.get());
    ASSERT_NE(multiplication, nullptr);
    EXPECT_EQ(multiplication->operator_.getType(), TokenType::MULT);

    auto mult_left = dynamic_cast<LiteralExpression*>(multiplication->left.get());
    ASSERT_NE(mult_left, nullptr);
    EXPECT_EQ(mult_left->value.getValue(), "3");

    auto mult_right = dynamic_cast<LiteralExpression*>(multiplication->right.get());
    ASSERT_NE(mult_right, nullptr);
    EXPECT_EQ(mult_right->value.getValue(), "4");
}

TEST_F(ParserTest, ParseDivisionPrecedence) {
    // 10 - 8 / 2 should be parsed as 10 - (8 / 2)
    auto expr = parseExpression("10 - 8 / 2");

    ASSERT_NE(expr, nullptr);
    auto subtraction = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(subtraction, nullptr);
    EXPECT_EQ(subtraction->operator_.getType(), TokenType::MINUS);

    // Right side should be division 8 / 2
    auto division = dynamic_cast<BinaryExpression*>(subtraction->right.get());
    ASSERT_NE(division, nullptr);
    EXPECT_EQ(division->operator_.getType(), TokenType::DIV);
}

TEST_F(ParserTest, ParseComparisonPrecedence) {
    // a + b == c * d should be parsed as (a + b) == (c * d)
    auto expr = parseExpression("a + b == c * d");

    ASSERT_NE(expr, nullptr);
    auto equality = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(equality, nullptr);
    EXPECT_EQ(equality->operator_.getType(), TokenType::EQUAL_EQUAL);

    // Left side should be addition a + b
    auto left_addition = dynamic_cast<BinaryExpression*>(equality->left.get());
    ASSERT_NE(left_addition, nullptr);
    EXPECT_EQ(left_addition->operator_.getType(), TokenType::PLUS);

    // Right side should be multiplication c * d
    auto right_multiplication = dynamic_cast<BinaryExpression*>(equality->right.get());
    ASSERT_NE(right_multiplication, nullptr);
    EXPECT_EQ(right_multiplication->operator_.getType(), TokenType::MULT);
}

// ===== LEFT ASSOCIATIVITY TESTS =====

TEST_F(ParserTest, ParseLeftAssociativeAddition) {
    // 1 + 2 + 3 should be parsed as ((1 + 2) + 3)
    auto expr = parseExpression("1 + 2 + 3");

    ASSERT_NE(expr, nullptr);
    auto outer_addition = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(outer_addition, nullptr);
    EXPECT_EQ(outer_addition->operator_.getType(), TokenType::PLUS);

    // Left side should be another addition (1 + 2)
    auto inner_addition = dynamic_cast<BinaryExpression*>(outer_addition->left.get());
    ASSERT_NE(inner_addition, nullptr);
    EXPECT_EQ(inner_addition->operator_.getType(), TokenType::PLUS);

    // Right side should be literal 3
    auto right = dynamic_cast<LiteralExpression*>(outer_addition->right.get());
    ASSERT_NE(right, nullptr);
    EXPECT_EQ(right->value.getValue(), "3");
}

TEST_F(ParserTest, ParseLeftAssociativeMultiplication) {
    // 2 * 3 * 4 should be parsed as ((2 * 3) * 4)
    auto expr = parseExpression("2 * 3 * 4");

    ASSERT_NE(expr, nullptr);
    auto outer_mult = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(outer_mult, nullptr);
    EXPECT_EQ(outer_mult->operator_.getType(), TokenType::MULT);

    // Left side should be another multiplication (2 * 3)
    auto inner_mult = dynamic_cast<BinaryExpression*>(outer_mult->left.get());
    ASSERT_NE(inner_mult, nullptr);
    EXPECT_EQ(inner_mult->operator_.getType(), TokenType::MULT);
}

TEST_F(ParserTest, ParseMixedLeftAssociativity) {
    // 8 / 2 / 2 should be parsed as ((8 / 2) / 2)
    auto expr = parseExpression("8 / 2 / 2");

    ASSERT_NE(expr, nullptr);
    auto outer_div = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(outer_div, nullptr);
    EXPECT_EQ(outer_div->operator_.getType(), TokenType::DIV);

    auto inner_div = dynamic_cast<BinaryExpression*>(outer_div->left.get());
    ASSERT_NE(inner_div, nullptr);
    EXPECT_EQ(inner_div->operator_.getType(), TokenType::DIV);
}

// ===== PARENTHESES TESTS =====

TEST_F(ParserTest, ParseSimpleParentheses) {
    auto expr = parseExpression("(42)");

    ASSERT_NE(expr, nullptr);
    auto literal = dynamic_cast<LiteralExpression*>(expr.get());
    ASSERT_NE(literal, nullptr);
    EXPECT_EQ(literal->value.getValue(), "42");
}

TEST_F(ParserTest, ParseParenthesesOverridePrecedence) {
    // (2 + 3) * 4 should be parsed as (2 + 3) * 4, not 2 + (3 * 4)
    auto expr = parseExpression("(2 + 3) * 4");

    ASSERT_NE(expr, nullptr);
    auto multiplication = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(multiplication, nullptr);
    EXPECT_EQ(multiplication->operator_.getType(), TokenType::MULT);

    // Left side should be addition (2 + 3)
    auto addition = dynamic_cast<BinaryExpression*>(multiplication->left.get());
    ASSERT_NE(addition, nullptr);
    EXPECT_EQ(addition->operator_.getType(), TokenType::PLUS);

    // Right side should be literal 4
    auto right = dynamic_cast<LiteralExpression*>(multiplication->right.get());
    ASSERT_NE(right, nullptr);
    EXPECT_EQ(right->value.getValue(), "4");
}

TEST_F(ParserTest, ParseNestedParentheses) {
    auto expr = parseExpression("((1 + 2) * 3) / 4");

    ASSERT_NE(expr, nullptr);
    auto division = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(division, nullptr);
    EXPECT_EQ(division->operator_.getType(), TokenType::DIV);

    // Left side should be multiplication
    auto multiplication = dynamic_cast<BinaryExpression*>(division->left.get());
    ASSERT_NE(multiplication, nullptr);
    EXPECT_EQ(multiplication->operator_.getType(), TokenType::MULT);

    // Left side of multiplication should be addition
    auto addition = dynamic_cast<BinaryExpression*>(multiplication->left.get());
    ASSERT_NE(addition, nullptr);
    EXPECT_EQ(addition->operator_.getType(), TokenType::PLUS);
}

// ===== COMPARISON OPERATOR TESTS =====

TEST_F(ParserTest, ParseEqualityOperators) {
    auto expr1 = parseExpression("a == b");
    auto binary1 = dynamic_cast<BinaryExpression*>(expr1.get());
    ASSERT_NE(binary1, nullptr);
    EXPECT_EQ(binary1->operator_.getType(), TokenType::EQUAL_EQUAL);

    auto expr2 = parseExpression("x != y");
    auto binary2 = dynamic_cast<BinaryExpression*>(expr2.get());
    ASSERT_NE(binary2, nullptr);
    EXPECT_EQ(binary2->operator_.getType(), TokenType::NOT_EQUAL);
}

TEST_F(ParserTest, ParseComparisonOperators) {
    auto expr1 = parseExpression("a < b");
    auto binary1 = dynamic_cast<BinaryExpression*>(expr1.get());
    ASSERT_NE(binary1, nullptr);
    EXPECT_EQ(binary1->operator_.getType(), TokenType::LESS);

    auto expr2 = parseExpression("a > b");
    auto binary2 = dynamic_cast<BinaryExpression*>(expr2.get());
    ASSERT_NE(binary2, nullptr);
    EXPECT_EQ(binary2->operator_.getType(), TokenType::GREATER);

    auto expr3 = parseExpression("a <= b");
    auto binary3 = dynamic_cast<BinaryExpression*>(expr3.get());
    ASSERT_NE(binary3, nullptr);
    EXPECT_EQ(binary3->operator_.getType(), TokenType::LEQ);

    auto expr4 = parseExpression("a >= b");
    auto binary4 = dynamic_cast<BinaryExpression*>(expr4.get());
    ASSERT_NE(binary4, nullptr);
    EXPECT_EQ(binary4->operator_.getType(), TokenType::GEQ);
}

// ===== COMPLEX EXPRESSION TESTS =====

TEST_F(ParserTest, ParseComplexExpression1) {
    // -a + b * c / d == e
    auto expr = parseExpression("-a + b * c / d == e");

    ASSERT_NE(expr, nullptr);
    auto equality = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(equality, nullptr);
    EXPECT_EQ(equality->operator_.getType(), TokenType::EQUAL_EQUAL);

    // Left side should be addition
    auto addition = dynamic_cast<BinaryExpression*>(equality->left.get());
    ASSERT_NE(addition, nullptr);
    EXPECT_EQ(addition->operator_.getType(), TokenType::PLUS);

    // Left side of addition should be unary negation
    auto negation = dynamic_cast<UnaryExpression*>(addition->left.get());
    ASSERT_NE(negation, nullptr);
    EXPECT_EQ(negation->operator_.getType(), TokenType::MINUS);
}

TEST_F(ParserTest, ParseComplexExpression2) {
    // !flag && (x + y) * 2 > threshold
    auto expr = parseExpression("!flag > threshold");

    ASSERT_NE(expr, nullptr);
    auto comparison = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(comparison, nullptr);
    EXPECT_EQ(comparison->operator_.getType(), TokenType::GREATER);

    // Left side should be unary not
    auto negation = dynamic_cast<UnaryExpression*>(comparison->left.get());
    ASSERT_NE(negation, nullptr);
    EXPECT_EQ(negation->operator_.getType(), TokenType::NOT);
}

TEST_F(ParserTest, ParseVeryComplexExpression) {
    // ((-a + b) * (c - d)) / ((e + f) - g)
    auto expr = parseExpression("((-a + b) * (c - d)) / ((e + f) - g)");

    ASSERT_NE(expr, nullptr);
    auto division = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(division, nullptr);
    EXPECT_EQ(division->operator_.getType(), TokenType::DIV);

    // Both sides should be complex expressions
    ASSERT_NE(division->left, nullptr);
    ASSERT_NE(division->right, nullptr);
}

// ===== WHITESPACE HANDLING TESTS =====

TEST_F(ParserTest, ParseWithExtraWhitespace) {
    auto expr = parseExpression("  2   +   3  ");

    ASSERT_NE(expr, nullptr);
    auto binary = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary, nullptr);
    EXPECT_EQ(binary->operator_.getType(), TokenType::PLUS);
}

TEST_F(ParserTest, ParseWithNoWhitespace) {
    auto expr = parseExpression("2+3*4");

    ASSERT_NE(expr, nullptr);
    auto addition = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(addition, nullptr);
    EXPECT_EQ(addition->operator_.getType(), TokenType::PLUS);

    // Right side should be multiplication
    auto multiplication = dynamic_cast<BinaryExpression*>(addition->right.get());
    ASSERT_NE(multiplication, nullptr);
    EXPECT_EQ(multiplication->operator_.getType(), TokenType::MULT);
}

// ===== EDGE CASE TESTS =====

TEST_F(ParserTest, ParseSingleToken) {
    auto expr = parseExpression("x");

    ASSERT_NE(expr, nullptr);
    auto identifier = dynamic_cast<IdentifierExpression*>(expr.get());
    ASSERT_NE(identifier, nullptr);
    EXPECT_EQ(identifier->name, "x");
}

TEST_F(ParserTest, ParseLongChain) {
    // Test a long chain of operations
    auto expr = parseExpression("1 + 2 + 3 + 4 + 5");

    ASSERT_NE(expr, nullptr);
    auto binary = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary, nullptr);

    // Should be left-associative, so rightmost should be literal 5
    auto right = dynamic_cast<LiteralExpression*>(binary->right.get());
    ASSERT_NE(right, nullptr);
    EXPECT_EQ(right->value.getValue(), "5");
}

TEST_F(ParserTest, ParseDeepNesting) {
    auto expr = parseExpression("((((1))))");

    ASSERT_NE(expr, nullptr);
    auto literal = dynamic_cast<LiteralExpression*>(expr.get());
    ASSERT_NE(literal, nullptr);
    EXPECT_EQ(literal->value.getValue(), "1");
}

// ===== ERROR HANDLING TESTS =====
// Note: These tests expect exceptions to be thrown

TEST_F(ParserTest, ParseUnmatchedLeftParen) {
    EXPECT_THROW(parseExpression("(2 + 3"), std::exception);
}

TEST_F(ParserTest, ParseUnmatchedRightParen) {
    EXPECT_THROW(parseExpression("2 + 3)"), std::exception);
}

TEST_F(ParserTest, ParseEmptyParentheses) {
    EXPECT_THROW(parseExpression("()"), std::exception);
}

TEST_F(ParserTest, ParseMissingOperand) {
    EXPECT_THROW(parseExpression("2 +"), std::exception);
    EXPECT_THROW(parseExpression("+ 2"), std::exception);
}

TEST_F(ParserTest, ParseInvalidTokenSequence) {
    EXPECT_THROW(parseExpression("2 3"), std::exception);
    EXPECT_THROW(parseExpression("+ +"), std::exception);
}

// ===== UNARY EDGE CASES =====

TEST_F(ParserTest, ParseUnaryWithParentheses) {
    auto expr = parseExpression("-(2 + 3)");

    ASSERT_NE(expr, nullptr);
    auto unary = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_NE(unary, nullptr);
    EXPECT_EQ(unary->operator_.getType(), TokenType::MINUS);

    // Operand should be addition
    auto addition = dynamic_cast<BinaryExpression*>(unary->operand.get());
    ASSERT_NE(addition, nullptr);
    EXPECT_EQ(addition->operator_.getType(), TokenType::PLUS);
}

TEST_F(ParserTest, ParseComplexUnaryChain) {
    auto expr = parseExpression("---x");

    ASSERT_NE(expr, nullptr);
    auto outer = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_NE(outer, nullptr);
    EXPECT_EQ(outer->operator_.getType(), TokenType::MINUS);

    auto middle = dynamic_cast<UnaryExpression*>(outer->operand.get());
    ASSERT_NE(middle, nullptr);
    EXPECT_EQ(middle->operator_.getType(), TokenType::MINUS);

    auto inner = dynamic_cast<UnaryExpression*>(middle->operand.get());
    ASSERT_NE(inner, nullptr);
    EXPECT_EQ(inner->operator_.getType(), TokenType::MINUS);

    auto identifier = dynamic_cast<IdentifierExpression*>(inner->operand.get());
    ASSERT_NE(identifier, nullptr);
    EXPECT_EQ(identifier->name, "x");
}

// ===== COMPREHENSIVE PRECEDENCE TESTS =====

TEST_F(ParserTest, ParseAllOperatorPrecedence) {
    // Test multiple precedence levels: a == b + c * d
    auto expr = parseExpression("a == b + c * d");

    ASSERT_NE(expr, nullptr);
    auto equality = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(equality, nullptr);
    EXPECT_EQ(equality->operator_.getType(), TokenType::EQUAL_EQUAL);

    // Right side should be addition b + (c * d)
    auto addition = dynamic_cast<BinaryExpression*>(equality->right.get());
    ASSERT_NE(addition, nullptr);
    EXPECT_EQ(addition->operator_.getType(), TokenType::PLUS);

    // Right side of addition should be multiplication c * d
    auto multiplication = dynamic_cast<BinaryExpression*>(addition->right.get());
    ASSERT_NE(multiplication, nullptr);
    EXPECT_EQ(multiplication->operator_.getType(), TokenType::MULT);
}