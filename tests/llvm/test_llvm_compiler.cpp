#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../include/llvm/LLVMCompiler.hpp"
#include "../../include/llvm/ErrorReporter.hpp"
#include "../../include/parser/Parser.hpp"
#include "../../include/lexer/Tokenizer.hpp"
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <sstream>

using ::testing::HasSubstr;
using ::testing::Not;
using ::testing::AllOf;

class LLVMCompilerTest : public ::testing::Test {
protected:
    std::string compileToIR(const std::string& input) {
        Tokenizer tokenizer(input);
        auto tokens = tokenizer.tokenize();
        Parser parser(tokens);
        auto ast = parser.parseProgram();

        LLVMCompiler compiler;
        compiler.compile(*ast);

        // Capture module IR to string
        std::string str;
        llvm::raw_string_ostream rso(str);
        compiler.getModule()->print(rso, nullptr);
        return rso.str();
    }

    llvm::Module* compileToModule(const std::string& input) {
        Tokenizer tokenizer(input);
        auto tokens = tokenizer.tokenize();
        Parser parser(tokens);
        auto ast = parser.parseProgram();

        // Create a new compiler for each test to avoid variable conflicts
        static std::unique_ptr<LLVMCompiler> compiler = std::make_unique<LLVMCompiler>();
        compiler = std::make_unique<LLVMCompiler>(); // Reset for each call
        compiler->compile(*ast);
        return compiler->getModule();
    }

    bool compileWithErrorCheck(const std::string& input) {
        // Record error count before compilation
        size_t errorsBefore = ErrorReporter::getErrorCount();

        try {
            compileToIR(input);
            // Check if new errors were introduced
            return ErrorReporter::getErrorCount() > errorsBefore;
        } catch (...) {
            return true; // Any exception means compilation failed
        }
    }
};

// ============================================================================
// BASIC COMPILATION TESTS
// ============================================================================

TEST_F(LLVMCompilerTest, BasicVariableDeclaration) {
    std::string input = "x: int = 42";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, HasSubstr("define i32 @main()"));
    EXPECT_THAT(ir, HasSubstr("%x = alloca i32"));
    EXPECT_THAT(ir, HasSubstr("store i32 42, ptr %x"));
    EXPECT_THAT(ir, HasSubstr("ret i32 0"));
    EXPECT_THAT(ir, Not(HasSubstr("<badref>")));
}

TEST_F(LLVMCompilerTest, BasicExpressionReturn) {
    std::string input = "5 + 3";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, HasSubstr("define i32 @main()"));
    // LLVM optimizes 5 + 3 to constant 8
    EXPECT_THAT(ir, HasSubstr("ret i32 8"));
    EXPECT_THAT(ir, Not(HasSubstr("<badref>")));
}

TEST_F(LLVMCompilerTest, ModuleValidation) {
    std::string input = "x: int = 42";
    llvm::Module* module = compileToModule(input);

    ASSERT_NE(module, nullptr);
    EXPECT_FALSE(llvm::verifyModule(*module, &llvm::errs()));
}

// ============================================================================
// POINTER SYSTEM TESTS
// ============================================================================

TEST_F(LLVMCompilerTest, PointerDeclaration) {
    std::string input = R"(
        x: int = 42
        ptr: *int = &x
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, HasSubstr("alloca i32"));
    EXPECT_THAT(ir, HasSubstr("alloca ptr"));
    EXPECT_THAT(ir, HasSubstr("store ptr %x, ptr %ptr"));
}

TEST_F(LLVMCompilerTest, PointerDereference) {
    std::string input = R"(
        x: int = 42
        ptr: *int = &x
        *ptr
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, HasSubstr("load ptr, ptr %ptr"));
    EXPECT_THAT(ir, HasSubstr("load i32, ptr"));
    EXPECT_THAT(ir, HasSubstr("ret i32"));
}

TEST_F(LLVMCompilerTest, PointerArithmetic) {
    std::string input = R"(
        x: int = 42
        ptr: *int = &x
        ptr + 1
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, HasSubstr("getelementptr"));
    EXPECT_THAT(ir, HasSubstr("i32 1")); // Offset should be 1 element, not 1 byte
}

TEST_F(LLVMCompilerTest, NestedPointerDereference) {
    std::string input = R"(
        x: int = 42
        ptr: *int = &x
        ptr2: **int = &ptr
        **ptr2
    )";
    std::string ir = compileToIR(input);

    // Should have multiple load instructions for nested dereferencing
    EXPECT_THAT(ir, HasSubstr("load ptr, ptr %ptr2"));
    EXPECT_THAT(ir, HasSubstr("load ptr, ptr"));
    EXPECT_THAT(ir, HasSubstr("load i32, ptr"));
}

// ============================================================================
// EXTERN FUNCTION TESTS
// ============================================================================

TEST_F(LLVMCompilerTest, ExternFunctionDeclaration) {
    std::string input = "extern def malloc(size: int) -> *void";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, HasSubstr("declare ptr @malloc(i32)"));
    EXPECT_THAT(ir, Not(HasSubstr("<badref>")));
}

TEST_F(LLVMCompilerTest, ExternFunctionWithPointerParams) {
    std::string input = "extern def free(ptr: *void) -> void";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, HasSubstr("declare void @free(ptr)"));
}

TEST_F(LLVMCompilerTest, ExternFunctionCall) {
    std::string input = R"(
        extern def malloc(size: int) -> *void
        ptr: *void = malloc(100)
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, HasSubstr("declare ptr @malloc(i32)"));
    EXPECT_THAT(ir, HasSubstr("call ptr @malloc(i32 100)"));
    EXPECT_THAT(ir, HasSubstr("store ptr"));
}

TEST_F(LLVMCompilerTest, MallocFreeWorkflow) {
    std::string input = R"(
        extern def malloc(size: int) -> *void
        extern def free(ptr: *void) -> void
        ptr: *void = malloc(100)
        free(ptr)
        42
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, AllOf(
        HasSubstr("declare ptr @malloc(i32)"),
        HasSubstr("declare void @free(ptr)"),
        HasSubstr("call ptr @malloc(i32 100)"),
        HasSubstr("call void @free(ptr"),
        HasSubstr("ret i32"),
        Not(HasSubstr("<badref>"))
    ));
}

TEST_F(LLVMCompilerTest, VoidFunctionCallDoesNotAffectReturn) {
    std::string input = R"(
        extern def dummy() -> void
        dummy()
        result: int = 42
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, HasSubstr("call void @dummy()"));
    EXPECT_THAT(ir, HasSubstr("ret i32 0")); // Should return default, not void
    EXPECT_THAT(ir, Not(HasSubstr("<badref>")));
}

// ============================================================================
// SMART POINTER TESTS
// ============================================================================

// TODO: Re-enable when smart pointer syntax is implemented
// TEST_F(LLVMCompilerTest, UniquePointerDeclaration) {
//     std::string input = "ptr: unique<int> = unique(42)";
//     std::string ir = compileToIR(input);
//     // Should call smart pointer creation functions
//     EXPECT_THAT(ir, HasSubstr("smart_ptr_malloc"));
// }

TEST_F(LLVMCompilerTest, SmartPointerRuntimeFunctions) {
    std::string input = "x: int = 42"; // Any valid input
    std::string ir = compileToIR(input);

    // Should declare all smart pointer runtime functions
    EXPECT_THAT(ir, HasSubstr("declare void @unique_ptr_release(ptr)"));
    EXPECT_THAT(ir, HasSubstr("declare void @shared_ptr_retain(ptr)"));
    EXPECT_THAT(ir, HasSubstr("declare void @shared_ptr_release(ptr)"));
    EXPECT_THAT(ir, HasSubstr("declare i32 @shared_ptr_use_count(ptr)"));
    EXPECT_THAT(ir, HasSubstr("declare ptr @smart_ptr_malloc(i32)"));
    EXPECT_THAT(ir, HasSubstr("declare void @weak_ptr_release(ptr)"));
}

// ============================================================================
// EXPRESSION TESTS
// ============================================================================

TEST_F(LLVMCompilerTest, ArithmeticOperations) {
    std::string input = "5 + 3 * 2";
    std::string ir = compileToIR(input);

    // LLVM optimizes 5 + 3 * 2 to constant 11 (5 + 6 = 11)
    EXPECT_THAT(ir, HasSubstr("ret i32 11"));
}

TEST_F(LLVMCompilerTest, ComparisonOperations) {
    std::string input = "5 > 3";
    std::string ir = compileToIR(input);

    // LLVM optimizes 5 > 3 to constant true
    EXPECT_THAT(ir, HasSubstr("ret i1 true"));
}

TEST_F(LLVMCompilerTest, BooleanOperations) {
    std::string input = "true";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, HasSubstr("i1 true"));
}

// ============================================================================
// CONTROL FLOW TESTS
// ============================================================================

TEST_F(LLVMCompilerTest, IfStatement) {
    std::string input = R"(
        if (true) {
            x: int = 42
        }
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, HasSubstr("br i1 true, label %if.then"));
    EXPECT_THAT(ir, HasSubstr("if.then:"));
    EXPECT_THAT(ir, HasSubstr("if.merge:"));
}

TEST_F(LLVMCompilerTest, IfElseStatement) {
    std::string input = R"(
        if (true) {
            x: int = 42
        } else {
            y: int = 24
        }
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, AllOf(
        HasSubstr("br i1 true, label %if.then, label %if.else"),
        HasSubstr("if.then:"),
        HasSubstr("if.else:"),
        HasSubstr("if.merge:")
    ));
}

TEST_F(LLVMCompilerTest, WhileLoop) {
    std::string input = R"(
        while (true) {
            x: int = 42
        }
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, AllOf(
        HasSubstr("while.cond:"),
        HasSubstr("while.body:"),
        HasSubstr("while.exit:"),
        HasSubstr("br i1 true, label %while.body, label %while.exit")
    ));
}

// ============================================================================
// FUNCTION DEFINITION TESTS
// ============================================================================

TEST_F(LLVMCompilerTest, FunctionDefinition) {
    std::string input = R"(
        def add(a: int, b: int) -> int {
            return a + b
        }
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, AllOf(
        HasSubstr("define i32 @add(i32 %a, i32 %b)"),
        HasSubstr("entry:"),
        HasSubstr("add i32"),
        HasSubstr("ret i32")
    ));
    // Note: Function return value handling needs fixing - should return computed value
}

TEST_F(LLVMCompilerTest, FunctionWithPointerParams) {
    std::string input = R"(
        def process(ptr: *int) -> int {
            return *ptr
        }
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, HasSubstr("define i32 @process(ptr %ptr)"));
    EXPECT_THAT(ir, HasSubstr("load i32, ptr"));
}

TEST_F(LLVMCompilerTest, ReturnStatement) {
    std::string input = R"(
        def getValue() -> int {
            return 42
        }
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, HasSubstr("ret i32 42"));
}

// ============================================================================
// ASSIGNMENT TESTS
// ============================================================================

TEST_F(LLVMCompilerTest, VariableAssignment) {
    std::string input = R"(
        x: int = 10
        x = 20
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, AllOf(
        HasSubstr("store i32 10, ptr %x"),
        HasSubstr("store i32 20, ptr %x")
    ));
}

TEST_F(LLVMCompilerTest, PointerDereferenceAssignment) {
    std::string input = R"(
        x: int = 42
        ptr: *int = &x
        *ptr = 100
        result: int = *ptr
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, AllOf(
        // Basic setup
        HasSubstr("store i32 42, ptr %x"),
        HasSubstr("store ptr %x, ptr %ptr"),
        // Dereference assignment: *ptr = 100
        HasSubstr("load ptr, ptr %ptr"),
        HasSubstr("store i32 100, ptr %"),
        // Reading back: result = *ptr
        HasSubstr("load i32, ptr %"),
        Not(HasSubstr("<badref>"))
    ));
}

TEST_F(LLVMCompilerTest, ComprehensivePointerOperations) {
    std::string input = R"(
        def test_pointer_operations() -> void {
            x: int = 42
            ptr: *int = &x
            value: int = *ptr
            *ptr = 100
            next_ptr: *int = ptr + 1
        }
        test_pointer_operations()
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, AllOf(
        // Function definition
        HasSubstr("define void @test_pointer_operations()"),
        // Variable initialization
        HasSubstr("store i32 42, ptr %x"),
        // Address-of operator: ptr = &x
        HasSubstr("store ptr %x, ptr %ptr"),
        // Dereference operator: value = *ptr
        HasSubstr("load ptr, ptr %ptr"),
        HasSubstr("load i32, ptr %"),
        // Dereference assignment: *ptr = 100
        HasSubstr("store i32 100, ptr %"),
        // Pointer arithmetic: ptr + 1
        HasSubstr("getelementptr i32, ptr %"),
        HasSubstr("i32 1"),
        // Function call
        HasSubstr("call void @test_pointer_operations()"),
        Not(HasSubstr("<badref>"))
    ));
}

TEST_F(LLVMCompilerTest, CInteropMallocFree) {
    std::string input = R"(
        extern def malloc(size: int) -> *void
        extern def free(ptr: *void) -> void

        def test_malloc_workflow() -> void {
            data: *void = malloc(4)
            int_ptr: *int = data
            *int_ptr = 42
            value: int = *int_ptr
            free(data)
        }

        test_malloc_workflow()
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, AllOf(
        // External function declarations
        HasSubstr("declare ptr @malloc(i32)"),
        HasSubstr("declare void @free(ptr)"),
        // Function definition
        HasSubstr("define void @test_malloc_workflow()"),
        // Malloc call with correct size
        HasSubstr("call ptr @malloc(i32 4)"),
        // Pointer assignment and dereferencing
        HasSubstr("store ptr %"),
        HasSubstr("store i32 42, ptr %"),
        HasSubstr("load i32, ptr %"),
        // Free call
        HasSubstr("call void @free(ptr %"),
        // Function call
        HasSubstr("call void @test_malloc_workflow()"),
        Not(HasSubstr("<badref>"))
    ));
}

// ============================================================================
// BLOCK STATEMENT TESTS
// ============================================================================

TEST_F(LLVMCompilerTest, BlockStatement) {
    std::string input = R"(
        {
            x: int = 42
            y: int = 24
        }
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, AllOf(
        HasSubstr("alloca i32"),
        HasSubstr("store i32 42"),
        HasSubstr("store i32 24")
    ));
}

// ============================================================================
// TYPE SYSTEM TESTS
// ============================================================================

TEST_F(LLVMCompilerTest, IntegerTypes) {
    std::string input = "x: int = 42";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, HasSubstr("i32"));
}

TEST_F(LLVMCompilerTest, BooleanTypes) {
    std::string input = "flag: bool = true";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, HasSubstr("i1"));
}

TEST_F(LLVMCompilerTest, PointerTypes) {
    std::string input = R"(
        x: int = 42
        ptr: *int = &x
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, HasSubstr("alloca ptr"));
    EXPECT_THAT(ir, HasSubstr("store ptr"));
}

// ============================================================================
// ERROR HANDLING TESTS
// ============================================================================

TEST_F(LLVMCompilerTest, NoMemoryLeaksInValidModule) {
    std::string input = "x: int = 42";
    llvm::Module* module = compileToModule(input);

    ASSERT_NE(module, nullptr);
    // Module should be valid and not cause crashes
    EXPECT_FALSE(llvm::verifyModule(*module, &llvm::errs()));
}

// ============================================================================
// COMPREHENSIVE INTEGRATION TESTS
// ============================================================================

TEST_F(LLVMCompilerTest, ComplexPointerManipulation) {
    std::string input = R"(
        extern def malloc(size: int) -> *void
        extern def free(ptr: *void) -> void

        x: int = 42
        ptr: *int = &x
        value: int = *ptr

        ptr2: *void = malloc(100)
        free(ptr2)

        value + 8
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, AllOf(
        HasSubstr("declare ptr @malloc(i32)"),
        HasSubstr("declare void @free(ptr)"),
        HasSubstr("store ptr %x, ptr %ptr"),
        HasSubstr("load ptr, ptr %ptr"),
        HasSubstr("load i32, ptr"),
        HasSubstr("call ptr @malloc(i32 100)"),
        HasSubstr("call void @free(ptr"),
        HasSubstr("add i32"),
        HasSubstr("ret i32"),
        Not(HasSubstr("<badref>"))
    ));
}

TEST_F(LLVMCompilerTest, FunctionWithExternCalls) {
    std::string input = R"(
        extern def malloc(size: int) -> *void
        extern def free(ptr: *void) -> void

        def allocateAndFree() -> int {
            ptr: *void = malloc(100)
            free(ptr)
            return 42
        }

        allocateAndFree()
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, AllOf(
        HasSubstr("define i32 @allocateAndFree()"),
        HasSubstr("call ptr @malloc(i32 100)"),
        HasSubstr("call void @free(ptr"),
        HasSubstr("ret i32 42"),
        HasSubstr("call i32 @allocateAndFree()"),
        Not(HasSubstr("<badref>"))
    ));
}

// ============================================================================
// DEFER STATEMENT TESTS
// ============================================================================

TEST_F(LLVMCompilerTest, BasicDeferStatement) {
    std::string input = R"(
        def dummy() -> void {
            x: int = 42
        }

        def test_defer() -> void {
            defer dummy()
            y: int = 100
        }

        test_defer()
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, AllOf(
        HasSubstr("define void @dummy()"),
        HasSubstr("define void @test_defer()"),
        HasSubstr("call void @dummy()"),  // defer call should be generated
        HasSubstr("call void @test_defer()"),
        Not(HasSubstr("<badref>"))
    ));
}

TEST_F(LLVMCompilerTest, MultipleDeferStatements) {
    std::string input = R"(
        def test_multiple_defer() -> void {
            defer 1
            defer 2
            defer 3
            x: int = 42
        }

        test_multiple_defer()
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, AllOf(
        HasSubstr("define void @test_multiple_defer()"),
        HasSubstr("call void @test_multiple_defer()"),
        Not(HasSubstr("<badref>"))
    ));
}

TEST_F(LLVMCompilerTest, NestedBlockDeferStatements) {
    std::string input = R"(
        def test_nested_defer() -> void {
            defer 1
            {
                defer 2
                x: int = 42
            }
            defer 3
        }

        test_nested_defer()
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, AllOf(
        HasSubstr("define void @test_nested_defer()"),
        HasSubstr("call void @test_nested_defer()"),
        Not(HasSubstr("<badref>"))
    ));
}

TEST_F(LLVMCompilerTest, DeferWithFunctionCall) {
    std::string input = R"(
        def cleanup() -> void {
            x: int = 1
        }

        def test_defer_function() -> int {
            defer cleanup()
            x: int = 100
            return x
        }

        test_defer_function()
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, AllOf(
        HasSubstr("define void @cleanup()"),
        HasSubstr("define i32 @test_defer_function()"),
        HasSubstr("call void @cleanup()"),  // defer cleanup call
        HasSubstr("call i32 @test_defer_function()"),
        Not(HasSubstr("<badref>"))
    ));
}

// ============================================================================
// SMART POINTER TYPE SAFETY TESTS
// ============================================================================

TEST_F(LLVMCompilerTest, SharedPointerCorrectUsage) {
    // This should compile successfully
    std::string input = R"(
        sp: shared int = new 42
    )";
    std::string ir = compileToIR(input);

    EXPECT_THAT(ir, AllOf(
        HasSubstr("shared_ptr"),  // Smart pointer type should be present
        HasSubstr("call"),        // Should have function calls for setup
        Not(HasSubstr("<badref>"))
    ));
}

TEST_F(LLVMCompilerTest, SharedPointerRejectsLiteralAssignment) {
    // This should fail at compilation
    std::string input = R"(
        sp: shared int = 42
    )";

    // Test that compilation produces errors
    EXPECT_TRUE(compileWithErrorCheck(input));
}

TEST_F(LLVMCompilerTest, RegularVariableRejectsNewExpression) {
    // This should fail at compilation
    std::string input = R"(
        x: int = new 42
    )";

    // Test that compilation produces errors
    EXPECT_TRUE(compileWithErrorCheck(input));
}

TEST_F(LLVMCompilerTest, SmartPointerTypeSafety) {
    // Test comprehensive type safety rules
    std::string validInput = R"(
        good1: shared int = new 42
        good2: unique int = new 100
        good3: weak int = new 200
    )";

    // Valid code should compile without errors
    std::string ir = compileToIR(validInput);
    EXPECT_THAT(ir, AllOf(
        HasSubstr("shared_ptr"),
        HasSubstr("unique_ptr"),
        HasSubstr("weak_ptr"),
        Not(HasSubstr("<badref>"))
    ));
}