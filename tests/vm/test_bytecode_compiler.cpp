#include <gtest/gtest.h>
#include <limits>
#include <cmath>
#include "../../include/backends/vm/BytecodeCompiler.hpp"
#include "../../include/parser/Expression.hpp"
#include "../../include/parser/Statement.hpp"
#include "../../include/lexer/Token.hpp"
#include "../../include/backends/vm/OPCode.hpp"

class BytecodeCompilerTest : public ::testing::Test {
protected:
    BytecodeCompiler compiler;

    void SetUp() override {}
    void TearDown() override {}

    // Helper to create tokens
    Token makeIntToken(int value) {
        return Token(TokenType::INT, std::to_string(value), 1, 1);
    }

    Token makeFloatToken(float value) {
        return Token(TokenType::FLOAT, std::to_string(value), 1, 1);
    }

    Token makeDoubleToken(double value) {
        return Token(TokenType::DOUBLE, std::to_string(value), 1, 1);
    }

    Token makeBoolToken(bool value) {
        return Token(TokenType::BOOL, value ? "true" : "false", 1, 1);
    }

    Token makeStringToken(const std::string& value) {
        return Token(TokenType::STRING, value, 1, 1);
    }

    Token makeOperatorToken(TokenType type) {
        return Token(type, "", 1, 1);
    }

    // Helper to create expressions
    std::unique_ptr<LiteralExpression> makeLiteral(const Token& token) {
        return std::make_unique<LiteralExpression>(token);
    }

    std::unique_ptr<BinaryExpression> makeBinary(
        std::unique_ptr<Expression> left,
        const Token& op,
        std::unique_ptr<Expression> right) {
        return std::make_unique<BinaryExpression>(
            std::move(left), op, std::move(right));
    }

    std::unique_ptr<UnaryExpression> makeUnary(
        const Token& op,
        std::unique_ptr<Expression> operand) {
        return std::make_unique<UnaryExpression>(op, std::move(operand));
    }

    // Helper to wrap expression in statement for compilation
    std::unique_ptr<Statement> wrapInProgram(std::unique_ptr<Expression> expr) {
        auto exprStmt = std::make_unique<ExpressionStatement>(std::move(expr));
        std::vector<std::unique_ptr<Statement>> statements;
        statements.push_back(std::move(exprStmt));
        return std::make_unique<Program>(std::move(statements));
    }
};

// ============================================================================
// LITERAL EXPRESSION TESTS
// ============================================================================

TEST_F(BytecodeCompilerTest, CompileIntLiteral) {
    auto expr = makeLiteral(makeIntToken(42));
    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 2);  // LOAD_INT + HALT
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_INT);
    EXPECT_EQ(program.instructions[0].operand, 0);  // Index in constants pool
    EXPECT_EQ(program.instructions[1].opcode, OPCode::HALT);
    ASSERT_EQ(program.constants.size(), 1);
    EXPECT_EQ(program.constants[0].type, TypedValue::Type::INT);
    EXPECT_EQ(program.constants[0].value.i, 42);
    EXPECT_TRUE(program.strings.empty());
}

TEST_F(BytecodeCompilerTest, CompileFloatLiteral) {
    auto expr = makeLiteral(makeFloatToken(3.14f));
    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 2);  // LOAD_FLOAT + HALT
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_FLOAT);
    EXPECT_EQ(program.instructions[0].operand, 0);  // Index in constants pool
    EXPECT_EQ(program.instructions[1].opcode, OPCode::HALT);
    ASSERT_EQ(program.constants.size(), 1);
    EXPECT_EQ(program.constants[0].type, TypedValue::Type::FLOAT);
    EXPECT_FLOAT_EQ(program.constants[0].value.f, 3.14f);
    EXPECT_TRUE(program.strings.empty());
}

TEST_F(BytecodeCompilerTest, CompileDoubleLiteral) {
    auto expr = makeLiteral(makeDoubleToken(2.718));
    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 2);  // LOAD_DOUBLE + HALT
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_DOUBLE);
    EXPECT_EQ(program.instructions[0].operand, 0);  // Index in constants pool
    EXPECT_EQ(program.instructions[1].opcode, OPCode::HALT);
    ASSERT_EQ(program.constants.size(), 1);
    EXPECT_EQ(program.constants[0].type, TypedValue::Type::DOUBLE);
    EXPECT_DOUBLE_EQ(program.constants[0].value.d, 2.718);
    EXPECT_TRUE(program.strings.empty());
}

TEST_F(BytecodeCompilerTest, CompileBoolLiteralTrue) {
    auto expr = makeLiteral(makeBoolToken(true));
    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 2);  // LOAD_BOOL + HALT
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_BOOL);
    EXPECT_EQ(program.instructions[0].operand, 0);  // Index in constants pool
    EXPECT_EQ(program.instructions[1].opcode, OPCode::HALT);
    ASSERT_EQ(program.constants.size(), 1);
    EXPECT_EQ(program.constants[0].type, TypedValue::Type::BOOL);
    EXPECT_EQ(program.constants[0].value.b, true);
    EXPECT_TRUE(program.strings.empty());
}

TEST_F(BytecodeCompilerTest, CompileBoolLiteralFalse) {
    auto expr = makeLiteral(makeBoolToken(false));
    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 2);  // LOAD_BOOL + HALT
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_BOOL);
    EXPECT_EQ(program.instructions[0].operand, 0);  // Index in constants pool
    EXPECT_EQ(program.instructions[1].opcode, OPCode::HALT);
    ASSERT_EQ(program.constants.size(), 1);
    EXPECT_EQ(program.constants[0].type, TypedValue::Type::BOOL);
    EXPECT_EQ(program.constants[0].value.b, false);
    EXPECT_TRUE(program.strings.empty());
}

TEST_F(BytecodeCompilerTest, CompileStringLiteral) {
    auto expr = makeLiteral(makeStringToken("hello"));
    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 2);  // LOAD_STRING + HALT
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_STRING);
    EXPECT_EQ(program.instructions[0].operand, 0);  // Index in string pool
    EXPECT_EQ(program.instructions[1].opcode, OPCode::HALT);
    EXPECT_TRUE(program.constants.empty());
    ASSERT_EQ(program.strings.size(), 1);
    EXPECT_EQ(program.strings[0], "hello");
}

TEST_F(BytecodeCompilerTest, CompileMultipleStringLiterals) {
    auto expr1 = makeLiteral(makeStringToken("first"));
    auto expr2 = makeLiteral(makeStringToken("second"));
    auto binary = makeBinary(std::move(expr1), makeOperatorToken(TokenType::PLUS), std::move(expr2));

    auto program = compiler.compile(wrapInProgram(std::move(binary)));

    EXPECT_EQ(program.strings.size(), 2);
    EXPECT_EQ(program.strings[0], "first");
    EXPECT_EQ(program.strings[1], "second");
}

// ============================================================================
// BINARY EXPRESSION TESTS - ARITHMETIC
// ============================================================================

TEST_F(BytecodeCompilerTest, CompileIntAddition) {
    auto left = makeLiteral(makeIntToken(5));
    auto right = makeLiteral(makeIntToken(3));
    auto expr = makeBinary(std::move(left), makeOperatorToken(TokenType::PLUS), std::move(right));

    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 4);  // 2 LOAD_INT + ADD_INT + HALT
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_INT);
    EXPECT_EQ(program.instructions[0].operand, 0);  // Index 0 in constants
    EXPECT_EQ(program.instructions[1].opcode, OPCode::LOAD_INT);
    EXPECT_EQ(program.instructions[1].operand, 1);  // Index 1 in constants
    EXPECT_EQ(program.instructions[2].opcode, OPCode::ADD_INT);
    EXPECT_EQ(program.instructions[3].opcode, OPCode::HALT);
    ASSERT_EQ(program.constants.size(), 2);
    EXPECT_EQ(program.constants[0].value.i, 5);
    EXPECT_EQ(program.constants[1].value.i, 3);
}

TEST_F(BytecodeCompilerTest, CompileFloatAddition) {
    auto left = makeLiteral(makeFloatToken(2.5f));
    auto right = makeLiteral(makeFloatToken(1.5f));
    auto expr = makeBinary(std::move(left), makeOperatorToken(TokenType::PLUS), std::move(right));

    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 4);  // 2 LOAD_FLOAT + ADD_FLOAT + HALT
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_FLOAT);
    EXPECT_EQ(program.instructions[1].opcode, OPCode::LOAD_FLOAT);
    EXPECT_EQ(program.instructions[2].opcode, OPCode::ADD_FLOAT);
    EXPECT_EQ(program.instructions[3].opcode, OPCode::HALT);
}

TEST_F(BytecodeCompilerTest, CompileDoubleAddition) {
    auto left = makeLiteral(makeDoubleToken(2.5));
    auto right = makeLiteral(makeDoubleToken(1.5));
    auto expr = makeBinary(std::move(left), makeOperatorToken(TokenType::PLUS), std::move(right));

    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 4);  // 2 LOAD_DOUBLE + ADD_DOUBLE + HALT
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_DOUBLE);
    EXPECT_EQ(program.instructions[1].opcode, OPCode::LOAD_DOUBLE);
    EXPECT_EQ(program.instructions[2].opcode, OPCode::ADD_DOUBLE);
    EXPECT_EQ(program.instructions[3].opcode, OPCode::HALT);
}

TEST_F(BytecodeCompilerTest, CompileIntSubtraction) {
    auto left = makeLiteral(makeIntToken(10));
    auto right = makeLiteral(makeIntToken(4));
    auto expr = makeBinary(std::move(left), makeOperatorToken(TokenType::MINUS), std::move(right));

    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 4);  // 2 LOAD_INT + SUB_INT + HALT
    EXPECT_EQ(program.instructions[2].opcode, OPCode::SUB_INT);
    EXPECT_EQ(program.instructions[3].opcode, OPCode::HALT);
}

TEST_F(BytecodeCompilerTest, CompileIntMultiplication) {
    auto left = makeLiteral(makeIntToken(6));
    auto right = makeLiteral(makeIntToken(7));
    auto expr = makeBinary(std::move(left), makeOperatorToken(TokenType::MULT), std::move(right));

    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 4);  // 2 LOAD_INT + MULT_INT + HALT
    EXPECT_EQ(program.instructions[2].opcode, OPCode::MULT_INT);
    EXPECT_EQ(program.instructions[3].opcode, OPCode::HALT);
}

TEST_F(BytecodeCompilerTest, CompileIntDivision) {
    auto left = makeLiteral(makeIntToken(15));
    auto right = makeLiteral(makeIntToken(3));
    auto expr = makeBinary(std::move(left), makeOperatorToken(TokenType::DIV), std::move(right));

    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 4);  // 2 LOAD_INT + DIV_INT + HALT
    EXPECT_EQ(program.instructions[2].opcode, OPCode::DIV_INT);
    EXPECT_EQ(program.instructions[3].opcode, OPCode::HALT);
}

// ============================================================================
// BINARY EXPRESSION TESTS - COMPARISON
// ============================================================================

TEST_F(BytecodeCompilerTest, CompileIntEquality) {
    auto left = makeLiteral(makeIntToken(5));
    auto right = makeLiteral(makeIntToken(5));
    auto expr = makeBinary(std::move(left), makeOperatorToken(TokenType::EQUAL_EQUAL), std::move(right));

    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 4);  // 2 LOAD_INT + EQ_INT + HALT
    EXPECT_EQ(program.instructions[2].opcode, OPCode::EQ_INT);
    EXPECT_EQ(program.instructions[3].opcode, OPCode::HALT);
}

TEST_F(BytecodeCompilerTest, CompileDoubleEquality) {
    auto left = makeLiteral(makeDoubleToken(3.14));
    auto right = makeLiteral(makeDoubleToken(3.14));
    auto expr = makeBinary(std::move(left), makeOperatorToken(TokenType::EQUAL_EQUAL), std::move(right));

    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 4);  // 2 LOAD_DOUBLE + EQ_DOUBLE + HALT
    EXPECT_EQ(program.instructions[2].opcode, OPCode::EQ_DOUBLE);
    EXPECT_EQ(program.instructions[3].opcode, OPCode::HALT);
}

TEST_F(BytecodeCompilerTest, CompileIntLessThan) {
    auto left = makeLiteral(makeIntToken(3));
    auto right = makeLiteral(makeIntToken(5));
    auto expr = makeBinary(std::move(left), makeOperatorToken(TokenType::LESS), std::move(right));

    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 4);  // 2 LOAD_INT + LT_INT + HALT
    EXPECT_EQ(program.instructions[2].opcode, OPCode::LT_INT);
    EXPECT_EQ(program.instructions[3].opcode, OPCode::HALT);
}

TEST_F(BytecodeCompilerTest, CompileDoubleLessThan) {
    auto left = makeLiteral(makeDoubleToken(2.5));
    auto right = makeLiteral(makeDoubleToken(3.5));
    auto expr = makeBinary(std::move(left), makeOperatorToken(TokenType::LESS), std::move(right));

    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 4);  // 2 LOAD_DOUBLE + LT_DOUBLE + HALT
    EXPECT_EQ(program.instructions[2].opcode, OPCode::LT_DOUBLE);
    EXPECT_EQ(program.instructions[3].opcode, OPCode::HALT);
}

// ============================================================================
// UNARY EXPRESSION TESTS
// ============================================================================

TEST_F(BytecodeCompilerTest, CompileIntNegation) {
    auto operand = makeLiteral(makeIntToken(42));
    auto expr = makeUnary(makeOperatorToken(TokenType::MINUS), std::move(operand));

    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 3);  // LOAD_INT + NEG_INT + HALT
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_INT);
    EXPECT_EQ(program.instructions[0].operand, 0);  // Index in constants pool
    EXPECT_EQ(program.instructions[1].opcode, OPCode::NEG_INT);
    EXPECT_EQ(program.instructions[2].opcode, OPCode::HALT);
    ASSERT_EQ(program.constants.size(), 1);
    EXPECT_EQ(program.constants[0].value.i, 42);
}

TEST_F(BytecodeCompilerTest, CompileFloatNegation) {
    auto operand = makeLiteral(makeFloatToken(3.14f));
    auto expr = makeUnary(makeOperatorToken(TokenType::MINUS), std::move(operand));

    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 3);  // LOAD_FLOAT + NEG_FLOAT + HALT
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_FLOAT);
    EXPECT_EQ(program.instructions[0].operand, 0);  // Index in constants pool
    EXPECT_EQ(program.instructions[1].opcode, OPCode::NEG_FLOAT);
    EXPECT_EQ(program.instructions[2].opcode, OPCode::HALT);
    ASSERT_EQ(program.constants.size(), 1);
    EXPECT_FLOAT_EQ(program.constants[0].value.f, 3.14f);
}

TEST_F(BytecodeCompilerTest, CompileDoubleNegation) {
    auto operand = makeLiteral(makeDoubleToken(2.718));
    auto expr = makeUnary(makeOperatorToken(TokenType::MINUS), std::move(operand));

    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 3);  // LOAD_DOUBLE + NEG_DOUBLE + HALT
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_DOUBLE);
    EXPECT_EQ(program.instructions[0].operand, 0);  // Index in constants pool
    EXPECT_EQ(program.instructions[1].opcode, OPCode::NEG_DOUBLE);
    EXPECT_EQ(program.instructions[2].opcode, OPCode::HALT);
    ASSERT_EQ(program.constants.size(), 1);
    EXPECT_DOUBLE_EQ(program.constants[0].value.d, 2.718);
}

TEST_F(BytecodeCompilerTest, CompileBoolNot) {
    auto operand = makeLiteral(makeBoolToken(true));
    auto expr = makeUnary(makeOperatorToken(TokenType::NOT), std::move(operand));

    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 3);  // LOAD_BOOL + NOT_BOOL + HALT
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_BOOL);
    EXPECT_EQ(program.instructions[0].operand, 0);  // Index in constants pool
    EXPECT_EQ(program.instructions[1].opcode, OPCode::NOT_BOOL);
    EXPECT_EQ(program.instructions[2].opcode, OPCode::HALT);
    ASSERT_EQ(program.constants.size(), 1);
    EXPECT_EQ(program.constants[0].value.b, true);
}

// ============================================================================
// COMPLEX EXPRESSION TESTS
// ============================================================================

TEST_F(BytecodeCompilerTest, CompileNestedArithmetic) {
    // Test: (5 + 3) * 2
    auto five = makeLiteral(makeIntToken(5));
    auto three = makeLiteral(makeIntToken(3));
    auto addition = makeBinary(std::move(five), makeOperatorToken(TokenType::PLUS), std::move(three));

    auto two = makeLiteral(makeIntToken(2));
    auto multiplication = makeBinary(std::move(addition), makeOperatorToken(TokenType::MULT), std::move(two));

    auto program = compiler.compile(wrapInProgram(std::move(multiplication)));

    ASSERT_EQ(program.instructions.size(), 6);  // 3 LOAD_INT + ADD_INT + MULT_INT + HALT
    // Left side: 5 + 3
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_INT);
    EXPECT_EQ(program.instructions[0].operand, 0);  // Index in constants
    EXPECT_EQ(program.instructions[1].opcode, OPCode::LOAD_INT);
    EXPECT_EQ(program.instructions[1].operand, 1);  // Index in constants
    EXPECT_EQ(program.instructions[2].opcode, OPCode::ADD_INT);
    // Right side and final operation
    EXPECT_EQ(program.instructions[3].opcode, OPCode::LOAD_INT);
    EXPECT_EQ(program.instructions[3].operand, 2);  // Index in constants
    EXPECT_EQ(program.instructions[4].opcode, OPCode::MULT_INT);
    EXPECT_EQ(program.instructions[5].opcode, OPCode::HALT);
    ASSERT_EQ(program.constants.size(), 3);
    EXPECT_EQ(program.constants[0].value.i, 5);
    EXPECT_EQ(program.constants[1].value.i, 3);
    EXPECT_EQ(program.constants[2].value.i, 2);
}

TEST_F(BytecodeCompilerTest, CompileNestedWithUnary) {
    // Test: -5 + 3
    auto five = makeLiteral(makeIntToken(5));
    auto negation = makeUnary(makeOperatorToken(TokenType::MINUS), std::move(five));

    auto three = makeLiteral(makeIntToken(3));
    auto addition = makeBinary(std::move(negation), makeOperatorToken(TokenType::PLUS), std::move(three));

    auto program = compiler.compile(wrapInProgram(std::move(addition)));

    ASSERT_EQ(program.instructions.size(), 5);  // 2 LOAD_INT + NEG_INT + ADD_INT + HALT
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_INT);
    EXPECT_EQ(program.instructions[0].operand, 0);  // Index in constants
    EXPECT_EQ(program.instructions[1].opcode, OPCode::NEG_INT);
    EXPECT_EQ(program.instructions[2].opcode, OPCode::LOAD_INT);
    EXPECT_EQ(program.instructions[2].operand, 1);  // Index in constants
    EXPECT_EQ(program.instructions[3].opcode, OPCode::ADD_INT);
    EXPECT_EQ(program.instructions[4].opcode, OPCode::HALT);
    ASSERT_EQ(program.constants.size(), 2);
    EXPECT_EQ(program.constants[0].value.i, 5);
    EXPECT_EQ(program.constants[1].value.i, 3);
}

// ============================================================================
// ERROR CONDITION TESTS
// ============================================================================

TEST_F(BytecodeCompilerTest, TypeMismatchInBinaryExpression) {
    auto intLiteral = makeLiteral(makeIntToken(5));
    auto floatLiteral = makeLiteral(makeFloatToken(3.14f));
    auto expr = makeBinary(std::move(intLiteral), makeOperatorToken(TokenType::PLUS), std::move(floatLiteral));

    EXPECT_THROW(compiler.compile(wrapInProgram(std::move(expr))), RuntimeException);
}

TEST_F(BytecodeCompilerTest, IdentifierNotImplemented) {
    auto expr = std::make_unique<IdentifierExpression>("myVariable");

    EXPECT_THROW(compiler.compile(wrapInProgram(std::move(expr))), RuntimeException);
}

// ============================================================================
// EDGE CASES AND BOUNDARY TESTS
// ============================================================================

TEST_F(BytecodeCompilerTest, CompileZeroValues) {
    auto expr = makeLiteral(makeIntToken(0));
    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 2);  // LOAD_INT + HALT
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_INT);
    EXPECT_EQ(program.instructions[0].operand, 0);
    EXPECT_EQ(program.instructions[1].opcode, OPCode::HALT);
}

TEST_F(BytecodeCompilerTest, CompileNegativeValues) {
    auto expr = makeLiteral(makeIntToken(-42));
    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 2);  // LOAD_INT + HALT
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_INT);
    EXPECT_EQ(program.instructions[0].operand, 0);  // Index in constants
    EXPECT_EQ(program.instructions[1].opcode, OPCode::HALT);
    ASSERT_EQ(program.constants.size(), 1);
    EXPECT_EQ(program.constants[0].value.i, -42);
}

TEST_F(BytecodeCompilerTest, CompileEmptyString) {
    auto expr = makeLiteral(makeStringToken(""));
    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 2);  // LOAD_STRING + HALT
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_STRING);
    EXPECT_EQ(program.instructions[1].opcode, OPCode::HALT);
    ASSERT_EQ(program.strings.size(), 1);
    EXPECT_EQ(program.strings[0], "");
}

TEST_F(BytecodeCompilerTest, CompileLongString) {
    std::string longStr(1000, 'A');
    auto expr = makeLiteral(makeStringToken(longStr));
    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 2);  // LOAD_STRING + HALT
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_STRING);
    EXPECT_EQ(program.instructions[1].opcode, OPCode::HALT);
    ASSERT_EQ(program.strings.size(), 1);
    EXPECT_EQ(program.strings[0], longStr);
}

// ============================================================================
// PERFORMANCE AND STRESS TESTS
// ============================================================================

TEST_F(BytecodeCompilerTest, CompileDeeplyNestedExpression) {
    // Create a deeply nested expression: 1 + (2 + (3 + (4 + 5)))
    auto five = makeLiteral(makeIntToken(5));
    auto four = makeLiteral(makeIntToken(4));
    auto nested1 = makeBinary(std::move(four), makeOperatorToken(TokenType::PLUS), std::move(five));

    auto three = makeLiteral(makeIntToken(3));
    auto nested2 = makeBinary(std::move(three), makeOperatorToken(TokenType::PLUS), std::move(nested1));

    auto two = makeLiteral(makeIntToken(2));
    auto nested3 = makeBinary(std::move(two), makeOperatorToken(TokenType::PLUS), std::move(nested2));

    auto one = makeLiteral(makeIntToken(1));
    auto final = makeBinary(std::move(one), makeOperatorToken(TokenType::PLUS), std::move(nested3));

    auto program = compiler.compile(wrapInProgram(std::move(final)));

    // Should have 9 instructions: 5 loads + 4 additions + HALT
    EXPECT_EQ(program.instructions.size(), 10);
}

TEST_F(BytecodeCompilerTest, CompileMultipleBinaryOperations) {
    // Test: 1 + 2 - 3 * 4 / 5 (as flat binary operations)
    auto one = makeLiteral(makeIntToken(1));
    auto two = makeLiteral(makeIntToken(2));
    auto add = makeBinary(std::move(one), makeOperatorToken(TokenType::PLUS), std::move(two));

    auto three = makeLiteral(makeIntToken(3));
    auto sub = makeBinary(std::move(add), makeOperatorToken(TokenType::MINUS), std::move(three));

    auto four = makeLiteral(makeIntToken(4));
    auto mult = makeBinary(std::move(sub), makeOperatorToken(TokenType::MULT), std::move(four));

    auto five = makeLiteral(makeIntToken(5));
    auto div = makeBinary(std::move(mult), makeOperatorToken(TokenType::DIV), std::move(five));

    auto program = compiler.compile(wrapInProgram(std::move(div)));

    // Verify we get the right number of instructions and opcodes
    EXPECT_GT(program.instructions.size(), 5);
    EXPECT_EQ(program.constants.size(), 5);  // Should have 5 constants (1,2,3,4,5)
    EXPECT_TRUE(program.strings.empty());
}

// ============================================================================
// ADDITIONAL EDGE CASES AND ROBUSTNESS TESTS
// ============================================================================

TEST_F(BytecodeCompilerTest, CompileChainedNegation) {
    // Test: --5 (double negation)
    auto five = makeLiteral(makeIntToken(5));
    auto negation1 = makeUnary(makeOperatorToken(TokenType::MINUS), std::move(five));
    auto negation2 = makeUnary(makeOperatorToken(TokenType::MINUS), std::move(negation1));

    auto program = compiler.compile(wrapInProgram(std::move(negation2)));

    ASSERT_EQ(program.instructions.size(), 4);
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_INT);
    EXPECT_EQ(program.instructions[0].operand, 0);
    EXPECT_EQ(program.instructions[1].opcode, OPCode::NEG_INT);
    EXPECT_EQ(program.instructions[2].opcode, OPCode::NEG_INT);
    ASSERT_EQ(program.constants.size(), 1);
    EXPECT_EQ(program.constants[0].value.i, 5);
}

TEST_F(BytecodeCompilerTest, CompileChainedNotOperations) {
    // Test: !!true (double NOT)
    auto trueLit = makeLiteral(makeBoolToken(true));
    auto not1 = makeUnary(makeOperatorToken(TokenType::NOT), std::move(trueLit));
    auto not2 = makeUnary(makeOperatorToken(TokenType::NOT), std::move(not1));

    auto program = compiler.compile(wrapInProgram(std::move(not2)));

    ASSERT_EQ(program.instructions.size(), 4);
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_BOOL);
    EXPECT_EQ(program.instructions[1].opcode, OPCode::NOT_BOOL);
    EXPECT_EQ(program.instructions[2].opcode, OPCode::NOT_BOOL);
    EXPECT_EQ(program.instructions[3].opcode, OPCode::HALT);
    ASSERT_EQ(program.constants.size(), 1);
    EXPECT_EQ(program.constants[0].value.b, true);
}

TEST_F(BytecodeCompilerTest, CompileVeryLargeInteger) {
    auto expr = makeLiteral(makeIntToken(2147483647));  // INT_MAX
    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 2);
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_INT);
    EXPECT_EQ(program.instructions[0].operand, 0);
    ASSERT_EQ(program.constants.size(), 1);
    EXPECT_EQ(program.constants[0].value.i, 2147483647);
}

TEST_F(BytecodeCompilerTest, CompileVerySmallInteger) {
    auto expr = makeLiteral(makeIntToken(-2147483648));  // INT_MIN
    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 2);
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_INT);
    EXPECT_EQ(program.instructions[0].operand, 0);
    ASSERT_EQ(program.constants.size(), 1);
    EXPECT_EQ(program.constants[0].value.i, -2147483648);
}

// TODO: Fix this as curerntly the compiler does not support precision
// TEST_F(BytecodeCompilerTest, CompileVerySmallFloat) {
//     auto expr = makeLiteral(makeFloatToken(1.0e-10f));  // Small but representable float
//     auto program = compiler.compile(wrapInProgram(std::move(expr)));

//     ASSERT_EQ(program.instructions.size(), 1);
//     EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_DOUBLE);
//     EXPECT_EQ(program.instructions[0].operand, 0);
//     ASSERT_EQ(program.constants.size(), 1);
//     EXPECT_FLOAT_EQ(program.constants[0].value.f, 1.0e-10f);
// }

TEST_F(BytecodeCompilerTest, CompileSpecialDoubleValues) {
    // Test infinity
    auto expr = makeLiteral(makeDoubleToken(std::numeric_limits<double>::infinity()));
    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 2);
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_DOUBLE);
    ASSERT_EQ(program.constants.size(), 1);
    EXPECT_TRUE(std::isinf(program.constants[0].value.d));
}

TEST_F(BytecodeCompilerTest, CompileStringWithSpecialCharacters) {
    auto expr = makeLiteral(makeStringToken("Hello\nWorld\t\"Quote\"\\Backslash"));
    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 2);
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_STRING);
    EXPECT_EQ(program.instructions[0].operand, 0);
    ASSERT_EQ(program.strings.size(), 1);
    EXPECT_EQ(program.strings[0], "Hello\nWorld\t\"Quote\"\\Backslash");
}

TEST_F(BytecodeCompilerTest, CompileStringWithUnicodeCharacters) {
    auto expr = makeLiteral(makeStringToken("🚀 Hello 世界 🌍"));
    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    ASSERT_EQ(program.instructions.size(), 2);
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_STRING);
    ASSERT_EQ(program.strings.size(), 1);
    EXPECT_EQ(program.strings[0], "🚀 Hello 世界 🌍");
}

TEST_F(BytecodeCompilerTest, CompileMultipleIdenticalStrings) {
    // Test string deduplication (multiple "hello" should reuse same index)
    auto str1 = makeLiteral(makeStringToken("hello"));
    auto str2 = makeLiteral(makeStringToken("hello"));
    auto expr = makeBinary(std::move(str1), makeOperatorToken(TokenType::PLUS), std::move(str2));

    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    EXPECT_EQ(program.strings.size(), 2);  // Without deduplication, should have 2
    EXPECT_EQ(program.strings[0], "hello");
    EXPECT_EQ(program.strings[1], "hello");
}

TEST_F(BytecodeCompilerTest, CompileLeftAssociativeOperations) {
    // Test: ((1 + 2) + 3) + 4 (deeply left-associative)
    auto one = makeLiteral(makeIntToken(1));
    auto two = makeLiteral(makeIntToken(2));
    auto add1 = makeBinary(std::move(one), makeOperatorToken(TokenType::PLUS), std::move(two));

    auto three = makeLiteral(makeIntToken(3));
    auto add2 = makeBinary(std::move(add1), makeOperatorToken(TokenType::PLUS), std::move(three));

    auto four = makeLiteral(makeIntToken(4));
    auto add3 = makeBinary(std::move(add2), makeOperatorToken(TokenType::PLUS), std::move(four));

    auto program = compiler.compile(wrapInProgram(std::move(add3)));

    ASSERT_EQ(program.instructions.size(), 8);  // 4 loads + 3 adds + 1 halt
    EXPECT_EQ(program.constants.size(), 4);
}

TEST_F(BytecodeCompilerTest, CompileRightAssociativeOperations) {
    // Test: 1 + (2 + (3 + 4)) (deeply right-associative)
    auto three = makeLiteral(makeIntToken(3));
    auto four = makeLiteral(makeIntToken(4));
    auto add1 = makeBinary(std::move(three), makeOperatorToken(TokenType::PLUS), std::move(four));

    auto two = makeLiteral(makeIntToken(2));
    auto add2 = makeBinary(std::move(two), makeOperatorToken(TokenType::PLUS), std::move(add1));

    auto one = makeLiteral(makeIntToken(1));
    auto add3 = makeBinary(std::move(one), makeOperatorToken(TokenType::PLUS), std::move(add2));

    auto program = compiler.compile(wrapInProgram(std::move(add3)));

    ASSERT_EQ(program.instructions.size(), 8);  // 4 loads + 3 adds + 1 halt
    EXPECT_EQ(program.constants.size(), 4);
}

TEST_F(BytecodeCompilerTest, CompileMixedTypeComparison) {
    // Test mixed type error for comparison
    auto intLit = makeLiteral(makeIntToken(5));
    auto doubleLit = makeLiteral(makeDoubleToken(5.0));
    auto expr = makeBinary(std::move(intLit), makeOperatorToken(TokenType::EQUAL_EQUAL), std::move(doubleLit));

    EXPECT_THROW(compiler.compile(wrapInProgram(std::move(expr))), RuntimeException);
}

TEST_F(BytecodeCompilerTest, CompileUnsupportedBinaryOperation) {
    // Test unsupported operation (e.g., string - string doesn't exist)
    auto str1 = makeLiteral(makeStringToken("hello"));
    auto str2 = makeLiteral(makeStringToken("world"));
    auto expr = makeBinary(std::move(str1), makeOperatorToken(TokenType::MINUS), std::move(str2));

    EXPECT_THROW(compiler.compile(wrapInProgram(std::move(expr))), RuntimeException);
}

TEST_F(BytecodeCompilerTest, CompileUnsupportedUnaryOperation) {
    // Test unsupported unary operation (e.g., -string doesn't exist)
    auto str = makeLiteral(makeStringToken("hello"));
    auto expr = makeUnary(makeOperatorToken(TokenType::MINUS), std::move(str));

    EXPECT_THROW(compiler.compile(wrapInProgram(std::move(expr))), RuntimeException);
}

TEST_F(BytecodeCompilerTest, CompileConstantPoolReuse) {
    // Test that identical constants reuse the same index
    auto five1 = makeLiteral(makeIntToken(5));
    auto five2 = makeLiteral(makeIntToken(5));
    auto expr = makeBinary(std::move(five1), makeOperatorToken(TokenType::PLUS), std::move(five2));

    auto program = compiler.compile(wrapInProgram(std::move(expr)));

    // Should have 2 constants entries (since no deduplication implemented)
    EXPECT_EQ(program.constants.size(), 2);
    EXPECT_EQ(program.constants[0].value.i, 5);
    EXPECT_EQ(program.constants[1].value.i, 5);
}

TEST_F(BytecodeCompilerTest, CompileEmptyProgramHandling) {
    // This would require a different test setup since we need at least one expression
    // But it's good to think about: what happens with completely empty input?
}

TEST_F(BytecodeCompilerTest, CompileProgramStateReset) {
    // Test that multiple compilations don't interfere with each other
    auto expr1 = makeLiteral(makeIntToken(42));
    auto program1 = compiler.compile(wrapInProgram(std::move(expr1)));

    auto expr2 = makeLiteral(makeIntToken(24));
    auto program2 = compiler.compile(wrapInProgram(std::move(expr2)));

    // Each program should be independent
    ASSERT_EQ(program1.constants.size(), 1);
    ASSERT_EQ(program2.constants.size(), 1);
    EXPECT_EQ(program1.constants[0].value.i, 42);
    EXPECT_EQ(program2.constants[0].value.i, 24);
    EXPECT_TRUE(program1.strings.empty());
    EXPECT_TRUE(program2.strings.empty());
}