#include <gtest/gtest.h>
#include "../../include/parser/Parser.hpp"
#include "../../include/parser/Statement.hpp"
#include "../../include/lexer/Tokenizer.hpp"
#include "../../include/parser/ParserException.hpp"
#include <memory>
#include <vector>

// Helper function to parse statement from string
std::unique_ptr<Statement> parseStatement(const std::string& input) {
    Tokenizer tokenizer(input);
    std::vector<Token> tokens = tokenizer.tokenize();
    Parser parser(tokens);
    return parser.parseProgram();
}

// Helper function for backward compatibility with expression tests
Expression* parseExpressionRaw(const std::string& input) {
    auto stmt = parseStatement(input);
    auto program = dynamic_cast<Program*>(stmt.get());
    if (!program || program->statements.empty()) return nullptr;

    auto exprStmt = dynamic_cast<ExpressionStatement*>(program->statements[0].get());
    if (!exprStmt) return nullptr;

    return exprStmt->expression.get();
}

// For tests that expect unique_ptr
std::unique_ptr<Expression> parseExpression(const std::string& input) {
    // Parse the statement and extract the expression safely
    auto stmt = parseStatement(input);
    auto program = dynamic_cast<Program*>(stmt.get());
    if (!program || program->statements.empty()) return nullptr;

    // For single expression tests, we should have exactly one statement
    if (program->statements.size() != 1) {
        throw std::runtime_error("Expected single expression, got multiple statements");
    }

    auto exprStmt = dynamic_cast<ExpressionStatement*>(program->statements[0].get());
    if (!exprStmt) return nullptr;

    // Move the expression out of the statement (transfer ownership)
    return std::move(exprStmt->expression);
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

// ================================
// ===== STATEMENT PARSING TESTS =====
// ================================

class StatementParserTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// ===== VARIABLE DECLARATION TESTS =====

TEST_F(StatementParserTest, ParseSimpleVariableDeclaration) {
    auto stmt = parseStatement("x: int = 42");
    ASSERT_NE(stmt, nullptr);

    auto program = dynamic_cast<Program*>(stmt.get());
    ASSERT_NE(program, nullptr);
    ASSERT_EQ(program->statements.size(), 1);

    auto varDecl = dynamic_cast<VariableDeclaration*>(program->statements[0].get());
    ASSERT_NE(varDecl, nullptr);
    EXPECT_EQ(varDecl->variable.getValue(), "x");
    EXPECT_EQ(varDecl->type.getType(), TokenType::INT);

    auto expr = dynamic_cast<LiteralExpression*>(varDecl->expr.get());
    ASSERT_NE(expr, nullptr);
    EXPECT_EQ(expr->value.getValue(), "42");
}

TEST_F(StatementParserTest, ParseVariableDeclarationAllTypes) {
    // Test all supported types
    auto stmtInt = parseStatement("x: int = 42");
    auto programInt = dynamic_cast<Program*>(stmtInt.get());
    auto varDeclInt = dynamic_cast<VariableDeclaration*>(programInt->statements[0].get());
    EXPECT_EQ(varDeclInt->type.getType(), TokenType::INT);

    auto stmtFloat = parseStatement("y: float = 3.14");
    auto programFloat = dynamic_cast<Program*>(stmtFloat.get());
    auto varDeclFloat = dynamic_cast<VariableDeclaration*>(programFloat->statements[0].get());
    EXPECT_EQ(varDeclFloat->type.getType(), TokenType::FLOAT);

    auto stmtDouble = parseStatement("z: double = 2.718");
    auto programDouble = dynamic_cast<Program*>(stmtDouble.get());
    auto varDeclDouble = dynamic_cast<VariableDeclaration*>(programDouble->statements[0].get());
    EXPECT_EQ(varDeclDouble->type.getType(), TokenType::DOUBLE);

    auto stmtBool = parseStatement("flag: bool = true");
    auto programBool = dynamic_cast<Program*>(stmtBool.get());
    auto varDeclBool = dynamic_cast<VariableDeclaration*>(programBool->statements[0].get());
    EXPECT_EQ(varDeclBool->type.getType(), TokenType::BOOL);

    auto stmtStr = parseStatement("name: str = \"hello\"");
    auto programStr = dynamic_cast<Program*>(stmtStr.get());
    auto varDeclStr = dynamic_cast<VariableDeclaration*>(programStr->statements[0].get());
    EXPECT_EQ(varDeclStr->type.getType(), TokenType::STR);
}

TEST_F(StatementParserTest, ParseVariableDeclarationWithComplexExpression) {
    auto stmt = parseStatement("result: int = 2 + 3 * 4");
    auto program = dynamic_cast<Program*>(stmt.get());
    auto varDecl = dynamic_cast<VariableDeclaration*>(program->statements[0].get());

    ASSERT_NE(varDecl, nullptr);
    EXPECT_EQ(varDecl->variable.getValue(), "result");

    auto binaryExpr = dynamic_cast<BinaryExpression*>(varDecl->expr.get());
    ASSERT_NE(binaryExpr, nullptr);
    EXPECT_EQ(binaryExpr->operator_.getType(), TokenType::PLUS);
}

TEST_F(StatementParserTest, ParseVariableDeclarationWithIdentifiers) {
    auto stmt = parseStatement("sum: int = x + y");
    auto program = dynamic_cast<Program*>(stmt.get());
    auto varDecl = dynamic_cast<VariableDeclaration*>(program->statements[0].get());

    auto binaryExpr = dynamic_cast<BinaryExpression*>(varDecl->expr.get());
    ASSERT_NE(binaryExpr, nullptr);

    auto leftId = dynamic_cast<IdentifierExpression*>(binaryExpr->left.get());
    auto rightId = dynamic_cast<IdentifierExpression*>(binaryExpr->right.get());
    ASSERT_NE(leftId, nullptr);
    ASSERT_NE(rightId, nullptr);
    EXPECT_EQ(leftId->name, "x");
    EXPECT_EQ(rightId->name, "y");
}

// ===== VARIABLE DECLARATION ERROR TESTS =====

TEST_F(StatementParserTest, ParseVariableDeclarationMissingColon) {
    EXPECT_THROW(parseStatement("x int = 42"), ParsingException);
}

TEST_F(StatementParserTest, ParseVariableDeclarationMissingAssign) {
    EXPECT_THROW(parseStatement("x: int 42"), ParsingException);
}

TEST_F(StatementParserTest, ParseVariableDeclarationInvalidType) {
    EXPECT_THROW(parseStatement("x: badtype = 42"), ParsingException);
}

TEST_F(StatementParserTest, ParseVariableDeclarationMissingExpression) {
    EXPECT_THROW(parseStatement("x: int ="), ParsingException);
}

// ===== ASSIGNMENT TESTS =====

TEST_F(StatementParserTest, ParseSimpleAssignment) {
    auto stmt = parseStatement("x = 100");
    auto program = dynamic_cast<Program*>(stmt.get());
    ASSERT_EQ(program->statements.size(), 1);

    auto assignment = dynamic_cast<Assignment*>(program->statements[0].get());
    ASSERT_NE(assignment, nullptr);
    EXPECT_EQ(assignment->variable.getValue(), "x");

    auto literal = dynamic_cast<LiteralExpression*>(assignment->expr.get());
    ASSERT_NE(literal, nullptr);
    EXPECT_EQ(literal->value.getValue(), "100");
}

TEST_F(StatementParserTest, ParseAssignmentWithComplexExpression) {
    auto stmt = parseStatement("result = a * 2 + b");
    auto program = dynamic_cast<Program*>(stmt.get());
    auto assignment = dynamic_cast<Assignment*>(program->statements[0].get());

    ASSERT_NE(assignment, nullptr);
    EXPECT_EQ(assignment->variable.getValue(), "result");

    auto binaryExpr = dynamic_cast<BinaryExpression*>(assignment->expr.get());
    ASSERT_NE(binaryExpr, nullptr);
    EXPECT_EQ(binaryExpr->operator_.getType(), TokenType::PLUS);
}

TEST_F(StatementParserTest, ParseAssignmentMissingEqual) {
    EXPECT_THROW(parseStatement("x 42"), ParsingException);
}

TEST_F(StatementParserTest, ParseAssignmentMissingExpression) {
    EXPECT_THROW(parseStatement("x ="), ParsingException);
}

// ===== FUNCTION CALL TESTS =====

TEST_F(StatementParserTest, ParseFunctionCallStatement) {
    auto stmt = parseStatement("test()");
    auto program = dynamic_cast<Program*>(stmt.get());
    auto exprStmt = dynamic_cast<ExpressionStatement*>(program->statements[0].get());

    ASSERT_NE(exprStmt, nullptr);
    auto funcCall = dynamic_cast<FunctionCall*>(exprStmt->expression.get());
    ASSERT_NE(funcCall, nullptr);
    EXPECT_EQ(funcCall->functionName, "test");
    EXPECT_TRUE(funcCall->arguments.empty());
}

TEST_F(StatementParserTest, ParseMultipleFunctionCalls) {
    auto stmt = parseStatement("foo()\nbar()");
    auto program = dynamic_cast<Program*>(stmt.get());
    ASSERT_EQ(program->statements.size(), 2);

    auto exprStmt1 = dynamic_cast<ExpressionStatement*>(program->statements[0].get());
    auto exprStmt2 = dynamic_cast<ExpressionStatement*>(program->statements[1].get());

    auto funcCall1 = dynamic_cast<FunctionCall*>(exprStmt1->expression.get());
    auto funcCall2 = dynamic_cast<FunctionCall*>(exprStmt2->expression.get());

    ASSERT_NE(funcCall1, nullptr);
    ASSERT_NE(funcCall2, nullptr);
    EXPECT_EQ(funcCall1->functionName, "foo");
    EXPECT_EQ(funcCall2->functionName, "bar");
}

TEST_F(StatementParserTest, ParseFunctionCallMissingCloseParen) {
    EXPECT_THROW(parseStatement("test("), ParsingException);
}

// ===== IF STATEMENT TESTS =====

TEST_F(StatementParserTest, ParseSimpleIfStatement) {
    auto stmt = parseStatement("if (x == 5) {\n    test()\n}");
    auto program = dynamic_cast<Program*>(stmt.get());
    auto ifStmt = dynamic_cast<IfStatement*>(program->statements[0].get());

    ASSERT_NE(ifStmt, nullptr);

    // Check condition
    auto condition = dynamic_cast<BinaryExpression*>(ifStmt->condition.get());
    ASSERT_NE(condition, nullptr);
    EXPECT_EQ(condition->operator_.getType(), TokenType::EQUAL_EQUAL);

    // Check body
    ASSERT_EQ(ifStmt->body.size(), 1);
    auto bodyStmt = dynamic_cast<ExpressionStatement*>(ifStmt->body[0].get());
    ASSERT_NE(bodyStmt, nullptr);

    auto funcCall = dynamic_cast<FunctionCall*>(bodyStmt->expression.get());
    ASSERT_NE(funcCall, nullptr);
    EXPECT_EQ(funcCall->functionName, "test");

    // Check no else body
    EXPECT_TRUE(ifStmt->elseBody.empty());
}

TEST_F(StatementParserTest, ParseIfWithComplexCondition) {
    auto stmt = parseStatement("if (x + y > 10 * 2) {\n    foo()\n}");
    auto program = dynamic_cast<Program*>(stmt.get());
    auto ifStmt = dynamic_cast<IfStatement*>(program->statements[0].get());

    auto condition = dynamic_cast<BinaryExpression*>(ifStmt->condition.get());
    ASSERT_NE(condition, nullptr);
    EXPECT_EQ(condition->operator_.getType(), TokenType::GREATER);
}

TEST_F(StatementParserTest, ParseIfWithVariableAssignment) {
    auto stmt = parseStatement("if (flag) {\n    x = 42\n}");
    auto program = dynamic_cast<Program*>(stmt.get());
    auto ifStmt = dynamic_cast<IfStatement*>(program->statements[0].get());

    auto bodyStmt = dynamic_cast<Assignment*>(ifStmt->body[0].get());
    ASSERT_NE(bodyStmt, nullptr);
    EXPECT_EQ(bodyStmt->variable.getValue(), "x");
}

// ===== IF STATEMENT ERROR TESTS =====

TEST_F(StatementParserTest, ParseIfMissingOpenParen) {
    EXPECT_THROW(parseStatement("if x == 5) {\n    test()\n}"), ParsingException);
}

TEST_F(StatementParserTest, ParseIfMissingCloseParen) {
    EXPECT_THROW(parseStatement("if (x == 5 {\n    test()\n}"), ParsingException);
}

TEST_F(StatementParserTest, ParseIfMissingOpenBrace) {
    EXPECT_THROW(parseStatement("if (x == 5) test()"), ParsingException);
}

TEST_F(StatementParserTest, ParseIfMissingCondition) {
    EXPECT_THROW(parseStatement("if () {\n    test()\n}"), ParsingException);
}

// ===== PROGRAM TESTS (MULTIPLE STATEMENTS) =====

TEST_F(StatementParserTest, ParseMultipleStatements) {
    auto stmt = parseStatement("x: int = 42\ny: int = 10\nz: int = x + y");
    auto program = dynamic_cast<Program*>(stmt.get());
    ASSERT_EQ(program->statements.size(), 3);

    auto varDecl1 = dynamic_cast<VariableDeclaration*>(program->statements[0].get());
    auto varDecl2 = dynamic_cast<VariableDeclaration*>(program->statements[1].get());
    auto varDecl3 = dynamic_cast<VariableDeclaration*>(program->statements[2].get());

    ASSERT_NE(varDecl1, nullptr);
    ASSERT_NE(varDecl2, nullptr);
    ASSERT_NE(varDecl3, nullptr);

    EXPECT_EQ(varDecl1->variable.getValue(), "x");
    EXPECT_EQ(varDecl2->variable.getValue(), "y");
    EXPECT_EQ(varDecl3->variable.getValue(), "z");
}

TEST_F(StatementParserTest, ParseMixedStatements) {
    auto stmt = parseStatement("x: int = 5\nx = 10\nif (x > 5) {\n    test()\n}");
    auto program = dynamic_cast<Program*>(stmt.get());
    ASSERT_EQ(program->statements.size(), 3);

    auto varDecl = dynamic_cast<VariableDeclaration*>(program->statements[0].get());
    auto assignment = dynamic_cast<Assignment*>(program->statements[1].get());
    auto ifStmt = dynamic_cast<IfStatement*>(program->statements[2].get());

    ASSERT_NE(varDecl, nullptr);
    ASSERT_NE(assignment, nullptr);
    ASSERT_NE(ifStmt, nullptr);
}

TEST_F(StatementParserTest, ParseStatementsWithComments) {
    auto stmt = parseStatement("x: int = 42 // This is a variable\ny: int = x + 1");
    auto program = dynamic_cast<Program*>(stmt.get());
    ASSERT_EQ(program->statements.size(), 2);

    auto varDecl1 = dynamic_cast<VariableDeclaration*>(program->statements[0].get());
    auto varDecl2 = dynamic_cast<VariableDeclaration*>(program->statements[1].get());

    ASSERT_NE(varDecl1, nullptr);
    ASSERT_NE(varDecl2, nullptr);
    EXPECT_EQ(varDecl1->variable.getValue(), "x");
    EXPECT_EQ(varDecl2->variable.getValue(), "y");
}

// ===== NEWLINE AND WHITESPACE HANDLING =====

TEST_F(StatementParserTest, ParseStatementsWithExtraNewlines) {
    auto stmt = parseStatement("\n\nx: int = 42\n\n\ny: int = 10\n\n");
    auto program = dynamic_cast<Program*>(stmt.get());
    ASSERT_EQ(program->statements.size(), 2);
}

TEST_F(StatementParserTest, ParseEmptyProgram) {
    auto stmt = parseStatement("");
    auto program = dynamic_cast<Program*>(stmt.get());
    ASSERT_EQ(program->statements.size(), 0);
}

TEST_F(StatementParserTest, ParseOnlyNewlines) {
    auto stmt = parseStatement("\n\n\n");
    auto program = dynamic_cast<Program*>(stmt.get());
    ASSERT_EQ(program->statements.size(), 0);
}

// ===== COMPLEX NESTED EXPRESSION TESTS =====

TEST_F(StatementParserTest, ParseNestedExpressions) {
    auto stmt = parseStatement("result: int = ((x + y) * 2) - (z / 4)");
    auto program = dynamic_cast<Program*>(stmt.get());
    auto varDecl = dynamic_cast<VariableDeclaration*>(program->statements[0].get());

    ASSERT_NE(varDecl, nullptr);
    auto expr = dynamic_cast<BinaryExpression*>(varDecl->expr.get());
    ASSERT_NE(expr, nullptr);
    EXPECT_EQ(expr->operator_.getType(), TokenType::MINUS);
}

TEST_F(StatementParserTest, ParseComplexIfConditions) {
    auto stmt = parseStatement("if (!flag && (x > 0)) {\n    result = x * 2\n}");
    auto program = dynamic_cast<Program*>(stmt.get());
    auto ifStmt = dynamic_cast<IfStatement*>(program->statements[0].get());

    ASSERT_NE(ifStmt, nullptr);
    // Condition should be parsed correctly even if we don't have && operator yet
    ASSERT_NE(ifStmt->condition, nullptr);
}

// ===== IDENTIFIER AND LITERAL EDGE CASES =====

TEST_F(StatementParserTest, ParseLongIdentifiers) {
    auto stmt = parseStatement("very_long_variable_name_with_underscores: int = 42");
    auto program = dynamic_cast<Program*>(stmt.get());
    auto varDecl = dynamic_cast<VariableDeclaration*>(program->statements[0].get());

    EXPECT_EQ(varDecl->variable.getValue(), "very_long_variable_name_with_underscores");
}

TEST_F(StatementParserTest, ParseIdentifiersWithNumbers) {
    auto stmt = parseStatement("var123: int = 456");
    auto program = dynamic_cast<Program*>(stmt.get());
    auto varDecl = dynamic_cast<VariableDeclaration*>(program->statements[0].get());

    EXPECT_EQ(varDecl->variable.getValue(), "var123");
}

TEST_F(StatementParserTest, ParseStringLiterals) {
    auto stmt = parseStatement("name: str = \"Hello, World!\"");
    auto program = dynamic_cast<Program*>(stmt.get());
    auto varDecl = dynamic_cast<VariableDeclaration*>(program->statements[0].get());

    auto literal = dynamic_cast<LiteralExpression*>(varDecl->expr.get());
    ASSERT_NE(literal, nullptr);
    EXPECT_EQ(literal->value.getValue(), "Hello, World!");
}

// ===== EXPRESSION STATEMENT TESTS =====

TEST_F(StatementParserTest, ParseExpressionStatements) {
    auto stmt = parseStatement("x + y\n42\n\"hello\"");
    auto program = dynamic_cast<Program*>(stmt.get());
    ASSERT_EQ(program->statements.size(), 3);

    auto exprStmt1 = dynamic_cast<ExpressionStatement*>(program->statements[0].get());
    auto exprStmt2 = dynamic_cast<ExpressionStatement*>(program->statements[1].get());
    auto exprStmt3 = dynamic_cast<ExpressionStatement*>(program->statements[2].get());

    ASSERT_NE(exprStmt1, nullptr);
    ASSERT_NE(exprStmt2, nullptr);
    ASSERT_NE(exprStmt3, nullptr);
}

// ===== COMPREHENSIVE INTEGRATION TESTS =====

TEST_F(StatementParserTest, ParseComprehensiveProgram) {
    std::string program = R"(
        x: int = 10
        y: int = 20
        sum: int = x + y
        product: int = x * y

        x = x + 1

        if (sum > 25) {
            result = sum * 2
        }

        test()
        foo()
    )";

    auto stmt = parseStatement(program);
    auto parsedProgram = dynamic_cast<Program*>(stmt.get());
    ASSERT_GE(parsedProgram->statements.size(), 7); // At least 7 statements
}

TEST_F(StatementParserTest, ParseNestedComplexity) {
    auto stmt = parseStatement("complex: int = (((x + y) * z) - ((a / b) + c)) * 2");
    auto program = dynamic_cast<Program*>(stmt.get());
    auto varDecl = dynamic_cast<VariableDeclaration*>(program->statements[0].get());

    ASSERT_NE(varDecl, nullptr);
    EXPECT_EQ(varDecl->variable.getValue(), "complex");

    // Just verify it parses without error - the expression structure is complex
    ASSERT_NE(varDecl->expr, nullptr);
}

// ===== BITWISE OPERATOR TESTS =====

TEST_F(ParserTest, ParseBitwiseAndOperator) {
    auto expr = parseExpression("a & b");

    ASSERT_NE(expr, nullptr);
    auto binary = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary, nullptr);
    EXPECT_EQ(binary->operator_.getType(), TokenType::BITWISE_AND);

    // Check operands
    auto leftId = dynamic_cast<IdentifierExpression*>(binary->left.get());
    auto rightId = dynamic_cast<IdentifierExpression*>(binary->right.get());
    ASSERT_NE(leftId, nullptr);
    ASSERT_NE(rightId, nullptr);
    EXPECT_EQ(leftId->name, "a");
    EXPECT_EQ(rightId->name, "b");
}

TEST_F(ParserTest, ParseBitwiseOrOperator) {
    auto expr = parseExpression("x | y");

    ASSERT_NE(expr, nullptr);
    auto binary = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary, nullptr);
    EXPECT_EQ(binary->operator_.getType(), TokenType::BITWISE_OR);

    // Check operands
    auto leftId = dynamic_cast<IdentifierExpression*>(binary->left.get());
    auto rightId = dynamic_cast<IdentifierExpression*>(binary->right.get());
    ASSERT_NE(leftId, nullptr);
    ASSERT_NE(rightId, nullptr);
    EXPECT_EQ(leftId->name, "x");
    EXPECT_EQ(rightId->name, "y");
}

TEST_F(ParserTest, ParseBitwiseXorOperator) {
    auto expr = parseExpression("m ^ n");

    ASSERT_NE(expr, nullptr);
    auto binary = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(binary, nullptr);
    EXPECT_EQ(binary->operator_.getType(), TokenType::BITWISE_XOR);

    // Check operands
    auto leftId = dynamic_cast<IdentifierExpression*>(binary->left.get());
    auto rightId = dynamic_cast<IdentifierExpression*>(binary->right.get());
    ASSERT_NE(leftId, nullptr);
    ASSERT_NE(rightId, nullptr);
    EXPECT_EQ(leftId->name, "m");
    EXPECT_EQ(rightId->name, "n");
}

TEST_F(ParserTest, ParseBitwiseOperatorPrecedence) {
    // Test: a & b | c should be parsed as (a & b) | c
    // because & has higher precedence than |
    auto expr = parseExpression("a & b | c");

    ASSERT_NE(expr, nullptr);
    auto bitwiseOr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(bitwiseOr, nullptr);
    EXPECT_EQ(bitwiseOr->operator_.getType(), TokenType::BITWISE_OR);

    // Left side should be (a & b)
    auto bitwiseAnd = dynamic_cast<BinaryExpression*>(bitwiseOr->left.get());
    ASSERT_NE(bitwiseAnd, nullptr);
    EXPECT_EQ(bitwiseAnd->operator_.getType(), TokenType::BITWISE_AND);

    // Right side should be c
    auto rightId = dynamic_cast<IdentifierExpression*>(bitwiseOr->right.get());
    ASSERT_NE(rightId, nullptr);
    EXPECT_EQ(rightId->name, "c");
}

TEST_F(ParserTest, ParseLogicalVsBitwiseOperators) {
    // Test: a && b & c || d | e
    // Should be: ((a && (b & c)) || (d | e))
    auto expr = parseExpression("a && b & c || d | e\n");

    ASSERT_NE(expr, nullptr);
    auto logicalOr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(logicalOr, nullptr);
    EXPECT_EQ(logicalOr->operator_.getType(), TokenType::LOGIC_OR);

    // Left side should be (a && (b & c))
    auto logicalAnd = dynamic_cast<BinaryExpression*>(logicalOr->left.get());
    ASSERT_NE(logicalAnd, nullptr);
    EXPECT_EQ(logicalAnd->operator_.getType(), TokenType::LOGIC_AND);

    // Right side of logical AND should be (b & c)
    auto bitwiseAnd = dynamic_cast<BinaryExpression*>(logicalAnd->right.get());
    ASSERT_NE(bitwiseAnd, nullptr);
    EXPECT_EQ(bitwiseAnd->operator_.getType(), TokenType::BITWISE_AND);

    // Right side of logical OR should be (d | e)
    auto bitwiseOr = dynamic_cast<BinaryExpression*>(logicalOr->right.get());
    ASSERT_NE(bitwiseOr, nullptr);
    EXPECT_EQ(bitwiseOr->operator_.getType(), TokenType::BITWISE_OR);
}

TEST_F(ParserTest, ParseBitwiseWithArithmetic) {
    // Test: a + b & c * d
    // Should be: (a + b) & (c * d)
    // because arithmetic has higher precedence than bitwise
    auto expr = parseExpression("a + b & c * d");

    ASSERT_NE(expr, nullptr);
    auto bitwiseAnd = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(bitwiseAnd, nullptr);
    EXPECT_EQ(bitwiseAnd->operator_.getType(), TokenType::BITWISE_AND);

    // Left side should be (a + b)
    auto leftAddition = dynamic_cast<BinaryExpression*>(bitwiseAnd->left.get());
    ASSERT_NE(leftAddition, nullptr);
    EXPECT_EQ(leftAddition->operator_.getType(), TokenType::PLUS);

    // Right side should be (c * d)
    auto rightMultiplication = dynamic_cast<BinaryExpression*>(bitwiseAnd->right.get());
    ASSERT_NE(rightMultiplication, nullptr);
    EXPECT_EQ(rightMultiplication->operator_.getType(), TokenType::MULT);
}

TEST_F(ParserTest, ParseBitwiseWithComparison) {
    // Test: a & b == c | d
    // Should be: (a & b) == (c | d)
    // because comparison has lower precedence than bitwise
    auto expr = parseExpression("a & b == c | d\n");

    ASSERT_NE(expr, nullptr);
    auto equality = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(equality, nullptr);
    EXPECT_EQ(equality->operator_.getType(), TokenType::EQUAL_EQUAL);

    // Left side should be (a & b)
    auto leftBitwiseAnd = dynamic_cast<BinaryExpression*>(equality->left.get());
    ASSERT_NE(leftBitwiseAnd, nullptr);
    EXPECT_EQ(leftBitwiseAnd->operator_.getType(), TokenType::BITWISE_AND);

    // Right side should be (c | d)
    auto rightBitwiseOr = dynamic_cast<BinaryExpression*>(equality->right.get());
    ASSERT_NE(rightBitwiseOr, nullptr);
    EXPECT_EQ(rightBitwiseOr->operator_.getType(), TokenType::BITWISE_OR);
}

TEST_F(ParserTest, ParseComplexBitwiseExpression) {
    // Test: !flag & mask | default_value & 0xFF
    // Should be: ((!flag & mask) | (default_value & 0xFF))
    auto expr = parseExpression("!flag & mask | default_value & 255\n");

    ASSERT_NE(expr, nullptr);
    auto bitwiseOr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(bitwiseOr, nullptr);
    EXPECT_EQ(bitwiseOr->operator_.getType(), TokenType::BITWISE_OR);

    // Left side should be (!flag & mask)
    auto leftBitwiseAnd = dynamic_cast<BinaryExpression*>(bitwiseOr->left.get());
    ASSERT_NE(leftBitwiseAnd, nullptr);
    EXPECT_EQ(leftBitwiseAnd->operator_.getType(), TokenType::BITWISE_AND);

    // Left side of that should be !flag
    auto unaryNot = dynamic_cast<UnaryExpression*>(leftBitwiseAnd->left.get());
    ASSERT_NE(unaryNot, nullptr);
    EXPECT_EQ(unaryNot->operator_.getType(), TokenType::NOT);

    // Right side should be (default_value & 255)
    auto rightBitwiseAnd = dynamic_cast<BinaryExpression*>(bitwiseOr->right.get());
    ASSERT_NE(rightBitwiseAnd, nullptr);
    EXPECT_EQ(rightBitwiseAnd->operator_.getType(), TokenType::BITWISE_AND);
}

TEST_F(ParserTest, ParseBitwiseAssociativity) {
    // Test: a & b & c should be parsed as ((a & b) & c)
    // Left associative
    auto expr = parseExpression("a & b & c");

    ASSERT_NE(expr, nullptr);
    auto outerAnd = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(outerAnd, nullptr);
    EXPECT_EQ(outerAnd->operator_.getType(), TokenType::BITWISE_AND);

    // Left side should be (a & b)
    auto innerAnd = dynamic_cast<BinaryExpression*>(outerAnd->left.get());
    ASSERT_NE(innerAnd, nullptr);
    EXPECT_EQ(innerAnd->operator_.getType(), TokenType::BITWISE_AND);

    // Right side should be c
    auto rightId = dynamic_cast<IdentifierExpression*>(outerAnd->right.get());
    ASSERT_NE(rightId, nullptr);
    EXPECT_EQ(rightId->name, "c");
}

TEST_F(ParserTest, ParseBitwiseOrAssociativity) {
    // Test: a | b | c should be parsed as ((a | b) | c)
    // Left associative
    auto expr = parseExpression("a | b | c");

    ASSERT_NE(expr, nullptr);
    auto outerOr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(outerOr, nullptr);
    EXPECT_EQ(outerOr->operator_.getType(), TokenType::BITWISE_OR);

    // Left side should be (a | b)
    auto innerOr = dynamic_cast<BinaryExpression*>(outerOr->left.get());
    ASSERT_NE(innerOr, nullptr);
    EXPECT_EQ(innerOr->operator_.getType(), TokenType::BITWISE_OR);

    // Right side should be c
    auto rightId = dynamic_cast<IdentifierExpression*>(outerOr->right.get());
    ASSERT_NE(rightId, nullptr);
    EXPECT_EQ(rightId->name, "c");
}

TEST_F(ParserTest, ParseBitwiseXorPrecedence) {
    // Test: a & b ^ c | d should be parsed as ((a & b) ^ c) | d
    auto expr = parseExpression("a & b ^ c | d");

    ASSERT_NE(expr, nullptr);
    auto or_expr = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(or_expr, nullptr);
    EXPECT_EQ(or_expr->operator_.getType(), TokenType::BITWISE_OR);

    // Left side should be ((a & b) ^ c)
    auto xor_expr = dynamic_cast<BinaryExpression*>(or_expr->left.get());
    ASSERT_NE(xor_expr, nullptr);
    EXPECT_EQ(xor_expr->operator_.getType(), TokenType::BITWISE_XOR);

    // Left side of XOR should be (a & b)
    auto and_expr = dynamic_cast<BinaryExpression*>(xor_expr->left.get());
    ASSERT_NE(and_expr, nullptr);
    EXPECT_EQ(and_expr->operator_.getType(), TokenType::BITWISE_AND);
}


TEST_F(ParserTest, ParseBitwiseXorAssociativity) {
    // Test: a ^ b ^ c should be parsed as ((a ^ b) ^ c)
    // Left associative
    auto expr = parseExpression("a ^ b ^ c");

    ASSERT_NE(expr, nullptr);
    auto outerXor = dynamic_cast<BinaryExpression*>(expr.get());
    ASSERT_NE(outerXor, nullptr);
    EXPECT_EQ(outerXor->operator_.getType(), TokenType::BITWISE_XOR);

    // Left side should be (a ^ b)
    auto innerXor = dynamic_cast<BinaryExpression*>(outerXor->left.get());
    ASSERT_NE(innerXor, nullptr);
    EXPECT_EQ(innerXor->operator_.getType(), TokenType::BITWISE_XOR);

    // Right side should be c
    auto rightId = dynamic_cast<IdentifierExpression*>(outerXor->right.get());
    ASSERT_NE(rightId, nullptr);
    EXPECT_EQ(rightId->name, "c");
}