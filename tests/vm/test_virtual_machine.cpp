#include <gtest/gtest.h>
#include "../../include/backends/vm/VirtualMachine.hpp"
#include "../../include/backends/vm/BytecodeCompiler.hpp"
#include "../../include/parser/Parser.hpp"
#include "../../include/lexer/Tokenizer.hpp"

class VirtualMachineTest : public ::testing::Test {
protected:
    VirtualMachine vm;
    BytecodeCompiler compiler;

    VirtualMachineTest() : compiler(vm) {}

    void SetUp() override {
        vm.reset();
    }

    // Helper to compile and run source code
    void runSource(const std::string& source) {
        Tokenizer tokenizer(source);
        auto tokens = tokenizer.tokenize();
        Parser parser(tokens);
        auto ast = parser.parseProgram();
        auto program = compiler.compile(std::move(ast));
        vm.loadProgram(program.instructions, program.constants);
        vm.run();
    }

    // Helper to get top value from stack after execution
    Value getStackTop() {
        return vm.popValue();
    }

    // Helper to get global variable value by slot
    Value getGlobalVariable(int slot) {
        const auto& globals = vm.getGlobals();
        if (slot >= 0 && slot < static_cast<int>(globals.size())) {
            return globals[slot];
        }
        throw RuntimeException("Invalid global variable slot: " + std::to_string(slot));
    }

    // Helper to get the last (most recent) global variable
    Value getLastGlobalVariable() {
        const auto& globals = vm.getGlobals();
        if (globals.empty()) {
            throw RuntimeException("No global variables available");
        }
        return globals.back();
    }
};

// ========== BASIC INSTRUCTION EXECUTION TESTS ==========

TEST_F(VirtualMachineTest, ExecuteIntegerLiteral) {
    runSource("42");

    Value result = getStackTop();
    EXPECT_EQ(result.type, ValueType::INT);
    EXPECT_EQ(asInt(result), 42);
}

TEST_F(VirtualMachineTest, ExecuteFloatLiteral) {
    runSource("3.14f");

    Value result = getStackTop();
    EXPECT_EQ(result.type, ValueType::FLOAT);
    EXPECT_FLOAT_EQ(asFloat(result), 3.14f);
}

TEST_F(VirtualMachineTest, ExecuteDoubleLiteral) {
    runSource("2.718");

    Value result = getStackTop();
    EXPECT_EQ(result.type, ValueType::DOUBLE);
    EXPECT_DOUBLE_EQ(asDouble(result), 2.718);
}

TEST_F(VirtualMachineTest, ExecuteBooleanLiterals) {
    runSource("true");
    Value result = getStackTop();
    EXPECT_EQ(result.type, ValueType::BOOL);
    EXPECT_TRUE(asBool(result));

    vm.reset();
    runSource("false");
    result = getStackTop();
    EXPECT_EQ(result.type, ValueType::BOOL);
    EXPECT_FALSE(asBool(result));
}

TEST_F(VirtualMachineTest, ExecuteStringLiteral) {
    runSource("\"hello\"");

    Value result = getStackTop();
    EXPECT_EQ(result.type, ValueType::OBJECT);

    StringObject* str = asString(result);
    EXPECT_STREQ(str->chars, "hello");
    EXPECT_EQ(str->length, 5);
}

// ========== ARITHMETIC OPERATIONS TESTS ==========

TEST_F(VirtualMachineTest, ExecuteIntegerArithmetic) {
    runSource("5 + 3");
    EXPECT_EQ(asInt(getStackTop()), 8);

    vm.reset();
    runSource("10 - 4");
    EXPECT_EQ(asInt(getStackTop()), 6);

    vm.reset();
    runSource("6 * 7");
    EXPECT_EQ(asInt(getStackTop()), 42);

    vm.reset();
    runSource("20 / 5");
    EXPECT_EQ(asInt(getStackTop()), 4);
}

TEST_F(VirtualMachineTest, ExecuteFloatArithmetic) {
    runSource("3.5f + 2.1f");
    EXPECT_FLOAT_EQ(asFloat(getStackTop()), 5.6f);

    vm.reset();
    runSource("10.0f - 3.5f");
    EXPECT_FLOAT_EQ(asFloat(getStackTop()), 6.5f);

    vm.reset();
    runSource("2.5f * 4.0f");
    EXPECT_FLOAT_EQ(asFloat(getStackTop()), 10.0f);

    vm.reset();
    runSource("15.0f / 3.0f");
    EXPECT_FLOAT_EQ(asFloat(getStackTop()), 5.0f);
}

TEST_F(VirtualMachineTest, ExecuteDoubleArithmetic) {
    runSource("1.5 + 2.5");
    EXPECT_DOUBLE_EQ(asDouble(getStackTop()), 4.0);

    vm.reset();
    runSource("10.7 - 3.2");
    EXPECT_DOUBLE_EQ(asDouble(getStackTop()), 7.5);

    vm.reset();
    runSource("2.5 * 3.0");
    EXPECT_DOUBLE_EQ(asDouble(getStackTop()), 7.5);

    vm.reset();
    runSource("9.0 / 3.0");
    EXPECT_DOUBLE_EQ(asDouble(getStackTop()), 3.0);
}

TEST_F(VirtualMachineTest, ExecuteUnaryOperations) {
    runSource("-42");
    EXPECT_EQ(asInt(getStackTop()), -42);

    vm.reset();
    runSource("-3.14f");
    EXPECT_FLOAT_EQ(asFloat(getStackTop()), -3.14f);

    vm.reset();
    runSource("!true");
    EXPECT_FALSE(asBool(getStackTop()));

    vm.reset();
    runSource("!false");
    EXPECT_TRUE(asBool(getStackTop()));
}

// ========== COMPARISON OPERATIONS TESTS ==========

TEST_F(VirtualMachineTest, ExecuteIntegerComparisons) {
    runSource("5 == 5");
    EXPECT_TRUE(asBool(getStackTop()));

    vm.reset();
    runSource("5 == 3");
    EXPECT_FALSE(asBool(getStackTop()));

    vm.reset();
    runSource("10 > 5");
    EXPECT_TRUE(asBool(getStackTop()));

    vm.reset();
    runSource("3 > 8");
    EXPECT_FALSE(asBool(getStackTop()));

    vm.reset();
    runSource("2 < 7");
    EXPECT_TRUE(asBool(getStackTop()));

    vm.reset();
    runSource("9 < 4");
    EXPECT_FALSE(asBool(getStackTop()));

    vm.reset();
    runSource("5 >= 5");
    EXPECT_TRUE(asBool(getStackTop()));

    vm.reset();
    runSource("3 <= 7");
    EXPECT_TRUE(asBool(getStackTop()));
}

TEST_F(VirtualMachineTest, ExecuteFloatComparisons) {
    runSource("3.14f == 3.14f");
    EXPECT_TRUE(asBool(getStackTop()));

    vm.reset();
    runSource("5.0f > 3.0f");
    EXPECT_TRUE(asBool(getStackTop()));

    vm.reset();
    runSource("2.5f < 7.1f");
    EXPECT_TRUE(asBool(getStackTop()));
}

TEST_F(VirtualMachineTest, ExecuteBooleanComparisons) {
    runSource("true == true");
    EXPECT_TRUE(asBool(getStackTop()));

    vm.reset();
    runSource("true == false");
    EXPECT_FALSE(asBool(getStackTop()));

    vm.reset();
    runSource("false == false");
    EXPECT_TRUE(asBool(getStackTop()));
}

// ========== VARIABLE OPERATIONS TESTS ==========

TEST_F(VirtualMachineTest, ExecuteGlobalVariableDeclaration) {
    runSource(R"(
x: int = 42
y: int = x
)");

    // The last expression (y = x) should leave x's value on stack
    EXPECT_EQ(asInt(getLastGlobalVariable()), 42);
}

TEST_F(VirtualMachineTest, ExecuteGlobalVariableAssignment) {
    runSource(R"(
counter: int = 0
counter = 5
counter = counter + 10
result: int = counter
)");

    EXPECT_EQ(asInt(getGlobalVariable(0)), 15);
}

TEST_F(VirtualMachineTest, ExecuteMultipleGlobalVariables) {
    runSource(R"(
a: int = 10
b: int = 20
c: int = a + b
result: int = c
)");

    EXPECT_EQ(asInt(getGlobalVariable(3)), 30);
}

TEST_F(VirtualMachineTest, ExecuteStringVariables) {
    runSource(R"(
name: str = "Hello"
greeting: str = name
)");

    Value result = getLastGlobalVariable();  // greeting variable
    EXPECT_EQ(result.type, ValueType::OBJECT);
    StringObject* str = asString(result);
    EXPECT_STREQ(str->chars, "Hello");
}

// ========== FUNCTION EXECUTION TESTS ==========

TEST_F(VirtualMachineTest, ExecuteSimpleFunction) {
    runSource(R"(
def add(x: int, y: int) -> int {
    return x + y
}

result: int = add(5, 3)
)");

    // result is stored in global variable slot 0
    EXPECT_EQ(asInt(getGlobalVariable(0)), 8);
}

TEST_F(VirtualMachineTest, ExecuteFunctionWithLocalVariables) {
    runSource(R"(
def calculate(x: int) -> int {
    y: int = x * 2
    z: int = y + 10
    return z
}

result: int = calculate(5)
)");

    EXPECT_EQ(asInt(getLastGlobalVariable()), 20);  // (5 * 2) + 10 = 20
}

TEST_F(VirtualMachineTest, ExecuteRecursiveFunction) {
    runSource(R"(
def factorial(n: int) -> int {
    if (n <= 1) {
        return 1
    }
    return n * factorial(n - 1)
}

result: int = factorial(5)
)");

    EXPECT_EQ(asInt(getLastGlobalVariable()), 120);  // 5! = 120
}

TEST_F(VirtualMachineTest, ExecuteMultipleFunctionCalls) {
    runSource(R"(
def multiply2(x: int) -> int {
    return x * 2
}

def multiply3(x: int) -> int {
    return x * 3
}

result: int = multiply2(5) + multiply3(4)
)");

    EXPECT_EQ(asInt(getLastGlobalVariable()), 22);  // (5*2) + (4*3) = 10 + 12 = 22
}

TEST_F(VirtualMachineTest, ExecuteFunctionAccessingGlobals) {
    runSource(R"(
globalVar: int = 100

def addToGlobal(x: int) -> int {
    return globalVar + x
}

result: int = addToGlobal(42)
)");

    EXPECT_EQ(asInt(getLastGlobalVariable()), 142);
}

TEST_F(VirtualMachineTest, ExecuteFunctionModifyingGlobals) {
    runSource(R"(
counter: int = 0

def increment() -> int {
    counter = counter + 1
    return counter
}

first: int = increment()
second: int = increment()
result: int = second
)");

    EXPECT_EQ(asInt(getLastGlobalVariable()), 2);
}

// ========== CONTROL FLOW TESTS ==========

TEST_F(VirtualMachineTest, ExecuteIfStatement) {
    runSource(R"(
x: int = 5
if (x > 0) {
    x = 10
}
result: int = x
)");

    EXPECT_EQ(asInt(getLastGlobalVariable()), 10);
}

TEST_F(VirtualMachineTest, ExecuteIfElseStatement) {
    runSource(R"(
x: int = -5
if (x > 0) {
    x = 10
} else {
    x = 20
}
result: int = x
)");

    EXPECT_EQ(asInt(getLastGlobalVariable()), 20);
}

TEST_F(VirtualMachineTest, ExecuteNestedIfStatements) {
    runSource(R"(
x: int = 15
if (x > 10) {
    if (x > 20) {
        x = 1
    } else {
        x = 2
    }
} else {
    x = 3
}
result: int = x
)");

    EXPECT_EQ(asInt(getLastGlobalVariable()), 2);
}

TEST_F(VirtualMachineTest, ExecuteWhileLoop) {
    runSource(R"(
i: int = 0
sum: int = 0
while (i < 5) {
    sum = sum + i
    i = i + 1
}
result: int = sum
)");

    EXPECT_EQ(asInt(getLastGlobalVariable()), 10);  // 0+1+2+3+4 = 10
}

TEST_F(VirtualMachineTest, ExecuteWhileLoopWithCondition) {
    runSource(R"(
x: int = 1
while (x < 100) {
    x = x * 2
}
result: int = x
)");

    EXPECT_EQ(asInt(getLastGlobalVariable()), 128);  // 1*2*2*2*2*2*2*2 = 128
}

// ========== SCOPING TESTS ==========

TEST_F(VirtualMachineTest, ExecuteLocalVariableScoping) {
    runSource(R"(
global: int = 100

def test() -> int {
    local: int = 10
    return global + local
}

result: int = test()
)");

    EXPECT_EQ(asInt(getLastGlobalVariable()), 110);
}

TEST_F(VirtualMachineTest, ExecuteBlockScoping) {
    runSource(R"(
x: int = 1
if (true) {
    y: int = 2
    x = x + y
}
result: int = x
)");

    EXPECT_EQ(asInt(getLastGlobalVariable()), 3);
}

TEST_F(VirtualMachineTest, ExecuteParameterScoping) {
    runSource(R"(
def test(param: int) -> int {
    param = param + 10
    return param
}

x: int = 5
result: int = test(x)
)");

    EXPECT_EQ(asInt(getLastGlobalVariable()), 15);
}

// ========== COMPLEX PROGRAM TESTS ==========

TEST_F(VirtualMachineTest, ExecuteComplexProgram) {
    runSource(R"(
counter: int = 0

def fibonacci(n: int) -> int {
    if (n <= 1) {
        return n
    }
    return fibonacci(n - 1) + fibonacci(n - 2)
}

def increment() -> int {
    counter = counter + 1
    return counter
}

result1: int = fibonacci(6)
result2: int = increment()
result3: int = increment()
final: int = result1 + result2 + result3
)");

    EXPECT_EQ(asInt(getLastGlobalVariable()), 11);  // fib(6)=8, counter becomes 1 then 2, 8+1+2=11
}

TEST_F(VirtualMachineTest, ExecuteNestedFunctionCalls) {
    runSource(R"(
def add(a: int, b: int) -> int {
    return a + b
}

def multiply(a: int, b: int) -> int {
    return a * b
}

def calculate(x: int) -> int {
    return add(multiply(x, 2), 10)
}

result: int = calculate(5)
)");

    EXPECT_EQ(asInt(getLastGlobalVariable()), 20);  // multiply(5,2)=10, add(10,10)=20
}

// ========== ERROR HANDLING TESTS ==========

TEST_F(VirtualMachineTest, HandleStackUnderflow) {
    // Empty program should not crash
    runSource("");
    // No assertions needed, just that it doesn't crash
}

TEST_F(VirtualMachineTest, HandleDivisionByZero) {
    // Division by zero should throw a RuntimeException
    EXPECT_THROW({
        runSource("10 / 0");
    }, RuntimeException);
}

// ========== GARBAGE COLLECTION TESTS ==========

TEST_F(VirtualMachineTest, TriggerGarbageCollection) {
    // Create many string objects to trigger GC
    runSource(R"(
str1: str = "hello"
str2: str = "world"
str3: str = "test"
str4: str = "garbage"
str5: str = "collection"
result: str = str1
)");

    Value result = getLastGlobalVariable();
    EXPECT_EQ(result.type, ValueType::OBJECT);
    StringObject* str = asString(result);
    EXPECT_STREQ(str->chars, "hello");
}

TEST_F(VirtualMachineTest, StringObjectSurvivesGC) {
    // Test that reachable strings survive garbage collection
    runSource(R"(
def createString() -> str {
    temp: str = "temporary"
    return "permanent"
}

result: str = createString()
)");

    Value result = getLastGlobalVariable();
    EXPECT_EQ(result.type, ValueType::OBJECT);
    StringObject* str = asString(result);
    EXPECT_STREQ(str->chars, "permanent");
}

// ========== MIXED TYPE TESTS ==========

TEST_F(VirtualMachineTest, ExecuteMixedTypeProgram) {
    runSource(R"(
intVar: int = 42
floatVar: float = 3.14f
doubleVar: double = 2.718
boolVar: bool = true
stringVar: str = "hello"

def test() -> int {
    if (boolVar) {
        return intVar
    } else {
        return 0
    }
}

result: int = test()
)");

    EXPECT_EQ(asInt(getLastGlobalVariable()), 42);
}