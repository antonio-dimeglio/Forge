#include <gtest/gtest.h>
#include "../../include/backends/vm/VirtualMachine.hpp"
#include "../../include/backends/vm/Instruction.hpp"
#include "../../include/backends/vm/OPCode.hpp"
#include <limits>
#include <cmath>

class VMExtendedTest : public ::testing::Test {
protected:
    VirtualMachine vm;

    void SetUp() override {}
    void TearDown() override {}

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

// ===== EXTREME VALUES TESTS =====

TEST_F(VMExtendedTest, MaxIntValues) {
    std::vector<TypedValue> constants = {
        makeInt(std::numeric_limits<int>::max()),
        makeInt(std::numeric_limits<int>::min()),
        makeInt(1)
    };

    std::vector<Instruction> instructions = {
        {OPCode::LOAD_INT, 0},   // MAX_INT
        {OPCode::LOAD_INT, 1},   // MIN_INT
        {OPCode::LOAD_INT, 2},   // 1
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    int val3 = vm.popInt();
    int val2 = vm.popInt();
    int val1 = vm.popInt();

    EXPECT_EQ(val1, std::numeric_limits<int>::max());
    EXPECT_EQ(val2, std::numeric_limits<int>::min());
    EXPECT_EQ(val3, 1);
}

TEST_F(VMExtendedTest, FloatPrecision) {
    std::vector<TypedValue> constants = {
        makeFloat(0.1f),
        makeFloat(0.2f),
        makeFloat(0.3f)
    };

    std::vector<Instruction> instructions = {
        {OPCode::LOAD_FLOAT, 0},   // 0.1
        {OPCode::LOAD_FLOAT, 1},   // 0.2
        {OPCode::ADD_FLOAT, 0},    // 0.1 + 0.2
        {OPCode::LOAD_FLOAT, 2},   // 0.3
        {OPCode::SUB_FLOAT, 0},    // (0.1 + 0.2) - 0.3
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    float result = vm.popFloat();
    // Due to floating point precision, this might not be exactly 0
    EXPECT_NEAR(result, 0.0f, 1e-6f);
}

TEST_F(VMExtendedTest, DoublePrecision) {
    std::vector<TypedValue> constants = {
        makeDouble(1e-15),
        makeDouble(1e15),
        makeDouble(1e-15)
    };

    std::vector<Instruction> instructions = {
        {OPCode::LOAD_DOUBLE, 0},   // 1e-15
        {OPCode::LOAD_DOUBLE, 1},   // 1e15
        {OPCode::ADD_DOUBLE, 0},    // 1e-15 + 1e15
        {OPCode::LOAD_DOUBLE, 2},   // 1e-15
        {OPCode::SUB_DOUBLE, 0},    // (1e-15 + 1e15) - 1e-15
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    double result = vm.popDouble();
    EXPECT_DOUBLE_EQ(result, 1e15);
}

// ===== NEGATIVE NUMBER TESTS =====

TEST_F(VMExtendedTest, NegativeArithmetic) {
    std::vector<TypedValue> constants = {
        makeInt(-5),
        makeInt(-3),
        makeInt(2)
    };

    std::vector<Instruction> instructions = {
        {OPCode::LOAD_INT, 0},   // -5
        {OPCode::LOAD_INT, 1},   // -3
        {OPCode::ADD_INT, 0},    // -5 + (-3) = -8
        {OPCode::LOAD_INT, 2},   // 2
        {OPCode::MULT_INT, 0},   // -8 * 2 = -16
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    int result = vm.popInt();
    EXPECT_EQ(result, -16);
}

TEST_F(VMExtendedTest, NegativeComparisons) {
    std::vector<TypedValue> constants = {
        makeInt(-10),
        makeInt(-5),
        makeInt(0)
    };

    std::vector<Instruction> instructions = {
        // Test -10 < -5 (should be true)
        {OPCode::LOAD_INT, 0},   // -10
        {OPCode::LOAD_INT, 1},   // -5
        {OPCode::LT_INT, 0},     // -10 < -5 -> true

        // Test -5 < 0 (should be true)
        {OPCode::LOAD_INT, 1},   // -5
        {OPCode::LOAD_INT, 2},   // 0
        {OPCode::LT_INT, 0},     // -5 < 0 -> true

        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    bool result2 = vm.popBool();
    bool result1 = vm.popBool();

    EXPECT_EQ(result1, true);   // -10 < -5
    EXPECT_EQ(result2, true);   // -5 < 0
}

// ===== ZERO HANDLING TESTS =====

TEST_F(VMExtendedTest, ZeroArithmetic) {
    std::vector<TypedValue> constants = {
        makeInt(0),
        makeInt(5),
        makeFloat(0.0f),
        makeDouble(0.0)
    };

    std::vector<Instruction> instructions = {
        // 0 + 5 = 5
        {OPCode::LOAD_INT, 0},
        {OPCode::LOAD_INT, 1},
        {OPCode::ADD_INT, 0},

        // 5 * 0 = 0
        {OPCode::LOAD_INT, 0},
        {OPCode::MULT_INT, 0},

        // 0.0f + 0.0f = 0.0f
        {OPCode::LOAD_FLOAT, 2},
        {OPCode::LOAD_FLOAT, 2},
        {OPCode::ADD_FLOAT, 0},

        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    float floatResult = vm.popFloat();
    int intResult = vm.popInt();

    EXPECT_EQ(intResult, 0);
    EXPECT_FLOAT_EQ(floatResult, 0.0f);
}

// ===== COMPLEX EXPRESSION TESTS =====

TEST_F(VMExtendedTest, NestedArithmetic) {
    // Test: ((2 + 3) * (4 - 1)) / (6 + 3) = (5 * 3) / 9 = 15 / 9 = 1
    std::vector<TypedValue> constants = {
        makeInt(2), makeInt(3), makeInt(4), makeInt(1), makeInt(6), makeInt(3)
    };

    std::vector<Instruction> instructions = {
        // (2 + 3)
        {OPCode::LOAD_INT, 0},   // 2
        {OPCode::LOAD_INT, 1},   // 3
        {OPCode::ADD_INT, 0},    // 5

        // (4 - 1)
        {OPCode::LOAD_INT, 2},   // 4
        {OPCode::LOAD_INT, 3},   // 1
        {OPCode::SUB_INT, 0},    // 3

        // 5 * 3
        {OPCode::MULT_INT, 0},   // 15

        // (6 + 3)
        {OPCode::LOAD_INT, 4},   // 6
        {OPCode::LOAD_INT, 5},   // 3
        {OPCode::ADD_INT, 0},    // 9

        // 15 / 9
        {OPCode::DIV_INT, 0},    // 1

        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    int result = vm.popInt();
    EXPECT_EQ(result, 1);
}

TEST_F(VMExtendedTest, MixedTypeComplexExpression) {
    // Test: int(10) -> double + double(2.5) * float(2.0) -> double = 10.0 + 5.0 = 15.0
    std::vector<TypedValue> constants = {
        makeInt(10),
        makeDouble(2.5),
        makeFloat(2.0f)
    };

    std::vector<Instruction> instructions = {
        // Convert int to double
        {OPCode::LOAD_INT, 0},      // 10
        {OPCode::INT_TO_DOUBLE, 0}, // 10.0

        // Calculate 2.5 * 2.0 (as doubles)
        {OPCode::LOAD_DOUBLE, 1},   // 2.5
        {OPCode::LOAD_FLOAT, 2},    // 2.0f
        {OPCode::FLOAT_TO_DOUBLE, 0}, // 2.0 (as double)
        {OPCode::MULT_DOUBLE, 0},   // 2.5 * 2.0 = 5.0

        // 10.0 + 5.0
        {OPCode::ADD_DOUBLE, 0},    // 15.0

        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    double result = vm.popDouble();
    EXPECT_DOUBLE_EQ(result, 15.0);
}

// ===== STRING STRESS TESTS =====

TEST_F(VMExtendedTest, EmptyStringHandling) {
    std::vector<TypedValue> constants = {
        makeString(""),
        makeString("test"),
        makeString("")
    };

    std::vector<Instruction> instructions = {
        {OPCode::LOAD_STRING, 0},  // ""
        {OPCode::LOAD_STRING, 1},  // "test"
        {OPCode::LOAD_STRING, 2},  // ""
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    size_t id3 = vm.popStringId();
    size_t id2 = vm.popStringId();
    size_t id1 = vm.popStringId();

    EXPECT_EQ(vm.getString(id1), "");
    EXPECT_EQ(vm.getString(id2), "test");
    EXPECT_EQ(vm.getString(id3), "");
    EXPECT_EQ(id1, id3);  // Same empty string should have same ID
}

TEST_F(VMExtendedTest, LongStringHandling) {
    std::string longString(10000, 'a');  // 10,000 'a' characters
    std::vector<TypedValue> constants = {makeString(longString)};

    std::vector<Instruction> instructions = {
        {OPCode::LOAD_STRING, 0},
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    size_t id = vm.popStringId();
    std::string result = vm.getString(id);
    EXPECT_EQ(result.length(), 10000);
    EXPECT_EQ(result, longString);
}

TEST_F(VMExtendedTest, UnicodeStringHandling) {
    std::vector<TypedValue> constants = {
        makeString("Hello 世界"),
        makeString("🚀 Rocket"),
        makeString("αβγδε")
    };

    std::vector<Instruction> instructions = {
        {OPCode::LOAD_STRING, 0},
        {OPCode::LOAD_STRING, 1},
        {OPCode::LOAD_STRING, 2},
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    size_t id3 = vm.popStringId();
    size_t id2 = vm.popStringId();
    size_t id1 = vm.popStringId();

    EXPECT_EQ(vm.getString(id1), "Hello 世界");
    EXPECT_EQ(vm.getString(id2), "🚀 Rocket");
    EXPECT_EQ(vm.getString(id3), "αβγδε");
}

// ===== BOOLEAN LOGIC TESTS =====

TEST_F(VMExtendedTest, BooleanValues) {
    std::vector<TypedValue> constants = {
        makeBool(true),
        makeBool(false)
    };

    std::vector<Instruction> instructions = {
        {OPCode::LOAD_BOOL, 0},   // true
        {OPCode::LOAD_BOOL, 1},   // false
        {OPCode::LOAD_BOOL, 0},   // true
        {OPCode::LOAD_BOOL, 1},   // false
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    bool val4 = vm.popBool();
    bool val3 = vm.popBool();
    bool val2 = vm.popBool();
    bool val1 = vm.popBool();

    EXPECT_EQ(val1, true);
    EXPECT_EQ(val2, false);
    EXPECT_EQ(val3, true);
    EXPECT_EQ(val4, false);
}

// ===== STACK MANIPULATION TESTS =====

TEST_F(VMExtendedTest, DeepStackOperations) {
    // Push many values and verify stack order
    std::vector<TypedValue> constants;
    std::vector<Instruction> instructions;

    // Push numbers 1 through 20
    for (int i = 1; i <= 20; i++) {
        constants.push_back(makeInt(i));
        instructions.push_back({OPCode::LOAD_INT, i-1});
    }

    instructions.push_back({OPCode::HALT, 0});

    vm.loadProgram(instructions, constants);
    vm.run();

    // Pop in reverse order (20, 19, 18, ..., 1)
    for (int i = 20; i >= 1; i--) {
        int value = vm.popInt();
        EXPECT_EQ(value, i);
    }
}

TEST_F(VMExtendedTest, AlternatingTypes) {
    std::vector<TypedValue> constants = {
        makeInt(1),
        makeFloat(2.0f),
        makeInt(3),
        makeDouble(4.0),
        makeBool(true)
    };

    std::vector<Instruction> instructions = {
        {OPCode::LOAD_INT, 0},      // 1
        {OPCode::LOAD_FLOAT, 1},    // 2.0f
        {OPCode::LOAD_INT, 2},      // 3
        {OPCode::LOAD_DOUBLE, 3},   // 4.0
        {OPCode::LOAD_BOOL, 4},     // true
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    bool boolVal = vm.popBool();
    double doubleVal = vm.popDouble();
    int intVal2 = vm.popInt();
    float floatVal = vm.popFloat();
    int intVal1 = vm.popInt();

    EXPECT_EQ(intVal1, 1);
    EXPECT_FLOAT_EQ(floatVal, 2.0f);
    EXPECT_EQ(intVal2, 3);
    EXPECT_DOUBLE_EQ(doubleVal, 4.0);
    EXPECT_EQ(boolVal, true);
}

// ===== PERFORMANCE STRESS TESTS =====

TEST_F(VMExtendedTest, LargeComputationChain) {
    // Test: Compute factorial of 10 using repeated multiplication
    // 1 * 2 * 3 * 4 * 5 * 6 * 7 * 8 * 9 * 10 = 3,628,800
    std::vector<TypedValue> constants;
    std::vector<Instruction> instructions;

    // Load 1 (accumulator)
    constants.push_back(makeInt(1));
    instructions.push_back({OPCode::LOAD_INT, 0});

    // Multiply by 2, 3, 4, ..., 10
    for (int i = 2; i <= 10; i++) {
        constants.push_back(makeInt(i));
        instructions.push_back({OPCode::LOAD_INT, static_cast<int>(constants.size() - 1)});
        instructions.push_back({OPCode::MULT_INT, 0});
    }

    instructions.push_back({OPCode::HALT, 0});

    vm.loadProgram(instructions, constants);
    vm.run();

    int result = vm.popInt();
    EXPECT_EQ(result, 3628800);  // 10!
}

TEST_F(VMExtendedTest, ManyStringInternsDuplicates) {
    // Test performance with many duplicate string interns
    std::vector<std::string> patterns = {"test", "hello", "world", "vm", "forge"};
    std::vector<size_t> all_ids;

    // Intern 1000 strings (200 of each pattern)
    for (int i = 0; i < 200; i++) {
        for (const auto& pattern : patterns) {
            size_t id = vm.internString(pattern + std::to_string(i % 10)); // Create some duplicates
            all_ids.push_back(id);
        }
    }

    // Verify all strings can be retrieved
    size_t idx = 0;
    for (int i = 0; i < 200; i++) {
        for (const auto& pattern : patterns) {
            std::string expected = pattern + std::to_string(i % 10);
            std::string actual = vm.getString(all_ids[idx]);
            EXPECT_EQ(actual, expected);
            idx++;
        }
    }
}

// ===== EDGE CASE COMBINATIONS =====

TEST_F(VMExtendedTest, TypeConversionChains) {
    // Test: int -> double -> comparison
    std::vector<TypedValue> constants = {
        makeInt(5),
        makeInt(5),
        makeDouble(5.1)
    };

    std::vector<Instruction> instructions = {
        // Convert two ints to doubles and compare
        {OPCode::LOAD_INT, 0},      // 5
        {OPCode::INT_TO_DOUBLE, 0}, // 5.0
        {OPCode::LOAD_INT, 1},      // 5
        {OPCode::INT_TO_DOUBLE, 0}, // 5.0
        {OPCode::EQ_DOUBLE, 0},     // 5.0 == 5.0 -> true

        // Compare converted int with double literal
        {OPCode::LOAD_INT, 0},      // 5
        {OPCode::INT_TO_DOUBLE, 0}, // 5.0
        {OPCode::LOAD_DOUBLE, 2},   // 5.1
        {OPCode::LT_DOUBLE, 0},     // 5.0 < 5.1 -> true

        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    bool result2 = vm.popBool();
    bool result1 = vm.popBool();

    EXPECT_EQ(result1, true);   // 5.0 == 5.0
    EXPECT_EQ(result2, true);   // 5.0 < 5.1
}

TEST_F(VMExtendedTest, MaxStackDepth) {
    // Test deep stack by loading many values without consuming them
    std::vector<TypedValue> constants;
    std::vector<Instruction> instructions;

    // Load 1000 integers
    for (int i = 0; i < 1000; i++) {
        constants.push_back(makeInt(i));
        instructions.push_back({OPCode::LOAD_INT, i});
    }

    instructions.push_back({OPCode::HALT, 0});

    vm.loadProgram(instructions, constants);
    vm.run();

    // Verify all values are correct (in reverse order)
    for (int i = 999; i >= 0; i--) {
        int value = vm.popInt();
        EXPECT_EQ(value, i);
    }
}

// ===== INSTRUCTION VERIFICATION TESTS =====

TEST_F(VMExtendedTest, AllOpcodesUsed) {
    // Ensure all opcodes work (even if simple)
    std::vector<TypedValue> constants = {
        makeInt(10), makeInt(5),
        makeFloat(3.0f), makeFloat(2.0f),
        makeDouble(7.0), makeDouble(2.0),
        makeBool(true),
        makeString("test")
    };

    std::vector<Instruction> instructions = {
        // Integer ops
        {OPCode::LOAD_INT, 0}, {OPCode::LOAD_INT, 1}, {OPCode::ADD_INT, 0},
        {OPCode::LOAD_INT, 0}, {OPCode::LOAD_INT, 1}, {OPCode::SUB_INT, 0},
        {OPCode::LOAD_INT, 0}, {OPCode::LOAD_INT, 1}, {OPCode::MULT_INT, 0},
        {OPCode::LOAD_INT, 0}, {OPCode::LOAD_INT, 1}, {OPCode::DIV_INT, 0},

        // Float ops
        {OPCode::LOAD_FLOAT, 2}, {OPCode::LOAD_FLOAT, 3}, {OPCode::ADD_FLOAT, 0},
        {OPCode::LOAD_FLOAT, 2}, {OPCode::LOAD_FLOAT, 3}, {OPCode::SUB_FLOAT, 0},
        {OPCode::LOAD_FLOAT, 2}, {OPCode::LOAD_FLOAT, 3}, {OPCode::MULT_FLOAT, 0},
        {OPCode::LOAD_FLOAT, 2}, {OPCode::LOAD_FLOAT, 3}, {OPCode::DIV_FLOAT, 0},

        // Double ops
        {OPCode::LOAD_DOUBLE, 4}, {OPCode::LOAD_DOUBLE, 5}, {OPCode::ADD_DOUBLE, 0},
        {OPCode::LOAD_DOUBLE, 4}, {OPCode::LOAD_DOUBLE, 5}, {OPCode::SUB_DOUBLE, 0},
        {OPCode::LOAD_DOUBLE, 4}, {OPCode::LOAD_DOUBLE, 5}, {OPCode::MULT_DOUBLE, 0},
        {OPCode::LOAD_DOUBLE, 4}, {OPCode::LOAD_DOUBLE, 5}, {OPCode::DIV_DOUBLE, 0},

        // Conversions
        {OPCode::LOAD_INT, 1}, {OPCode::INT_TO_DOUBLE, 0},
        {OPCode::LOAD_FLOAT, 3}, {OPCode::FLOAT_TO_DOUBLE, 0},

        // Comparisons
        {OPCode::LOAD_INT, 0}, {OPCode::LOAD_INT, 1}, {OPCode::EQ_INT, 0},
        {OPCode::LOAD_INT, 0}, {OPCode::LOAD_INT, 1}, {OPCode::LT_INT, 0},
        {OPCode::LOAD_DOUBLE, 4}, {OPCode::LOAD_DOUBLE, 5}, {OPCode::EQ_DOUBLE, 0},
        {OPCode::LOAD_DOUBLE, 4}, {OPCode::LOAD_DOUBLE, 5}, {OPCode::LT_DOUBLE, 0},

        // Other types
        {OPCode::LOAD_BOOL, 6},
        {OPCode::LOAD_STRING, 7},

        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    // Just verify it completes without error
    // Stack will have many values, but we won't verify all -
    // the important thing is no exceptions were thrown
    EXPECT_NO_THROW(vm.popStringId());
    EXPECT_NO_THROW(vm.popBool());
}

TEST_F(VMExtendedTest, EmptyConstantsTable) {
    // Test program with no constants
    std::vector<TypedValue> constants = {};
    std::vector<Instruction> instructions = {
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    EXPECT_NO_THROW(vm.run());
}

TEST_F(VMExtendedTest, SingleInstruction) {
    // Test minimal program
    std::vector<TypedValue> constants = {};
    std::vector<Instruction> instructions = {
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();
    // Should complete without error
}

// ===== MATHEMATICAL PROPERTIES TESTS =====

TEST_F(VMExtendedTest, CommutativeProperty) {
    // Test that a + b == b + a and a * b == b * a
    std::vector<TypedValue> constants = {makeInt(7), makeInt(13)};

    std::vector<Instruction> instructions = {
        // a + b
        {OPCode::LOAD_INT, 0}, {OPCode::LOAD_INT, 1}, {OPCode::ADD_INT, 0},
        // b + a
        {OPCode::LOAD_INT, 1}, {OPCode::LOAD_INT, 0}, {OPCode::ADD_INT, 0},
        // a * b
        {OPCode::LOAD_INT, 0}, {OPCode::LOAD_INT, 1}, {OPCode::MULT_INT, 0},
        // b * a
        {OPCode::LOAD_INT, 1}, {OPCode::LOAD_INT, 0}, {OPCode::MULT_INT, 0},
        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    int mult_ba = vm.popInt();
    int mult_ab = vm.popInt();
    int add_ba = vm.popInt();
    int add_ab = vm.popInt();

    EXPECT_EQ(add_ab, add_ba);   // 7 + 13 == 13 + 7
    EXPECT_EQ(mult_ab, mult_ba); // 7 * 13 == 13 * 7
}

TEST_F(VMExtendedTest, AssociativeProperty) {
    // Test that (a + b) + c == a + (b + c)
    std::vector<TypedValue> constants = {makeInt(2), makeInt(3), makeInt(5)};

    std::vector<Instruction> instructions = {
        // (a + b) + c = (2 + 3) + 5 = 10
        {OPCode::LOAD_INT, 0}, {OPCode::LOAD_INT, 1}, {OPCode::ADD_INT, 0},  // 2 + 3 = 5
        {OPCode::LOAD_INT, 2}, {OPCode::ADD_INT, 0},                        // 5 + 5 = 10

        // a + (b + c) = 2 + (3 + 5) = 10
        {OPCode::LOAD_INT, 1}, {OPCode::LOAD_INT, 2}, {OPCode::ADD_INT, 0},  // 3 + 5 = 8
        {OPCode::LOAD_INT, 0}, {OPCode::ADD_INT, 0},                        // 2 + 8 = 10

        {OPCode::HALT, 0}
    };

    vm.loadProgram(instructions, constants);
    vm.run();

    int result2 = vm.popInt();
    int result1 = vm.popInt();

    EXPECT_EQ(result1, 10);
    EXPECT_EQ(result2, 10);
    EXPECT_EQ(result1, result2);
}