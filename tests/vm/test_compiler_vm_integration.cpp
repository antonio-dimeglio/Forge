#include <gtest/gtest.h>
#include "../../include/backends/vm/BytecodeCompiler.hpp"
#include "../../include/backends/vm/VirtualMachine.hpp"
#include "../../include/parser/Expression.hpp"
#include "../../include/parser/Statement.hpp"
#include "../../include/lexer/Token.hpp"

class CompilerVMIntegrationTest : public ::testing::Test {
protected:
    BytecodeCompiler compiler;
    VirtualMachine vm;

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

    // Helper to wrap expression in a Program statement for compilation
    std::unique_ptr<Statement> wrapInProgram(std::unique_ptr<Expression> expr) {
        auto exprStmt = std::make_unique<ExpressionStatement>(std::move(expr));
        std::vector<std::unique_ptr<Statement>> statements;
        statements.push_back(std::move(exprStmt));
        return std::make_unique<Program>(std::move(statements));
    }

    // Helper to compile and load program into VM
    void compileAndLoad(std::unique_ptr<Expression> expr) {
        auto program = compiler.compile(wrapInProgram(std::move(expr)));
        vm.loadProgram(program.instructions, program.constants, program.strings);
    }
};

// ============================================================================
// BASIC LITERAL TESTS
// ============================================================================

TEST_F(CompilerVMIntegrationTest, CompileAndExecuteIntLiteral) {
    auto expr = makeLiteral(makeIntToken(42));
    compileAndLoad(std::move(expr));

    vm.run();

    // VM should have the value on top of stack
    int result = vm.popInt();
    EXPECT_EQ(result, 42);
}

TEST_F(CompilerVMIntegrationTest, CompileAndExecuteFloatLiteral) {
    auto expr = makeLiteral(makeFloatToken(3.14f));
    compileAndLoad(std::move(expr));

    vm.run();

    float result = vm.popFloat();
    EXPECT_FLOAT_EQ(result, 3.14f);
}

TEST_F(CompilerVMIntegrationTest, CompileAndExecuteDoubleLiteral) {
    auto expr = makeLiteral(makeDoubleToken(2.718));
    compileAndLoad(std::move(expr));

    vm.run();

    double result = vm.popDouble();
    EXPECT_DOUBLE_EQ(result, 2.718);
}

TEST_F(CompilerVMIntegrationTest, CompileAndExecuteBoolLiteral) {
    auto expr = makeLiteral(makeBoolToken(true));
    compileAndLoad(std::move(expr));

    vm.run();

    bool result = vm.popBool();
    EXPECT_EQ(result, true);
}

// ============================================================================
// BINARY ARITHMETIC TESTS
// ============================================================================

TEST_F(CompilerVMIntegrationTest, CompileAndExecuteIntAddition) {
    // Test: 5 + 3
    auto left = makeLiteral(makeIntToken(5));
    auto right = makeLiteral(makeIntToken(3));
    auto expr = makeBinary(std::move(left), makeOperatorToken(TokenType::PLUS), std::move(right));

    compileAndLoad(std::move(expr));
    vm.run();

    int result = vm.popInt();
    EXPECT_EQ(result, 8);
}

TEST_F(CompilerVMIntegrationTest, CompileAndExecuteIntSubtraction) {
    // Test: 10 - 4
    auto left = makeLiteral(makeIntToken(10));
    auto right = makeLiteral(makeIntToken(4));
    auto expr = makeBinary(std::move(left), makeOperatorToken(TokenType::MINUS), std::move(right));

    compileAndLoad(std::move(expr));
    vm.run();

    int result = vm.popInt();
    EXPECT_EQ(result, 6);
}

TEST_F(CompilerVMIntegrationTest, CompileAndExecuteIntMultiplication) {
    // Test: 6 * 7
    auto left = makeLiteral(makeIntToken(6));
    auto right = makeLiteral(makeIntToken(7));
    auto expr = makeBinary(std::move(left), makeOperatorToken(TokenType::MULT), std::move(right));

    compileAndLoad(std::move(expr));
    vm.run();

    int result = vm.popInt();
    EXPECT_EQ(result, 42);
}

TEST_F(CompilerVMIntegrationTest, CompileAndExecuteIntDivision) {
    // Test: 15 / 3
    auto left = makeLiteral(makeIntToken(15));
    auto right = makeLiteral(makeIntToken(3));
    auto expr = makeBinary(std::move(left), makeOperatorToken(TokenType::DIV), std::move(right));

    compileAndLoad(std::move(expr));
    vm.run();

    int result = vm.popInt();
    EXPECT_EQ(result, 5);
}

TEST_F(CompilerVMIntegrationTest, CompileAndExecuteFloatArithmetic) {
    // Test: 2.5 + 1.5
    auto left = makeLiteral(makeFloatToken(2.5f));
    auto right = makeLiteral(makeFloatToken(1.5f));
    auto expr = makeBinary(std::move(left), makeOperatorToken(TokenType::PLUS), std::move(right));

    compileAndLoad(std::move(expr));
    vm.run();

    float result = vm.popFloat();
    EXPECT_FLOAT_EQ(result, 4.0f);
}

TEST_F(CompilerVMIntegrationTest, CompileAndExecuteDoubleArithmetic) {
    // Test: 10.5 - 2.5
    auto left = makeLiteral(makeDoubleToken(10.5));
    auto right = makeLiteral(makeDoubleToken(2.5));
    auto expr = makeBinary(std::move(left), makeOperatorToken(TokenType::MINUS), std::move(right));

    compileAndLoad(std::move(expr));
    vm.run();

    double result = vm.popDouble();
    EXPECT_DOUBLE_EQ(result, 8.0);
}

// ============================================================================
// COMPARISON TESTS
// ============================================================================

TEST_F(CompilerVMIntegrationTest, CompileAndExecuteIntEquality) {
    // Test: 5 == 5
    auto left = makeLiteral(makeIntToken(5));
    auto right = makeLiteral(makeIntToken(5));
    auto expr = makeBinary(std::move(left), makeOperatorToken(TokenType::EQUAL_EQUAL), std::move(right));

    compileAndLoad(std::move(expr));
    vm.run();

    bool result = vm.popBool();
    EXPECT_EQ(result, true);
}

TEST_F(CompilerVMIntegrationTest, CompileAndExecuteIntInequality) {
    // Test: 3 == 5
    auto left = makeLiteral(makeIntToken(3));
    auto right = makeLiteral(makeIntToken(5));
    auto expr = makeBinary(std::move(left), makeOperatorToken(TokenType::EQUAL_EQUAL), std::move(right));

    compileAndLoad(std::move(expr));
    vm.run();

    bool result = vm.popBool();
    EXPECT_EQ(result, false);
}

TEST_F(CompilerVMIntegrationTest, CompileAndExecuteIntLessThan) {
    // Test: 3 < 5
    auto left = makeLiteral(makeIntToken(3));
    auto right = makeLiteral(makeIntToken(5));
    auto expr = makeBinary(std::move(left), makeOperatorToken(TokenType::LESS), std::move(right));

    compileAndLoad(std::move(expr));
    vm.run();

    bool result = vm.popBool();
    EXPECT_EQ(result, true);
}

// ============================================================================
// UNARY OPERATION TESTS
// ============================================================================

TEST_F(CompilerVMIntegrationTest, CompileAndExecuteIntNegation) {
    // Test: -42
    auto operand = makeLiteral(makeIntToken(42));
    auto expr = makeUnary(makeOperatorToken(TokenType::MINUS), std::move(operand));

    compileAndLoad(std::move(expr));
    vm.run();

    int result = vm.popInt();
    EXPECT_EQ(result, -42);
}

TEST_F(CompilerVMIntegrationTest, CompileAndExecuteFloatNegation) {
    // Test: -3.14
    auto operand = makeLiteral(makeFloatToken(3.14f));
    auto expr = makeUnary(makeOperatorToken(TokenType::MINUS), std::move(operand));

    compileAndLoad(std::move(expr));
    vm.run();

    float result = vm.popFloat();
    EXPECT_FLOAT_EQ(result, -3.14f);
}

TEST_F(CompilerVMIntegrationTest, CompileAndExecuteBoolNot) {
    // Test: !true
    auto operand = makeLiteral(makeBoolToken(true));
    auto expr = makeUnary(makeOperatorToken(TokenType::NOT), std::move(operand));

    compileAndLoad(std::move(expr));
    vm.run();

    bool result = vm.popBool();
    EXPECT_EQ(result, false);
}

// ============================================================================
// COMPLEX EXPRESSION TESTS
// ============================================================================

TEST_F(CompilerVMIntegrationTest, CompileAndExecuteNestedArithmetic) {
    // Test: (5 + 3) * 2 = 16
    auto five = makeLiteral(makeIntToken(5));
    auto three = makeLiteral(makeIntToken(3));
    auto addition = makeBinary(std::move(five), makeOperatorToken(TokenType::PLUS), std::move(three));

    auto two = makeLiteral(makeIntToken(2));
    auto multiplication = makeBinary(std::move(addition), makeOperatorToken(TokenType::MULT), std::move(two));

    compileAndLoad(std::move(multiplication));
    vm.run();

    int result = vm.popInt();
    EXPECT_EQ(result, 16);
}

TEST_F(CompilerVMIntegrationTest, CompileAndExecuteNestedWithUnary) {
    // Test: -5 + 3 = -2
    auto five = makeLiteral(makeIntToken(5));
    auto negation = makeUnary(makeOperatorToken(TokenType::MINUS), std::move(five));

    auto three = makeLiteral(makeIntToken(3));
    auto addition = makeBinary(std::move(negation), makeOperatorToken(TokenType::PLUS), std::move(three));

    compileAndLoad(std::move(addition));
    vm.run();

    int result = vm.popInt();
    EXPECT_EQ(result, -2);
}

TEST_F(CompilerVMIntegrationTest, CompileAndExecuteComplexExpression) {
    // Test: (10 - 2) / (3 + 1) = 2
    auto ten = makeLiteral(makeIntToken(10));
    auto two = makeLiteral(makeIntToken(2));
    auto subtraction = makeBinary(std::move(ten), makeOperatorToken(TokenType::MINUS), std::move(two));

    auto three = makeLiteral(makeIntToken(3));
    auto one = makeLiteral(makeIntToken(1));
    auto addition = makeBinary(std::move(three), makeOperatorToken(TokenType::PLUS), std::move(one));

    auto division = makeBinary(std::move(subtraction), makeOperatorToken(TokenType::DIV), std::move(addition));

    compileAndLoad(std::move(division));
    vm.run();

    int result = vm.popInt();
    EXPECT_EQ(result, 2);
}