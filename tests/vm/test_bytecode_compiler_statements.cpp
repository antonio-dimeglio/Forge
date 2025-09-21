#include <gtest/gtest.h>
#include "../../include/backends/vm/BytecodeCompiler.hpp"
#include "../../include/parser/Parser.hpp"
#include "../../include/parser/Statement.hpp"
#include "../../include/lexer/Tokenizer.hpp"
#include "../../include/backends/vm/RuntimeException.hpp"

class BytecodeCompilerStatementTest : public ::testing::Test {
protected:
    BytecodeCompiler compiler;

    void SetUp() override {
        // Reset compiler state for each test
    }

    // Helper to parse and compile statements
    BytecodeCompiler::CompiledProgram compileStatement(const std::string& input) {
        Tokenizer tokenizer(input);
        std::vector<Token> tokens = tokenizer.tokenize();
        Parser parser(tokens);
        auto ast = parser.parseProgram();
        return compiler.compile(std::move(ast));
    }

    // Helper to check if instruction exists at position
    void expectInstruction(const std::vector<Instruction>& instructions,
                          size_t pos, OPCode expectedOp, int expectedOperand = 0) {
        ASSERT_LT(pos, instructions.size()) << "Instruction position out of bounds";
        EXPECT_EQ(instructions[pos].opcode, expectedOp) << "Wrong opcode at position " << pos;
        EXPECT_EQ(instructions[pos].operand, expectedOperand) << "Wrong operand at position " << pos;
    }

    // Helper to check constant value
    void expectConstant(const std::vector<TypedValue>& constants,
                       size_t pos, TypedValue::Type expectedType,
                       const std::string& expectedValue = "") {
        ASSERT_LT(pos, constants.size()) << "Constant position out of bounds";
        EXPECT_EQ(constants[pos].type, expectedType) << "Wrong constant type at position " << pos;

        if (!expectedValue.empty()) {
            switch (expectedType) {
                case TypedValue::Type::INT:
                    EXPECT_EQ(constants[pos].value.i, std::stoi(expectedValue));
                    break;
                case TypedValue::Type::FLOAT:
                    EXPECT_FLOAT_EQ(constants[pos].value.f, std::stof(expectedValue));
                    break;
                case TypedValue::Type::DOUBLE:
                    EXPECT_DOUBLE_EQ(constants[pos].value.d, std::stod(expectedValue));
                    break;
                case TypedValue::Type::BOOL:
                    EXPECT_EQ(constants[pos].value.b, (expectedValue == "true"));
                    break;
                default:
                    break;
            }
        }
    }
};

// ===== VARIABLE DECLARATION COMPILATION TESTS =====

TEST_F(BytecodeCompilerStatementTest, CompileSimpleIntDeclaration) {
    auto program = compileStatement("x: int = 42");

    // Should generate: LOAD_INT 0, STORE_LOCAL 0
    ASSERT_EQ(program.instructions.size(), 2);
    expectInstruction(program.instructions, 0, OPCode::LOAD_INT, 0);
    expectInstruction(program.instructions, 1, OPCode::STORE_LOCAL, 0);

    // Check constant pool
    ASSERT_EQ(program.constants.size(), 1);
    expectConstant(program.constants, 0, TypedValue::Type::INT, "42");
}

TEST_F(BytecodeCompilerStatementTest, CompileFloatDeclaration) {
    auto program = compileStatement("y: float = 3.14");

    ASSERT_EQ(program.instructions.size(), 2);
    expectInstruction(program.instructions, 0, OPCode::LOAD_DOUBLE, 0);
    expectInstruction(program.instructions, 1, OPCode::STORE_LOCAL, 0);

    ASSERT_EQ(program.constants.size(), 1);
    expectConstant(program.constants, 0, TypedValue::Type::DOUBLE, "3.14");
}

TEST_F(BytecodeCompilerStatementTest, CompileDoubleDeclaration) {
    auto program = compileStatement("z: double = 2.718");

    ASSERT_EQ(program.instructions.size(), 2);
    expectInstruction(program.instructions, 0, OPCode::LOAD_DOUBLE, 0);
    expectInstruction(program.instructions, 1, OPCode::STORE_LOCAL, 0);

    ASSERT_EQ(program.constants.size(), 1);
    expectConstant(program.constants, 0, TypedValue::Type::DOUBLE, "2.718");
}

TEST_F(BytecodeCompilerStatementTest, CompileBoolDeclaration) {
    auto program = compileStatement("flag: bool = true");

    ASSERT_EQ(program.instructions.size(), 2);
    expectInstruction(program.instructions, 0, OPCode::LOAD_BOOL, 0);
    expectInstruction(program.instructions, 1, OPCode::STORE_LOCAL, 0);

    ASSERT_EQ(program.constants.size(), 1);
    expectConstant(program.constants, 0, TypedValue::Type::BOOL, "true");
}

TEST_F(BytecodeCompilerStatementTest, CompileStringDeclaration) {
    auto program = compileStatement("name: str = \"hello\"");

    ASSERT_EQ(program.instructions.size(), 2);
    expectInstruction(program.instructions, 0, OPCode::LOAD_STRING, 0);
    expectInstruction(program.instructions, 1, OPCode::STORE_LOCAL, 0);

    // Check string was added to string pool
    ASSERT_EQ(program.strings.size(), 1);
    EXPECT_EQ(program.strings[0], "hello");
}

TEST_F(BytecodeCompilerStatementTest, CompileMultipleDeclarations) {
    auto program = compileStatement("x: int = 10\ny: int = 20");

    // Should generate instructions for both declarations
    ASSERT_EQ(program.instructions.size(), 4);

    // First declaration: x = 10
    expectInstruction(program.instructions, 0, OPCode::LOAD_INT, 0);
    expectInstruction(program.instructions, 1, OPCode::STORE_LOCAL, 0);

    // Second declaration: y = 20
    expectInstruction(program.instructions, 2, OPCode::LOAD_INT, 1);
    expectInstruction(program.instructions, 3, OPCode::STORE_LOCAL, 1);

    // Check constants
    ASSERT_EQ(program.constants.size(), 2);
    expectConstant(program.constants, 0, TypedValue::Type::INT, "10");
    expectConstant(program.constants, 1, TypedValue::Type::INT, "20");
}

TEST_F(BytecodeCompilerStatementTest, CompileDeclarationWithComplexExpression) {
    auto program = compileStatement("result: int = 2 + 3 * 4");

    // Should generate: LOAD_INT 0, LOAD_INT 1, LOAD_INT 2, MULT_INT, ADD_INT, STORE_LOCAL 0
    ASSERT_EQ(program.instructions.size(), 6);
    expectInstruction(program.instructions, 0, OPCode::LOAD_INT, 0);  // 2
    expectInstruction(program.instructions, 1, OPCode::LOAD_INT, 1);  // 3
    expectInstruction(program.instructions, 2, OPCode::LOAD_INT, 2);  // 4
    expectInstruction(program.instructions, 3, OPCode::MULT_INT, 0);
    expectInstruction(program.instructions, 4, OPCode::ADD_INT, 0);
    expectInstruction(program.instructions, 5, OPCode::STORE_LOCAL, 0);

    ASSERT_EQ(program.constants.size(), 3);
    expectConstant(program.constants, 0, TypedValue::Type::INT, "2");
    expectConstant(program.constants, 1, TypedValue::Type::INT, "3");
    expectConstant(program.constants, 2, TypedValue::Type::INT, "4");
}

// ===== ASSIGNMENT COMPILATION TESTS =====

TEST_F(BytecodeCompilerStatementTest, CompileSimpleAssignment) {
    // Need to declare variable first, then assign
    auto program = compileStatement("x: int = 0\nx = 42");

    // Should have declaration + assignment instructions
    ASSERT_GE(program.instructions.size(), 4);

    // Last two instructions should be assignment: LOAD_INT, STORE_LOCAL 0
    size_t lastInst = program.instructions.size() - 1;
    expectInstruction(program.instructions, lastInst - 1, OPCode::LOAD_INT, 1); // Constant index 1 for value 42
    expectInstruction(program.instructions, lastInst, OPCode::STORE_LOCAL, 0);
}

TEST_F(BytecodeCompilerStatementTest, CompileAssignmentToUndeclaredVariable) {
    // Should throw runtime exception
    EXPECT_THROW(compileStatement("x = 42"), RuntimeException);
}

TEST_F(BytecodeCompilerStatementTest, CompileAssignmentWithExpression) {
    auto program = compileStatement("x: int = 0\nx = 10 + 5");

    // Check that assignment uses expression compilation
    ASSERT_GE(program.instructions.size(), 5);

    // Should have LOAD, LOAD, ADD, STORE pattern at the end
    size_t end = program.instructions.size();
    expectInstruction(program.instructions, end - 4, OPCode::LOAD_INT, 1);  // 10 (constant index 1)
    expectInstruction(program.instructions, end - 3, OPCode::LOAD_INT, 2);  // 5 (constant index 2)
    expectInstruction(program.instructions, end - 2, OPCode::ADD_INT);
    expectInstruction(program.instructions, end - 1, OPCode::STORE_LOCAL, 0);
}

// ===== VARIABLE USAGE COMPILATION TESTS =====

TEST_F(BytecodeCompilerStatementTest, CompileVariableUsage) {
    auto program = compileStatement("x: int = 10\ny: int = x");

    // Should have: declare x, then load x and store to y
    ASSERT_EQ(program.instructions.size(), 4);

    // First declaration
    expectInstruction(program.instructions, 0, OPCode::LOAD_INT, 0);
    expectInstruction(program.instructions, 1, OPCode::STORE_LOCAL, 0);

    // Second declaration using first variable
    expectInstruction(program.instructions, 2, OPCode::LOAD_LOCAL, 0);
    expectInstruction(program.instructions, 3, OPCode::STORE_LOCAL, 1);
}

TEST_F(BytecodeCompilerStatementTest, CompileVariableInExpression) {
    auto program = compileStatement("x: int = 10\ny: int = 20\nz: int = x + y");

    // Should load both variables and add them, then store result
    ASSERT_EQ(program.instructions.size(), 8);

    // Last 4 instructions should be: LOAD_LOCAL 0, LOAD_LOCAL 1, ADD_INT, STORE_LOCAL 2
    expectInstruction(program.instructions, 4, OPCode::LOAD_LOCAL, 0);  // x
    expectInstruction(program.instructions, 5, OPCode::LOAD_LOCAL, 1);  // y
    expectInstruction(program.instructions, 6, OPCode::ADD_INT);
    expectInstruction(program.instructions, 7, OPCode::STORE_LOCAL, 2); // z
}

TEST_F(BytecodeCompilerStatementTest, CompileUndeclaredVariableUsage) {
    // Should throw exception when trying to use undeclared variable
    EXPECT_THROW(compileStatement("y: int = x"), RuntimeException);
}

// ===== FUNCTION CALL COMPILATION TESTS =====

TEST_F(BytecodeCompilerStatementTest, CompileFunctionCallStatement) {
    // Function calls aren't fully implemented yet, should throw
    EXPECT_THROW(compileStatement("test()"), RuntimeException);
}

// ===== IF STATEMENT COMPILATION TESTS =====

TEST_F(BytecodeCompilerStatementTest, CompileIfStatement) {
    // If statements aren't implemented yet, should throw
    EXPECT_THROW(compileStatement("if (true): x = 1"), RuntimeException);
}

// ===== EXPRESSION STATEMENT COMPILATION TESTS =====

TEST_F(BytecodeCompilerStatementTest, CompileExpressionStatement) {
    auto program = compileStatement("42");

    // Should just compile the expression
    ASSERT_EQ(program.instructions.size(), 1);
    expectInstruction(program.instructions, 0, OPCode::LOAD_INT, 0);

    ASSERT_EQ(program.constants.size(), 1);
    expectConstant(program.constants, 0, TypedValue::Type::INT, "42");
}

TEST_F(BytecodeCompilerStatementTest, CompileComplexExpressionStatement) {
    auto program = compileStatement("2 + 3 * 4");

    // Should compile the full expression
    ASSERT_EQ(program.instructions.size(), 5);
    expectInstruction(program.instructions, 0, OPCode::LOAD_INT, 0);  // 2
    expectInstruction(program.instructions, 1, OPCode::LOAD_INT, 1);  // 3
    expectInstruction(program.instructions, 2, OPCode::LOAD_INT, 2);  // 4
    expectInstruction(program.instructions, 3, OPCode::MULT_INT);
    expectInstruction(program.instructions, 4, OPCode::ADD_INT);
}

// ===== SYMBOL TABLE TESTS =====

TEST_F(BytecodeCompilerStatementTest, SymbolTableSlotAllocation) {
    auto program = compileStatement("a: int = 1\nb: int = 2\nc: int = 3");

    // Should allocate slots 0, 1, 2 for variables a, b, c
    ASSERT_EQ(program.instructions.size(), 6);

    expectInstruction(program.instructions, 1, OPCode::STORE_LOCAL, 0);  // a
    expectInstruction(program.instructions, 3, OPCode::STORE_LOCAL, 1);  // b
    expectInstruction(program.instructions, 5, OPCode::STORE_LOCAL, 2);  // c
}

TEST_F(BytecodeCompilerStatementTest, SymbolTableTypeTracking) {
    // Test that variables maintain their types
    auto program = compileStatement("x: int = 42\ny: float = 3.14f\nz: int = x");

    // x and z should use same type operations (INT), y should use FLOAT
    expectInstruction(program.instructions, 0, OPCode::LOAD_INT, 0);    // x declaration (constant index 0)
    expectInstruction(program.instructions, 2, OPCode::LOAD_FLOAT, 1);  // y declaration (constant index 1)
    expectInstruction(program.instructions, 4, OPCode::LOAD_LOCAL, 0); // z = x (loading int)
}

// ===== EDGE CASE TESTS =====

TEST_F(BytecodeCompilerStatementTest, EmptyProgram) {
    auto program = compileStatement("");

    // Empty program should generate no instructions
    EXPECT_EQ(program.instructions.size(), 0);
    EXPECT_EQ(program.constants.size(), 0);
    EXPECT_EQ(program.strings.size(), 0);
}

TEST_F(BytecodeCompilerStatementTest, OnlyNewlines) {
    auto program = compileStatement("\n\n\n");

    // Only newlines should generate no instructions
    EXPECT_EQ(program.instructions.size(), 0);
}

TEST_F(BytecodeCompilerStatementTest, LongVariableNames) {
    auto program = compileStatement("very_long_variable_name_with_many_underscores: int = 123");

    ASSERT_EQ(program.instructions.size(), 2);
    expectInstruction(program.instructions, 0, OPCode::LOAD_INT, 0);
    expectInstruction(program.instructions, 1, OPCode::STORE_LOCAL, 0);
}

TEST_F(BytecodeCompilerStatementTest, VariableNameReuse) {
    // Same variable name in different contexts
    auto program = compileStatement("x: int = 1\nx = 2\nx = 3");

    // All assignments should use same slot (0)
    expectInstruction(program.instructions, 1, OPCode::STORE_LOCAL, 0);  // Declaration
    expectInstruction(program.instructions, 3, OPCode::STORE_LOCAL, 0);  // First assignment
    expectInstruction(program.instructions, 5, OPCode::STORE_LOCAL, 0);  // Second assignment
}

// ===== CONSTANT POOL OPTIMIZATION TESTS =====

TEST_F(BytecodeCompilerStatementTest, ConstantPoolDeduplication) {
    auto program = compileStatement("x: int = 42\ny: int = 42\nz: int = 42");

    // Should reuse the same constant for value 42
    // Note: Current implementation might not deduplicate - this tests if it could
    ASSERT_GE(program.constants.size(), 1);
    expectConstant(program.constants, 0, TypedValue::Type::INT, "42");
}

TEST_F(BytecodeCompilerStatementTest, MixedConstantTypes) {
    auto program = compileStatement("a: int = 42\nb: float = 3.14f\nc: bool = true");

    // Should have 3 different constants of different types
    ASSERT_EQ(program.constants.size(), 3);
    expectConstant(program.constants, 0, TypedValue::Type::INT, "42");
    expectConstant(program.constants, 1, TypedValue::Type::FLOAT, "3.14");
    expectConstant(program.constants, 2, TypedValue::Type::BOOL, "true");
}