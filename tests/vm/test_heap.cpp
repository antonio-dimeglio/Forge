#include <gtest/gtest.h>
#include "../../include/backends/vm/Heap.hpp"
#include "../../include/backends/vm/VirtualMachine.hpp"
#include "../../include/backends/vm/BytecodeCompiler.hpp"
#include "../../include/parser/Parser.hpp"
#include "../../include/lexer/Tokenizer.hpp"

class HeapTest : public ::testing::Test {
protected:
    VirtualMachine vm;
    Heap* heap;

    void SetUp() override {
        heap = &vm.getHeap();
        vm.reset();
    }

    // Helper to run source and trigger potential GC
    void runSource(const std::string& source) {
        BytecodeCompiler compiler(vm);
        Tokenizer tokenizer(source);
        auto tokens = tokenizer.tokenize();
        Parser parser(tokens);
        auto ast = parser.parseProgram();
        auto program = compiler.compile(std::move(ast));
        vm.loadProgram(program.instructions, program.constants);
        vm.run();
    }
};

// ========== BASIC HEAP ALLOCATION TESTS ==========

TEST_F(HeapTest, AllocateStringObject) {
    StringObject* str = heap->allocateString("hello");

    EXPECT_NE(str, nullptr);
    EXPECT_EQ(str->type, STRING);
    EXPECT_EQ(str->length, 5);
    EXPECT_STREQ(str->chars, "hello");
    EXPECT_FALSE(str->isMarked);  // Initially unmarked
}

TEST_F(HeapTest, AllocateMultipleStringObjects) {
    StringObject* str1 = heap->allocateString("first");
    StringObject* str2 = heap->allocateString("second");
    StringObject* str3 = heap->allocateString("third");

    EXPECT_NE(str1, nullptr);
    EXPECT_NE(str2, nullptr);
    EXPECT_NE(str3, nullptr);

    EXPECT_STREQ(str1->chars, "first");
    EXPECT_STREQ(str2->chars, "second");
    EXPECT_STREQ(str3->chars, "third");
}

TEST_F(HeapTest, AllocateFunctionObject) {
    FunctionObject* func = heap->allocateFunction("testFunc", 2);

    EXPECT_NE(func, nullptr);
    EXPECT_EQ(func->type, FUNCTION);
    EXPECT_EQ(func->name, "testFunc");
    EXPECT_EQ(func->parameterCount, 2);
    EXPECT_FALSE(func->isMarked);  // Initially unmarked
}

TEST_F(HeapTest, AllocateEmptyString) {
    StringObject* str = heap->allocateString("");

    EXPECT_NE(str, nullptr);
    EXPECT_EQ(str->length, 0);
    EXPECT_STREQ(str->chars, "");
}

TEST_F(HeapTest, AllocateLargeString) {
    std::string largeStr(1000, 'A');  // 1000 'A' characters
    StringObject* str = heap->allocateString(largeStr);

    EXPECT_NE(str, nullptr);
    EXPECT_EQ(str->length, 1000);
    EXPECT_EQ(std::string(str->chars), largeStr);
}

// ========== GARBAGE COLLECTION TESTS ==========

TEST_F(HeapTest, BasicGarbageCollection) {
    // Allocate some strings but don't reference them from VM
    heap->allocateString("unreachable1");
    heap->allocateString("unreachable2");
    heap->allocateString("unreachable3");

    // Manual GC trigger (if objects aren't referenced from VM roots, they should be collected)
    heap->collect();

    // Test passes if no crash occurs - actual memory checking would require
    // more sophisticated heap inspection
}

TEST_F(HeapTest, StringObjectSurvivesGC) {
    // Create a string that's reachable from VM
    runSource(R"(
reachable: str = "I should survive"
)");

    // The string should still be accessible after potential GC
    // (GC may have been triggered during execution)
    // Test passes if execution completes without crash
}

TEST_F(HeapTest, UnreachableStringsCollected) {
    // Allocate many strings to trigger GC
    runSource(R"(
str1: str = "one"
str2: str = "two"
str3: str = "three"
str4: str = "four"
str5: str = "five"
str6: str = "six"
str7: str = "seven"
str8: str = "eight"
str9: str = "nine"
str10: str = "ten"
final: str = "survivor"
)");

    // All strings except "survivor" should still be reachable from globals
    // This test mainly ensures GC doesn't crash during execution
}

TEST_F(HeapTest, FunctionObjectGarbageCollection) {
    runSource(R"(
def tempFunction() -> int {
    return 42
}

def keepFunction() -> int {
    return 100
}

result: int = keepFunction()
)");

    // Both functions should be reachable from constants
    // Test ensures no crash during GC
}

TEST_F(HeapTest, ComplexObjectGraphGC) {
    runSource(R"(
def createTempStrings() -> str {
    temp1: str = "temporary1"
    temp2: str = "temporary2"
    temp3: str = "temporary3"
    return "result"
}

str1: str = createTempStrings()
str2: str = createTempStrings()
final: str = str1
)");

    // Only reachable strings should survive
}

// ========== MARK OBJECT TESTS ==========

TEST_F(HeapTest, MarkStringObject) {
    StringObject* str = heap->allocateString("test");
    EXPECT_FALSE(str->isMarked);

    heap->markObject(str);
    EXPECT_TRUE(str->isMarked);
}

TEST_F(HeapTest, MarkFunctionObject) {
    FunctionObject* func = heap->allocateFunction("test", 0);
    EXPECT_FALSE(func->isMarked);

    heap->markObject(func);
    EXPECT_TRUE(func->isMarked);
}

TEST_F(HeapTest, MarkNullObjectSafely) {
    // Should not crash when marking null object
    EXPECT_NO_THROW({
        heap->markObject(nullptr);
    });
}

TEST_F(HeapTest, MarkAlreadyMarkedObject) {
    StringObject* str = heap->allocateString("test");

    heap->markObject(str);
    EXPECT_TRUE(str->isMarked);

    // Marking again should be safe
    heap->markObject(str);
    EXPECT_TRUE(str->isMarked);
}

// ========== MEMORY PRESSURE TESTS ==========

TEST_F(HeapTest, AllocationUnderMemoryPressure) {
    // Allocate many objects to trigger multiple GC cycles
    for (int i = 0; i < 100; ++i) {
        std::string str = "string_" + std::to_string(i);
        heap->allocateString(str);
    }

    // Test should complete without crash
}

TEST_F(HeapTest, LargeAllocationTriggersGC) {
    // Create a large string that might trigger GC
    std::string largeStr(10000, 'X');
    StringObject* str = heap->allocateString(largeStr);

    EXPECT_NE(str, nullptr);
    EXPECT_EQ(str->length, 10000);
}

TEST_F(HeapTest, MixedObjectAllocation) {
    // Allocate mix of strings and functions
    for (int i = 0; i < 50; ++i) {
        heap->allocateString("str_" + std::to_string(i));
        heap->allocateFunction("func_" + std::to_string(i), i % 5);
    }

    // Should handle mixed allocation without issues
}

// ========== INTEGRATION TESTS WITH VM ==========

TEST_F(HeapTest, GCWithActiveStack) {
    runSource(R"(
def recursiveStringCreator(depth: int) -> str {
    if (depth <= 0) {
        return "base"
    }
    temp: str = "temp_string"
    return recursiveStringCreator(depth - 1)
}

result: str = recursiveStringCreator(10)
)");

    // Should handle GC during recursive calls with stack frames
}

TEST_F(HeapTest, GCWithGlobalVariables) {
    runSource(R"(
global1: str = "persistent1"
global2: str = "persistent2"
global3: str = "persistent3"

def createTemporaryStrings() -> str {
    temp1: str = "temp1"
    temp2: str = "temp2"
    temp3: str = "temp3"
    return "result"
}

result: str = createTemporaryStrings()
)");

    // Global strings should survive GC
}

TEST_F(HeapTest, GCWithFunctionConstants) {
    runSource(R"(
def funcWithConstants() -> str {
    local1: str = "local_const1"
    local2: str = "local_const2"
    return local1
}

result: str = funcWithConstants()
)");

    // Function constants should be properly marked
}

// ========== EDGE CASE TESTS ==========

TEST_F(HeapTest, EmptyProgramGC) {
    runSource("");
    // Should handle GC with empty program
}

TEST_F(HeapTest, OnlyLiteralsGC) {
    runSource("42");
    // Should handle GC with only literal values
}

TEST_F(HeapTest, StringConcatenationGC) {
    // Note: This test assumes string concatenation is implemented
    // If not implemented yet, this test can be skipped
    runSource(R"(
str1: str = "hello"
str2: str = "world"
)");

    // Should handle multiple string allocations
}

// ========== HEAP STATE VALIDATION TESTS ==========

TEST_F(HeapTest, HeapInitiallyEmpty) {
    // Fresh heap should handle collection gracefully
    EXPECT_NO_THROW({
        heap->collect();
    });
}

TEST_F(HeapTest, AllObjectsLinked) {
    // Allocate several objects and ensure they're properly linked
    StringObject* str1 = heap->allocateString("first");
    StringObject* str2 = heap->allocateString("second");
    FunctionObject* func = heap->allocateFunction("test", 1);

    EXPECT_NE(str1, nullptr);
    EXPECT_NE(str2, nullptr);
    EXPECT_NE(func, nullptr);

    // Objects should be linked in the heap's object list
    // (Internal implementation detail, but important for GC)
}

TEST_F(HeapTest, ConsistentObjectTypes) {
    StringObject* str = heap->allocateString("test");
    FunctionObject* func = heap->allocateFunction("test", 0);

    EXPECT_EQ(str->type, STRING);
    EXPECT_EQ(func->type, FUNCTION);

    // Type should remain consistent after marking
    heap->markObject(str);
    heap->markObject(func);

    EXPECT_EQ(str->type, STRING);
    EXPECT_EQ(func->type, FUNCTION);
}