#include <gtest/gtest.h>
#include "../../include/backends/vm/VirtualMachine.hpp"
#include "../../include/backends/vm/Instruction.hpp"
#include "../../include/backends/vm/OPCode.hpp"

class VMTest : public ::testing::Test {
protected:
    VirtualMachine vm;

    void SetUp() override {}
    void TearDown() override {}

    // Helper to create TypedValue
    TypedValue makeInt(int value) {
        return {TypedValue::INT, {.i = value}};
    }

    TypedValue makeFloat(float value) {
        return {TypedValue::FLOAT, {.f = value}};
    }

    TypedValue makeDouble(double value) {
        return {TypedValue::DOUBLE, {.d = value}};
    }

    TypedValue makeBool(bool value) {
        return {TypedValue::BOOL, {.b = value}};
    }

    TypedValue makeString(const std::string& str) {
        size_t id = vm.internString(str);
        return {TypedValue::STRING, {.string_id = id}};
    }
};

// ===== BASIC LOAD OPERATIONS =====

TEST_F(VMTest, LoadInt) {
    std::vector<TypedValue> constants = {makeInt(42)};
    std::vector<Instruction> instructions = {
        {OPCode::LOAD_INT, 0},
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    // Check result: 42 should be on stack
    int result = vm.popInt();
    EXPECT_EQ(result, 42);
}

TEST_F(VMTest, LoadFloat) {
    std::vector<TypedValue> constants = {makeFloat(3.14f)};
    std::vector<Instruction> instructions = {
        {OPCode::LOAD_FLOAT, 0},
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    float result = vm.popFloat();
    EXPECT_FLOAT_EQ(result, 3.14f);
}

TEST_F(VMTest, LoadDouble) {
    std::vector<TypedValue> constants = {makeDouble(2.718281828)};
    std::vector<Instruction> instructions = {
        {OPCode::LOAD_DOUBLE, 0},
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    double result = vm.popDouble();
    EXPECT_DOUBLE_EQ(result, 2.718281828);
}

TEST_F(VMTest, LoadBool) {
    std::vector<TypedValue> constants = {makeBool(true), makeBool(false)};
    std::vector<Instruction> instructions = {
        {OPCode::LOAD_BOOL, 0},  // Load true
        {OPCode::LOAD_BOOL, 1},  // Load false
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    bool result2 = vm.popBool();
    bool result1 = vm.popBool();
    EXPECT_EQ(result1, true);
    EXPECT_EQ(result2, false);
}

TEST_F(VMTest, LoadString) {
    std::vector<TypedValue> constants = {makeString("hello world")};
    std::vector<Instruction> instructions = {
        {OPCode::LOAD_STRING, 0},
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    size_t string_id = vm.popStringId();
    std::string result = vm.getString(string_id);
    EXPECT_EQ(result, "hello world");
}

// ===== INTEGER ARITHMETIC =====

TEST_F(VMTest, AddInt) {
    std::vector<TypedValue> constants = {makeInt(5), makeInt(3)};
    std::vector<Instruction> instructions = {
        {OPCode::LOAD_INT, 0},   // Load 5
        {OPCode::LOAD_INT, 1},   // Load 3
        {OPCode::ADD_INT, 0},    // 5 + 3
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    int result = vm.popInt();
    EXPECT_EQ(result, 8);
}

TEST_F(VMTest, SubInt) {
    std::vector<TypedValue> constants = {makeInt(10), makeInt(3)};
    std::vector<Instruction> instructions = {
        {OPCode::LOAD_INT, 0},   // Load 10
        {OPCode::LOAD_INT, 1},   // Load 3
        {OPCode::SUB_INT, 0},    // 10 - 3
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    int result = vm.popInt();
    EXPECT_EQ(result, 7);
}

TEST_F(VMTest, MultInt) {
    std::vector<TypedValue> constants = {makeInt(4), makeInt(7)};
    std::vector<Instruction> instructions = {
        {OPCode::LOAD_INT, 0},   // Load 4
        {OPCode::LOAD_INT, 1},   // Load 7
        {OPCode::MULT_INT, 0},   // 4 * 7
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    int result = vm.popInt();
    EXPECT_EQ(result, 28);
}

TEST_F(VMTest, DivInt) {
    std::vector<TypedValue> constants = {makeInt(15), makeInt(3)};
    std::vector<Instruction> instructions = {
        {OPCode::LOAD_INT, 0},   // Load 15
        {OPCode::LOAD_INT, 1},   // Load 3
        {OPCode::DIV_INT, 0},    // 15 / 3
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    int result = vm.popInt();
    EXPECT_EQ(result, 5);
}

// ===== FLOAT ARITHMETIC =====

TEST_F(VMTest, AddFloat) {
    std::vector<TypedValue> constants = {makeFloat(2.5f), makeFloat(1.5f)};
    std::vector<Instruction> instructions = {
        {OPCode::LOAD_FLOAT, 0},
        {OPCode::LOAD_FLOAT, 1},
        {OPCode::ADD_FLOAT, 0},
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    float result = vm.popFloat();
    EXPECT_FLOAT_EQ(result, 4.0f);
}

TEST_F(VMTest, MultFloat) {
    std::vector<TypedValue> constants = {makeFloat(2.0f), makeFloat(3.5f)};
    std::vector<Instruction> instructions = {
        {OPCode::LOAD_FLOAT, 0},
        {OPCode::LOAD_FLOAT, 1},
        {OPCode::MULT_FLOAT, 0},
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    float result = vm.popFloat();
    EXPECT_FLOAT_EQ(result, 7.0f);
}

// ===== DOUBLE ARITHMETIC =====

TEST_F(VMTest, AddDouble) {
    std::vector<TypedValue> constants = {makeDouble(1.1), makeDouble(2.2)};
    std::vector<Instruction> instructions = {
        {OPCode::LOAD_DOUBLE, 0},
        {OPCode::LOAD_DOUBLE, 1},
        {OPCode::ADD_DOUBLE, 0},
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    double result = vm.popDouble();
    EXPECT_DOUBLE_EQ(result, 3.3);
}

TEST_F(VMTest, DivDouble) {
    std::vector<TypedValue> constants = {makeDouble(9.0), makeDouble(3.0)};
    std::vector<Instruction> instructions = {
        {OPCode::LOAD_DOUBLE, 0},
        {OPCode::LOAD_DOUBLE, 1},
        {OPCode::DIV_DOUBLE, 0},
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    double result = vm.popDouble();
    EXPECT_DOUBLE_EQ(result, 3.0);
}

// ===== TYPE CONVERSIONS =====

TEST_F(VMTest, IntToDouble) {
    std::vector<TypedValue> constants = {makeInt(42)};
    std::vector<Instruction> instructions = {
        {OPCode::LOAD_INT, 0},
        {OPCode::INT_TO_DOUBLE, 0},
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    double result = vm.popDouble();
    EXPECT_DOUBLE_EQ(result, 42.0);
}

TEST_F(VMTest, FloatToDouble) {
    std::vector<TypedValue> constants = {makeFloat(3.14f)};
    std::vector<Instruction> instructions = {
        {OPCode::LOAD_FLOAT, 0},
        {OPCode::FLOAT_TO_DOUBLE, 0},
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    double result = vm.popDouble();
    EXPECT_DOUBLE_EQ(result, static_cast<double>(3.14f));
}

// ===== COMPARISONS =====

TEST_F(VMTest, EqualInt) {
    std::vector<TypedValue> constants = {makeInt(5), makeInt(5), makeInt(3)};
    std::vector<Instruction> instructions = {
        {OPCode::LOAD_INT, 0},   // 5
        {OPCode::LOAD_INT, 1},   // 5
        {OPCode::EQ_INT, 0},     // 5 == 5 -> true
        {OPCode::LOAD_INT, 0},   // 5
        {OPCode::LOAD_INT, 2},   // 3
        {OPCode::EQ_INT, 0},     // 5 == 3 -> false
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    bool result2 = vm.popBool();  // false
    bool result1 = vm.popBool();  // true
    EXPECT_EQ(result1, true);
    EXPECT_EQ(result2, false);
}

TEST_F(VMTest, LessThanInt) {
    std::vector<TypedValue> constants = {makeInt(3), makeInt(7)};
    std::vector<Instruction> instructions = {
        {OPCode::LOAD_INT, 0},   // 3
        {OPCode::LOAD_INT, 1},   // 7
        {OPCode::LT_INT, 0},     // 3 < 7 -> true
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    bool result = vm.popBool();
    EXPECT_EQ(result, true);
}

TEST_F(VMTest, EqualDouble) {
    std::vector<TypedValue> constants = {makeDouble(2.5), makeDouble(2.5)};
    std::vector<Instruction> instructions = {
        {OPCode::LOAD_DOUBLE, 0},
        {OPCode::LOAD_DOUBLE, 1},
        {OPCode::EQ_DOUBLE, 0},
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    bool result = vm.popBool();
    EXPECT_EQ(result, true);
}

// ===== COMPLEX EXPRESSIONS =====

TEST_F(VMTest, ComplexArithmetic) {
    // Test: (2 + 3) * 4 = 20
    std::vector<TypedValue> constants = {makeInt(2), makeInt(3), makeInt(4)};
    std::vector<Instruction> instructions = {
        {OPCode::LOAD_INT, 0},   // 2
        {OPCode::LOAD_INT, 1},   // 3
        {OPCode::ADD_INT, 0},    // 2 + 3 = 5
        {OPCode::LOAD_INT, 2},   // 4
        {OPCode::MULT_INT, 0},   // 5 * 4 = 20
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    int result = vm.popInt();
    EXPECT_EQ(result, 20);
}

TEST_F(VMTest, MixedTypeOperations) {
    // Test: int(5) -> double(5.0) + double(2.5) = 7.5
    std::vector<TypedValue> constants = {makeInt(5), makeDouble(2.5)};
    std::vector<Instruction> instructions = {
        {OPCode::LOAD_INT, 0},      // 5
        {OPCode::INT_TO_DOUBLE, 0}, // 5.0
        {OPCode::LOAD_DOUBLE, 1},   // 2.5
        {OPCode::ADD_DOUBLE, 0},    // 5.0 + 2.5 = 7.5
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    double result = vm.popDouble();
    EXPECT_DOUBLE_EQ(result, 7.5);
}

// ===== STRING OPERATIONS =====

TEST_F(VMTest, StringInterning) {
    // Test that same strings get same ID
    size_t id1 = vm.internString("test");
    size_t id2 = vm.internString("test");
    size_t id3 = vm.internString("different");

    EXPECT_EQ(id1, id2);  // Same string, same ID
    EXPECT_NE(id1, id3);  // Different string, different ID
}

TEST_F(VMTest, StringRetrieval) {
    size_t id = vm.internString("hello world");
    std::string retrieved = vm.getString(id);
    EXPECT_EQ(retrieved, "hello world");
}

// ===== ERROR HANDLING =====

TEST_F(VMTest, StackUnderflowInt) {
    std::vector<TypedValue> constants = {};
    std::vector<Instruction> instructions = {
        {OPCode::ADD_INT, 0},  // Try to add with empty stack
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    EXPECT_THROW(vm.run(), std::exception);
}

TEST_F(VMTest, TypeMismatchError) {
    std::vector<TypedValue> constants = {makeInt(5), makeFloat(3.0f)};
    std::vector<Instruction> instructions = {
        {OPCode::LOAD_INT, 0},    // Push int
        {OPCode::LOAD_FLOAT, 1},  // Push float
        {OPCode::ADD_INT, 0},     // Try to add as ints -> should fail
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    EXPECT_THROW(vm.run(), std::exception);
}

TEST_F(VMTest, InvalidStringId) {
    EXPECT_THROW(vm.getString(999), std::exception);
}

// ===== EDGE CASES =====

TEST_F(VMTest, DivisionByZeroInt) {
    std::vector<TypedValue> constants = {makeInt(5), makeInt(0)};
    std::vector<Instruction> instructions = {
        {OPCode::LOAD_INT, 0},
        {OPCode::LOAD_INT, 1},
        {OPCode::DIV_INT, 0},
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    // Note: This might crash or give undefined behavior - depends on your error handling
    // You might want to add division by zero checking in your VM
}

TEST_F(VMTest, EmptyProgram) {
    std::vector<TypedValue> constants = {};
    std::vector<Instruction> instructions = {
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();  // Should complete without error
}

TEST_F(VMTest, LargeNumbers) {
    std::vector<TypedValue> constants = {makeInt(1000000), makeInt(2000000)};
    std::vector<Instruction> instructions = {
        {OPCode::LOAD_INT, 0},
        {OPCode::LOAD_INT, 1},
        {OPCode::ADD_INT, 0},
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    int result = vm.popInt();
    EXPECT_EQ(result, 3000000);
}

// ===== STRESS TESTS =====

TEST_F(VMTest, DeepArithmetic) {
    // Test: 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 = 55
    std::vector<TypedValue> constants;
    std::vector<Instruction> instructions;

    // Load first number
    constants.push_back(makeInt(1));
    instructions.push_back({OPCode::LOAD_INT, 0});

    // Add numbers 2 through 10
    for (int i = 2; i <= 10; i++) {
        constants.push_back(makeInt(i));
        instructions.push_back({OPCode::LOAD_INT, static_cast<int>(constants.size() - 1)});
        instructions.push_back({OPCode::ADD_INT, 0});
    }

    instructions.push_back({OPCode::HALT, 0});

    vm.loadProgram(instructions, constants);
    vm.run();

    int result = vm.popInt();
    EXPECT_EQ(result, 55);
}

TEST_F(VMTest, ManyStringInterns) {
    // Test interning many strings
    std::vector<std::string> test_strings;
    for (int i = 0; i < 100; i++) {
        test_strings.push_back("string_" + std::to_string(i));
    }

    // Intern all strings
    std::vector<size_t> ids;
    for (const auto& str : test_strings) {
        ids.push_back(vm.internString(str));
    }

    // Verify all strings can be retrieved correctly
    for (size_t i = 0; i < test_strings.size(); i++) {
        std::string retrieved = vm.getString(ids[i]);
        EXPECT_EQ(retrieved, test_strings[i]);
    }
}