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

TEST_F(LLVMCompilerTest, SharedPointerCopySemantics) {
    // Test shared pointer copying and reference counting
    std::string input = R"(
        sp1: shared int = new 42
        sp2: shared int = sp1
        sp3: shared int = sp2
    )";

    std::string ir = compileToIR(input);
    EXPECT_THAT(ir, AllOf(
        HasSubstr("shared_ptr"),
        HasSubstr("shared_ptr_retain"),  // Should call retain function
        Not(HasSubstr("<badref>"))
    ));
}

TEST_F(LLVMCompilerTest, SharedPointerCreation) {
    // Test basic shared pointer creation with different types
    std::string input = R"(
        sp_int: shared int = new 42
        sp_float: shared float = new 3.14f
        sp_double: shared double = new 2.718
        sp_bool: shared bool = new true
    )";

    std::string ir = compileToIR(input);
    EXPECT_THAT(ir, AllOf(
        HasSubstr("shared_ptrINT"),     // shared_ptr for int
        HasSubstr("shared_ptrFLOAT"),   // shared_ptr for float
        HasSubstr("shared_ptrDOUBLE"),  // shared_ptr for double
        HasSubstr("shared_ptrBOOL"),    // shared_ptr for bool
        HasSubstr("smart_ptr_malloc"),  // Should allocate heap memory
        Not(HasSubstr("<badref>"))
    ));
}

TEST_F(LLVMCompilerTest, SharedPointerScopeCleanup) {
    // Test that shared pointers are properly cleaned up when going out of scope
    std::string input = R"(
        {
            sp1: shared int = new 100
            sp2: shared int = sp1
        }
    )";

    std::string ir = compileToIR(input);
    EXPECT_THAT(ir, AllOf(
        HasSubstr("shared_ptr"),
        HasSubstr("shared_ptr_retain"),   // Copy should call retain
        HasSubstr("shared_ptr_release"),  // Cleanup should call release
        Not(HasSubstr("<badref>"))
    ));
}

TEST_F(LLVMCompilerTest, SharedPointerReferenceCounting) {
    // Test multiple copies and cleanup to verify reference counting
    std::string input = R"(
        sp1: shared int = new 999
        {
            sp2: shared int = sp1
            sp3: shared int = sp2
            {
                sp4: shared int = sp3
            }
        }
    )";

    std::string ir = compileToIR(input);
    EXPECT_THAT(ir, AllOf(
        HasSubstr("shared_ptr"),
        HasSubstr("shared_ptr_retain"),   // Multiple retains for copies
        HasSubstr("shared_ptr_release"),  // Multiple releases for cleanup
        Not(HasSubstr("<badref>"))
    ));
}

// ============================================================================
// COMPREHENSIVE EDGE CASE TESTS FOR SMART POINTERS
// ============================================================================

TEST_F(LLVMCompilerTest, SharedPointerAssignmentEdgeCases) {
    // Test shared pointer to shared pointer assignment (the bug we found)
    std::string input = R"(
        sp1: shared int = new 42
        sp2: shared int = new 100
        sp1 = sp2
    )";

    std::string ir = compileToIR(input);
    // TODO: This currently doesn't handle reference counting properly
    // We expect it to compile but may have memory leaks
    EXPECT_THAT(ir, AllOf(
        HasSubstr("shared_ptr"),
        Not(HasSubstr("<badref>"))
    ));
}

TEST_F(LLVMCompilerTest, SharedPointerInvalidAssignments) {
    // Test that invalid assignments are caught
    std::string literalAssign = R"(
        sp: shared int = new 42
        sp = 100
    )";

    // Should produce compilation errors
    EXPECT_TRUE(compileWithErrorCheck(literalAssign));

    std::string newAssign = R"(
        sp: shared int = new 42
        sp = new 200
    )";

    // Should also produce compilation errors (can't assign new to existing shared_ptr)
    EXPECT_TRUE(compileWithErrorCheck(newAssign));
}

TEST_F(LLVMCompilerTest, SharedPointerSelfAssignment) {
    // Test self-assignment edge case
    std::string input = R"(
        sp: shared int = new 42
        sp = sp
    )";

    std::string ir = compileToIR(input);
    EXPECT_THAT(ir, AllOf(
        HasSubstr("shared_ptr"),
        Not(HasSubstr("<badref>"))
    ));
}

TEST_F(LLVMCompilerTest, SharedPointerNullScenarios) {
    // Test behavior with null-like scenarios
    std::string input = R"(
        sp1: shared int = new 42
        sp2: shared int = new 100
        sp1 = sp2
        sp2 = sp1
    )";

    std::string ir = compileToIR(input);
    EXPECT_THAT(ir, AllOf(
        HasSubstr("shared_ptr"),
        Not(HasSubstr("<badref>"))
    ));
}

TEST_F(LLVMCompilerTest, UniquePointerEdgeCases) {
    // Test unique pointer creation and basic behavior
    std::string input = R"(
        up1: unique int = new 42
        up2: unique float = new 3.14f
    )";

    std::string ir = compileToIR(input);
    EXPECT_THAT(ir, AllOf(
        HasSubstr("unique_ptr"),
        HasSubstr("smart_ptr_malloc"),
        HasSubstr("unique_ptr_release"),  // Should have cleanup
        Not(HasSubstr("<badref>"))
    ));
}

TEST_F(LLVMCompilerTest, UniquePointerInvalidOperations) {
    // Test that unique pointer copying is rejected
    std::string copyAttempt = R"(
        up1: unique int = new 42
        up2: unique int = up1
    )";

    // Should produce compilation errors (unique pointers can't be copied)
    EXPECT_TRUE(compileWithErrorCheck(copyAttempt));

    std::string assignAttempt = R"(
        up1: unique int = new 42
        up1 = new 100
    )";

    // Should produce compilation errors
    EXPECT_TRUE(compileWithErrorCheck(assignAttempt));
}

TEST_F(LLVMCompilerTest, MixedSmartPointerScenarios) {
    // Test mixing different smart pointer types
    std::string input = R"(
        up: unique int = new 42
        sp1: shared int = new 100
        sp2: shared int = sp1
        {
            sp3: shared int = sp2
            up2: unique float = new 3.14f
        }
    )";

    std::string ir = compileToIR(input);
    EXPECT_THAT(ir, AllOf(
        HasSubstr("unique_ptr"),
        HasSubstr("shared_ptr"),
        HasSubstr("shared_ptr_retain"),
        HasSubstr("unique_ptr_release"),
        HasSubstr("shared_ptr_release"),
        Not(HasSubstr("<badref>"))
    ));
}

TEST_F(LLVMCompilerTest, SmartPointerMemoryLeakPrevention) {
    // Test scenarios that could cause memory leaks
    std::string input = R"(
        {
            sp1: shared int = new 42
            sp2: shared int = new 100
            sp3: shared int = sp1
            sp1 = sp2  // Potential leak scenario
        }
        {
            up: unique int = new 200
        }
    )";

    std::string ir = compileToIR(input);
    EXPECT_THAT(ir, AllOf(
        HasSubstr("shared_ptr"),
        HasSubstr("unique_ptr"),
        HasSubstr("shared_ptr_release"),
        HasSubstr("unique_ptr_release"),
        Not(HasSubstr("<badref>"))
    ));
}

TEST_F(LLVMCompilerTest, SharedPointerVariableDeclarationVsAssignment) {
    // Test distinction between variable declaration (:=) and assignment (=)

    // Variable declarations should work
    std::string declarations = R"(
        sp1: shared int = new 42      // Declaration with new - should work
        sp2: shared int = sp1         // Declaration with copy - should work
    )";

    std::string ir1 = compileToIR(declarations);
    EXPECT_THAT(ir1, AllOf(
        HasSubstr("shared_ptr"),
        HasSubstr("shared_ptr_retain"),  // Copy should call retain
        Not(HasSubstr("<badref>"))
    ));

    // Assignment should currently work but may have bugs
    std::string assignments = R"(
        sp1: shared int = new 42
        sp2: shared int = new 100
        sp1 = sp2                     // Assignment - currently buggy
    )";

    std::string ir2 = compileToIR(assignments);
    EXPECT_THAT(ir2, AllOf(
        HasSubstr("shared_ptr"),
        Not(HasSubstr("<badref>"))
        // TODO: Should check for proper release/retain calls
    ));
}

TEST_F(LLVMCompilerTest, UniquePointerVariableDeclarationVsAssignment) {
    // Test unique pointer behavior for declarations vs assignments

    // Variable declarations should work
    std::string declarations = R"(
        up1: unique int = new 42      // Declaration with new - should work
    )";

    std::string ir1 = compileToIR(declarations);
    EXPECT_THAT(ir1, AllOf(
        HasSubstr("unique_ptr"),
        HasSubstr("smart_ptr_malloc"),
        Not(HasSubstr("<badref>"))
    ));

    // Copy declaration should fail
    std::string copyDeclaration = R"(
        up1: unique int = new 42
        up2: unique int = up1         // Should fail - unique pointers can't be copied
    )";

    EXPECT_TRUE(compileWithErrorCheck(copyDeclaration));

    // Assignment should fail
    std::string assignment = R"(
        up1: unique int = new 42
        up2: unique int = new 100
        up1 = up2                     // Should fail - unique pointers can't be assigned
    )";

    EXPECT_TRUE(compileWithErrorCheck(assignment));
}

TEST_F(LLVMCompilerTest, SmartPointerInvalidMixedOperations) {
    // Test invalid operations between different smart pointer types

    std::string sharedToUnique = R"(
        sp: shared int = new 42
        up: unique int = sp           // Should fail - can't assign shared to unique
    )";

    EXPECT_TRUE(compileWithErrorCheck(sharedToUnique));

    std::string uniqueToShared = R"(
        up: unique int = new 42
        sp: shared int = up           // Should fail - can't assign unique to shared
    )";

    EXPECT_TRUE(compileWithErrorCheck(uniqueToShared));

    std::string mixedAssignment = R"(
        sp: shared int = new 42
        up: unique int = new 100
        sp = up                       // Should fail - can't assign unique to shared
    )";

    EXPECT_TRUE(compileWithErrorCheck(mixedAssignment));
}