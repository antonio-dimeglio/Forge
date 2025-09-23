#include <gtest/gtest.h>
#include "../../include/backends/vm/BytecodeCompiler.hpp"
#include "../../include/backends/vm/VirtualMachine.hpp"
#include "../../include/parser/Parser.hpp"
#include "../../include/lexer/Tokenizer.hpp"

class BytecodeCompilerTest : public ::testing::Test {
protected:
    VirtualMachine vm;
    BytecodeCompiler compiler;

    BytecodeCompilerTest() : compiler(vm) {}

    void SetUp() override {
        vm.reset();
    }

    // Helper to compile from source code
    BytecodeCompiler::CompiledProgram compileSource(const std::string& source) {
        Tokenizer tokenizer(source);
        auto tokens = tokenizer.tokenize();
        Parser parser(tokens);
        auto ast = parser.parseProgram();
        return compiler.compile(std::move(ast));
    }

    // Helper to run compiled program
    void runProgram(const BytecodeCompiler::CompiledProgram& program) {
        vm.loadProgram(program.instructions, program.constants);
        vm.run();
    }
};

// ========== LITERAL COMPILATION TESTS ==========

TEST_F(BytecodeCompilerTest, CompileIntegerLiteral) {
    auto program = compileSource("42");

    ASSERT_GE(program.instructions.size(), 2);
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_INT);
    EXPECT_EQ(program.instructions[0].operand, 0);
    EXPECT_EQ(program.instructions.back().opcode, OPCode::HALT);

    ASSERT_EQ(program.constants.size(), 1);
    EXPECT_EQ(program.constants[0].type, ValueType::INT);
    EXPECT_EQ(asInt(program.constants[0]), 42);
}

TEST_F(BytecodeCompilerTest, CompileFloatLiteral) {
    auto program = compileSource("3.14f");

    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_FLOAT);
    ASSERT_EQ(program.constants.size(), 1);
    EXPECT_EQ(program.constants[0].type, ValueType::FLOAT);
    EXPECT_FLOAT_EQ(asFloat(program.constants[0]), 3.14f);
}

TEST_F(BytecodeCompilerTest, CompileDoubleLiteral) {
    auto program = compileSource("2.718");

    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_DOUBLE);
    ASSERT_EQ(program.constants.size(), 1);
    EXPECT_EQ(program.constants[0].type, ValueType::DOUBLE);
    EXPECT_DOUBLE_EQ(asDouble(program.constants[0]), 2.718);
}

TEST_F(BytecodeCompilerTest, CompileBooleanLiterals) {
    auto program = compileSource("true");
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_BOOL);
    EXPECT_EQ(asBool(program.constants[0]), true);

    program = compileSource("false");
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_BOOL);
    EXPECT_EQ(asBool(program.constants[0]), false);
}

TEST_F(BytecodeCompilerTest, CompileStringLiteral) {
    auto program = compileSource("\"hello world\"");

    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_STRING);
    ASSERT_EQ(program.constants.size(), 1);
    EXPECT_EQ(program.constants[0].type, ValueType::OBJECT);

    StringObject* str = asString(program.constants[0]);
    EXPECT_STREQ(str->chars, "hello world");
    EXPECT_EQ(str->length, 11);
}

// ========== BINARY EXPRESSION COMPILATION TESTS ==========

TEST_F(BytecodeCompilerTest, CompileSimpleAddition) {
    auto program = compileSource("5 + 3");

    // Should load 5, load 3, then add
    ASSERT_GE(program.instructions.size(), 4);
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_INT);
    EXPECT_EQ(program.instructions[1].opcode, OPCode::LOAD_INT);
    EXPECT_EQ(program.instructions[2].opcode, OPCode::ADD_INT);

    ASSERT_EQ(program.constants.size(), 2);
    EXPECT_EQ(asInt(program.constants[0]), 5);
    EXPECT_EQ(asInt(program.constants[1]), 3);
}

TEST_F(BytecodeCompilerTest, CompileArithmeticOperations) {
    // Test all arithmetic operations
    auto program = compileSource("10 - 4");
    EXPECT_EQ(program.instructions[2].opcode, OPCode::SUB_INT);

    program = compileSource("6 * 7");
    EXPECT_EQ(program.instructions[2].opcode, OPCode::MULT_INT);

    program = compileSource("20 / 5");
    EXPECT_EQ(program.instructions[2].opcode, OPCode::DIV_INT);
}

TEST_F(BytecodeCompilerTest, CompileFloatArithmetic) {
    auto program = compileSource("3.5f + 2.1f");

    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_FLOAT);
    EXPECT_EQ(program.instructions[1].opcode, OPCode::LOAD_FLOAT);
    EXPECT_EQ(program.instructions[2].opcode, OPCode::ADD_FLOAT);
}

TEST_F(BytecodeCompilerTest, CompileComparisonOperations) {
    auto program = compileSource("5 == 5");
    EXPECT_EQ(program.instructions[2].opcode, OPCode::EQ_INT);

    program = compileSource("10 > 3");
    EXPECT_EQ(program.instructions[2].opcode, OPCode::GT_INT);

    program = compileSource("2 < 8");
    EXPECT_EQ(program.instructions[2].opcode, OPCode::LT_INT);

    program = compileSource("5 >= 5");
    EXPECT_EQ(program.instructions[2].opcode, OPCode::GEQ_INT);

    program = compileSource("3 <= 7");
    EXPECT_EQ(program.instructions[2].opcode, OPCode::LEQ_INT);
}

TEST_F(BytecodeCompilerTest, CompileUnaryOperations) {
    auto program = compileSource("-42");
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_INT);
    EXPECT_EQ(program.instructions[1].opcode, OPCode::NEG_INT);

    program = compileSource("!true");
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_BOOL);
    EXPECT_EQ(program.instructions[1].opcode, OPCode::NOT_BOOL);
}

// ========== VARIABLE COMPILATION TESTS ==========

TEST_F(BytecodeCompilerTest, CompileGlobalVariableDeclaration) {
    auto program = compileSource("x: int = 42");

    // Should load 42, then store to global slot 0
    EXPECT_EQ(program.instructions[0].opcode, OPCode::LOAD_INT);
    EXPECT_EQ(program.instructions[1].opcode, OPCode::STORE_GLOBAL);
    EXPECT_EQ(program.instructions[1].operand, 0);
}

TEST_F(BytecodeCompilerTest, CompileLocalVariableDeclaration) {
    auto program = compileSource(R"(
def test() -> int {
    x: int = 10
    return x
}
)");

    // Function should contain local variable operations
    ASSERT_GE(program.constants.size(), 1);
    auto func = asFunction(program.constants[0]);

    // Look for STORE_LOCAL and LOAD_LOCAL in function instructions
    bool hasStoreLocal = false;
    bool hasLoadLocal = false;

    for (const auto& inst : func->instructions) {
        if (inst.opcode == OPCode::STORE_LOCAL) hasStoreLocal = true;
        if (inst.opcode == OPCode::LOAD_LOCAL) hasLoadLocal = true;
    }

    EXPECT_TRUE(hasStoreLocal);
    EXPECT_TRUE(hasLoadLocal);
}

TEST_F(BytecodeCompilerTest, CompileVariableAssignment) {
    auto program = compileSource(R"(
x: int = 5
x = 10
)");

    // Should have STORE_GLOBAL twice
    int storeGlobalCount = 0;
    for (const auto& inst : program.instructions) {
        if (inst.opcode == OPCode::STORE_GLOBAL) storeGlobalCount++;
    }
    EXPECT_EQ(storeGlobalCount, 2);
}

TEST_F(BytecodeCompilerTest, CompileVariableAccess) {
    auto program = compileSource(R"(
x: int = 42
y: int = x
)");

    // Should load from global variable x
    bool hasLoadGlobal = false;
    for (const auto& inst : program.instructions) {
        if (inst.opcode == OPCode::LOAD_GLOBAL) hasLoadGlobal = true;
    }
    EXPECT_TRUE(hasLoadGlobal);
}

// ========== FUNCTION COMPILATION TESTS ==========

TEST_F(BytecodeCompilerTest, CompileSimpleFunction) {
    auto program = compileSource(R"(
def add(x: int, y: int) -> int {
    return x + y
}
)");

    // Should create a function object in constants
    ASSERT_GE(program.constants.size(), 1);
    auto func = asFunction(program.constants[0]);

    EXPECT_EQ(func->name, "add");
    EXPECT_EQ(func->parameterCount, 2);
    EXPECT_GT(func->instructions.size(), 0);
}

TEST_F(BytecodeCompilerTest, CompileFunctionCall) {
    auto program = compileSource(R"(
def square(x: int) -> int {
    return x * x
}

result: int = square(5)
)");

    // Should have CALL instruction
    bool hasCall = false;
    for (const auto& inst : program.instructions) {
        if (inst.opcode == OPCode::CALL) hasCall = true;
    }
    EXPECT_TRUE(hasCall);
}

TEST_F(BytecodeCompilerTest, CompileRecursiveFunction) {
    auto program = compileSource(R"(
def factorial(n: int) -> int {
    if (n <= 1) {
        return 1
    }
    return n * factorial(n - 1)
}
)");

    // Should compile without errors and have recursive call
    ASSERT_GE(program.constants.size(), 1);
    auto func = asFunction(program.constants[0]);
    EXPECT_EQ(func->name, "factorial");

    // Should have at least one CALL instruction in function body
    bool hasCall = false;
    for (const auto& inst : func->instructions) {
        if (inst.opcode == OPCode::CALL) hasCall = true;
    }
    EXPECT_TRUE(hasCall);
}

// ========== CONTROL FLOW COMPILATION TESTS ==========

TEST_F(BytecodeCompilerTest, CompileIfStatement) {
    auto program = compileSource(R"(
x: int = 5
if (x > 0) {
    x = 10
}
)");

    // Should have JUMP_IF_FALSE instruction
    bool hasJumpIfFalse = false;
    for (const auto& inst : program.instructions) {
        if (inst.opcode == OPCode::JUMP_IF_FALSE) hasJumpIfFalse = true;
    }
    EXPECT_TRUE(hasJumpIfFalse);
}

TEST_F(BytecodeCompilerTest, CompileIfElseStatement) {
    auto program = compileSource(R"(
x: int = 5
if (x > 10) {
    x = 1
} else {
    x = 2
}
)");

    // Should have both JUMP_IF_FALSE and JUMP instructions
    bool hasJumpIfFalse = false;
    bool hasJump = false;

    for (const auto& inst : program.instructions) {
        if (inst.opcode == OPCode::JUMP_IF_FALSE) hasJumpIfFalse = true;
        if (inst.opcode == OPCode::JUMP) hasJump = true;
    }

    EXPECT_TRUE(hasJumpIfFalse);
    EXPECT_TRUE(hasJump);
}

TEST_F(BytecodeCompilerTest, CompileWhileLoop) {
    auto program = compileSource(R"(
i: int = 0
while (i < 5) {
    i = i + 1
}
)");

    // Should have JUMP_IF_FALSE and JUMP instructions
    bool hasJumpIfFalse = false;
    bool hasJump = false;

    for (const auto& inst : program.instructions) {
        if (inst.opcode == OPCode::JUMP_IF_FALSE) hasJumpIfFalse = true;
        if (inst.opcode == OPCode::JUMP) hasJump = true;
    }

    EXPECT_TRUE(hasJumpIfFalse);
    EXPECT_TRUE(hasJump);
}

// ========== ERROR HANDLING TESTS ==========

TEST_F(BytecodeCompilerTest, ThrowsOnUndefinedVariable) {
    EXPECT_THROW({
        compileSource("x = y");  // y is undefined
    }, RuntimeException);
}

TEST_F(BytecodeCompilerTest, ThrowsOnTypeMismatch) {
    EXPECT_THROW({
        compileSource("5 + \"hello\"");  // Type mismatch
    }, RuntimeException);
}

TEST_F(BytecodeCompilerTest, ThrowsOnUndefinedFunction) {
    EXPECT_THROW({
        compileSource("result: int = unknownFunction()");
    }, RuntimeException);
}

// ========== COMPLEX PROGRAM TESTS ==========

TEST_F(BytecodeCompilerTest, CompileComplexProgram) {
    auto program = compileSource(R"(
counter: int = 0

def increment() -> int {
    counter = counter + 1
    return counter
}

def main() -> int {
    first: int = increment()
    second: int = increment()
    return first + second
}

result: int = main()
)");

    // Should compile successfully
    EXPECT_GT(program.instructions.size(), 0);
    EXPECT_GT(program.constants.size(), 0);

    // Should have multiple functions
    int functionCount = 0;
    for (const auto& constant : program.constants) {
        if (constant.type == ValueType::OBJECT && isFunction(constant)) {
            functionCount++;
        }
    }
    EXPECT_GE(functionCount, 2);  // increment and main functions
}

TEST_F(BytecodeCompilerTest, CompileNestedScoping) {
    auto program = compileSource(R"(
global: int = 100

def test(param: int) -> int {
    local: int = 10
    if (param > 0) {
        block: int = 5
        return global + param + local + block
    }
    return global + param + local
}
)");

    // Should compile successfully with nested scoping
    EXPECT_GT(program.instructions.size(), 0);
    ASSERT_GE(program.constants.size(), 2);

    // Find the function object in constants (it's not necessarily at index 0)
    FunctionObject* func = nullptr;
    for (const auto& constant : program.constants) {
        if (constant.type == ValueType::OBJECT && isFunction(constant)) {
            func = asFunction(constant);
            break;
        }
    }
    ASSERT_NE(func, nullptr);
    EXPECT_EQ(func->name, "test");
    EXPECT_GT(func->instructions.size(), 0);
}