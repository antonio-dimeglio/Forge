#include <gtest/gtest.h>
#include <memory>
#include <vector>

// Include all necessary headers for TypeChecker testing
#include "../../include/backend/types/TypeChecker.hpp"
#include "../../include/backend/types/PrimitiveType.hpp"
#include "../../include/backend/types/FunctionType.hpp"
#include "../../include/backend/types/ReferenceType.hpp"
#include "../../include/backend/types/SmartPointerType.hpp"
#include "../../include/backend/types/PointerType.hpp"
#include "../../include/backend/errors/ErrorReporter.hpp"
#include "../../include/backend/codegen/SymbolTable.hpp"
#include "../../include/lexer/TokenType.hpp"

using namespace forge::types;
using namespace forge::errors;
using namespace forge::codegen;

// =============================================================================
// TEST FIXTURE FOR TYPECHECKER FUNCTION CALL VALIDATION
// =============================================================================

class TypeCheckerFunctionCallTest : public ::testing::Test {
protected:
    void SetUp() override {
        errorReporter = std::make_unique<ErrorReporter>();
        typeChecker = std::make_unique<TypeChecker>(*errorReporter);
        location = SourceLocation{"test.forge", 1, 1, 1};
    }

    void TearDown() override {
        errorReporter.reset();
        typeChecker.reset();
    }

    // Helper function to create primitive types
    std::unique_ptr<Type> createPrimitiveType(TokenType tokenType) {
        return std::make_unique<PrimitiveType>(tokenType);
    }

    // Helper function to create function types
    std::unique_ptr<FunctionType> createFunctionType(
        std::unique_ptr<Type> returnType,
        std::vector<std::unique_ptr<Type>> paramTypes,
        bool isVariadic = false
    ) {
        return std::make_unique<FunctionType>(
            std::move(returnType),
            std::move(paramTypes),
            isVariadic
        );
    }

    // Helper function to create reference types
    std::unique_ptr<ReferenceType> createReferenceType(std::unique_ptr<Type> pointedType, bool isMutable) {
        return std::make_unique<ReferenceType>(std::move(pointedType), isMutable);
    }

    // Helper function to create smart pointer types
    std::unique_ptr<SmartPointerType> createSmartPointerType(std::unique_ptr<Type> elementType, PointerKind kind) {
        return std::make_unique<SmartPointerType>(std::move(elementType), kind);
    }

    // Helper function to create pointer types
    std::unique_ptr<PointerType> createPointerType(std::unique_ptr<Type> pointeeType) {
        return std::make_unique<PointerType>(std::move(pointeeType));
    }

    // Helper function to convert unique_ptr vector to raw pointer vector
    std::vector<Type*> toRawPointers(const std::vector<std::unique_ptr<Type>>& types) {
        std::vector<Type*> rawPointers;
        for (const auto& type : types) {
            rawPointers.push_back(type.get());
        }
        return rawPointers;
    }

    std::unique_ptr<ErrorReporter> errorReporter;
    std::unique_ptr<TypeChecker> typeChecker;
    SourceLocation location;
};

// =============================================================================
// BASIC FUNCTION CALL VALIDATION TESTS
// =============================================================================

TEST_F(TypeCheckerFunctionCallTest, ValidFunctionCallNoParameters) {
    // Test: fn() -> int with no arguments
    auto returnType = createPrimitiveType(TokenType::INT);
    std::vector<std::unique_ptr<Type>> paramTypes;
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    std::vector<Type*> args;
    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isOk());
    EXPECT_TRUE(result.unwrap());
}

TEST_F(TypeCheckerFunctionCallTest, ValidFunctionCallSingleParameter) {
    // Test: fn(int) -> bool with int argument
    auto returnType = createPrimitiveType(TokenType::BOOL);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createPrimitiveType(TokenType::INT));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto intArg = createPrimitiveType(TokenType::INT);
    std::vector<Type*> args = {intArg.get()};
    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isOk());
    EXPECT_TRUE(result.unwrap());
}

TEST_F(TypeCheckerFunctionCallTest, ValidFunctionCallMultipleParameters) {
    // Test: fn(int, float, bool) -> string with matching arguments
    auto returnType = createPrimitiveType(TokenType::STRING);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createPrimitiveType(TokenType::INT));
    paramTypes.push_back(createPrimitiveType(TokenType::FLOAT));
    paramTypes.push_back(createPrimitiveType(TokenType::BOOL));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto intArg = createPrimitiveType(TokenType::INT);
    auto floatArg = createPrimitiveType(TokenType::FLOAT);
    auto boolArg = createPrimitiveType(TokenType::BOOL);
    std::vector<Type*> args = {intArg.get(), floatArg.get(), boolArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isOk());
    EXPECT_TRUE(result.unwrap());
}

// =============================================================================
// PARAMETER COUNT MISMATCH TESTS
// =============================================================================

TEST_F(TypeCheckerFunctionCallTest, TooFewArguments) {
    // Test: fn(int, float) -> void called with only int
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createPrimitiveType(TokenType::INT));
    paramTypes.push_back(createPrimitiveType(TokenType::FLOAT));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto intArg = createPrimitiveType(TokenType::INT);
    std::vector<Type*> args = {intArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isErr());
    EXPECT_TRUE(result.error().getMessage().find("Argument count mismatch") != std::string::npos);
    EXPECT_TRUE(result.error().getMessage().find("Expected 2") != std::string::npos);
    EXPECT_TRUE(result.error().getMessage().find("got 1") != std::string::npos);
}

TEST_F(TypeCheckerFunctionCallTest, TooManyArguments) {
    // Test: fn(int) -> void called with int, float
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createPrimitiveType(TokenType::INT));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto intArg = createPrimitiveType(TokenType::INT);
    auto floatArg = createPrimitiveType(TokenType::FLOAT);
    std::vector<Type*> args = {intArg.get(), floatArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isErr());
    EXPECT_TRUE(result.error().getMessage().find("Argument count mismatch") != std::string::npos);
    EXPECT_TRUE(result.error().getMessage().find("Expected 1") != std::string::npos);
    EXPECT_TRUE(result.error().getMessage().find("got 2") != std::string::npos);
}

TEST_F(TypeCheckerFunctionCallTest, NoArgumentsExpectedButProvided) {
    // Test: fn() -> void called with arguments
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto intArg = createPrimitiveType(TokenType::INT);
    std::vector<Type*> args = {intArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isErr());
    EXPECT_TRUE(result.error().getMessage().find("Expected 0") != std::string::npos);
    EXPECT_TRUE(result.error().getMessage().find("got 1") != std::string::npos);
}

TEST_F(TypeCheckerFunctionCallTest, ArgumentsExpectedButNoneProvided) {
    // Test: fn(int, float) -> void called with no arguments
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createPrimitiveType(TokenType::INT));
    paramTypes.push_back(createPrimitiveType(TokenType::FLOAT));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    std::vector<Type*> args;

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isErr());
    EXPECT_TRUE(result.error().getMessage().find("Expected 2") != std::string::npos);
    EXPECT_TRUE(result.error().getMessage().find("got 0") != std::string::npos);
}

// =============================================================================
// PARAMETER TYPE MISMATCH TESTS
// =============================================================================

TEST_F(TypeCheckerFunctionCallTest, SingleParameterTypeMismatch) {
    // Test: fn(int) -> void called with float
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createPrimitiveType(TokenType::INT));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto floatArg = createPrimitiveType(TokenType::FLOAT);
    std::vector<Type*> args = {floatArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isErr());
    EXPECT_TRUE(result.error().getMessage().find("Type mismatch for argument 1") != std::string::npos);
    EXPECT_TRUE(result.error().getMessage().find("Expected int") != std::string::npos);
    EXPECT_TRUE(result.error().getMessage().find("got float") != std::string::npos);
}

TEST_F(TypeCheckerFunctionCallTest, MultipleParameterTypeMismatch_FirstArgument) {
    // Test: fn(int, float) -> void called with (float, float)
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createPrimitiveType(TokenType::INT));
    paramTypes.push_back(createPrimitiveType(TokenType::FLOAT));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto floatArg1 = createPrimitiveType(TokenType::FLOAT);
    auto floatArg2 = createPrimitiveType(TokenType::FLOAT);
    std::vector<Type*> args = {floatArg1.get(), floatArg2.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isErr());
    EXPECT_TRUE(result.error().getMessage().find("Type mismatch for argument 1") != std::string::npos);
}

TEST_F(TypeCheckerFunctionCallTest, MultipleParameterTypeMismatch_SecondArgument) {
    // Test: fn(int, float) -> void called with (int, bool)
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createPrimitiveType(TokenType::INT));
    paramTypes.push_back(createPrimitiveType(TokenType::FLOAT));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto intArg = createPrimitiveType(TokenType::INT);
    auto boolArg = createPrimitiveType(TokenType::BOOL);
    std::vector<Type*> args = {intArg.get(), boolArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isErr());
    EXPECT_TRUE(result.error().getMessage().find("Type mismatch for argument 2") != std::string::npos);
    EXPECT_TRUE(result.error().getMessage().find("Expected float") != std::string::npos);
    EXPECT_TRUE(result.error().getMessage().find("got bool") != std::string::npos);
}

TEST_F(TypeCheckerFunctionCallTest, StringVsCharPointerMismatch) {
    // Test: fn(string) -> void called with different string-like types
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createPrimitiveType(TokenType::STRING));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto charArg = createPrimitiveType(TokenType::INT); // Using INT to represent char incompatibility
    std::vector<Type*> args = {charArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isErr());
}

// =============================================================================
// REFERENCE TYPE TESTS
// =============================================================================

TEST_F(TypeCheckerFunctionCallTest, ValidImmutableReferenceParameter) {
    // Test: fn(&int) -> void called with &int
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createReferenceType(createPrimitiveType(TokenType::INT), false));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto intRefArg = createReferenceType(createPrimitiveType(TokenType::INT), false);
    std::vector<Type*> args = {intRefArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isOk());
    EXPECT_TRUE(result.unwrap());
}

TEST_F(TypeCheckerFunctionCallTest, ValidMutableReferenceParameter) {
    // Test: fn(&mut int) -> void called with &mut int
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createReferenceType(createPrimitiveType(TokenType::INT), true));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto intMutRefArg = createReferenceType(createPrimitiveType(TokenType::INT), true);
    std::vector<Type*> args = {intMutRefArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isOk());
    EXPECT_TRUE(result.unwrap());
}

TEST_F(TypeCheckerFunctionCallTest, MutableReferenceToImmutableReferenceMismatch) {
    // Test: fn(&int) -> void called with &mut int (should fail if strict)
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createReferenceType(createPrimitiveType(TokenType::INT), false));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto intMutRefArg = createReferenceType(createPrimitiveType(TokenType::INT), true);
    std::vector<Type*> args = {intMutRefArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    // Depending on type system rules, this might be valid or invalid
    // We expect it to be valid (mutable ref can be used as immutable ref)
    EXPECT_TRUE(result.isOk());
}

TEST_F(TypeCheckerFunctionCallTest, ImmutableReferenceToMutableReferenceMismatch) {
    // Test: fn(&mut int) -> void called with &int (should fail)
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createReferenceType(createPrimitiveType(TokenType::INT), true));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto intRefArg = createReferenceType(createPrimitiveType(TokenType::INT), false);
    std::vector<Type*> args = {intRefArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isErr());
    EXPECT_TRUE(result.error().getMessage().find("Type mismatch") != std::string::npos);
}

TEST_F(TypeCheckerFunctionCallTest, ReferenceTypeMismatchedPointeeType) {
    // Test: fn(&int) -> void called with &float
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createReferenceType(createPrimitiveType(TokenType::INT), false));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto floatRefArg = createReferenceType(createPrimitiveType(TokenType::FLOAT), false);
    std::vector<Type*> args = {floatRefArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isErr());
    EXPECT_TRUE(result.error().getMessage().find("Type mismatch") != std::string::npos);
}

// =============================================================================
// POINTER TYPE TESTS
// =============================================================================

TEST_F(TypeCheckerFunctionCallTest, ValidPointerParameter) {
    // Test: fn(*int) -> void called with *int
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createPointerType(createPrimitiveType(TokenType::INT)));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto intPtrArg = createPointerType(createPrimitiveType(TokenType::INT));
    std::vector<Type*> args = {intPtrArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isOk());
    EXPECT_TRUE(result.unwrap());
}

TEST_F(TypeCheckerFunctionCallTest, PointerTypeMismatchedPointeeType) {
    // Test: fn(*int) -> void called with *float
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createPointerType(createPrimitiveType(TokenType::INT)));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto floatPtrArg = createPointerType(createPrimitiveType(TokenType::FLOAT));
    std::vector<Type*> args = {floatPtrArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isErr());
    EXPECT_TRUE(result.error().getMessage().find("Type mismatch") != std::string::npos);
}

TEST_F(TypeCheckerFunctionCallTest, PointerVsReferenceTypeMismatch) {
    // Test: fn(*int) -> void called with &int
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createPointerType(createPrimitiveType(TokenType::INT)));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto intRefArg = createReferenceType(createPrimitiveType(TokenType::INT), false);
    std::vector<Type*> args = {intRefArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isErr());
    EXPECT_TRUE(result.error().getMessage().find("Type mismatch") != std::string::npos);
}

// =============================================================================
// SMART POINTER TYPE TESTS
// =============================================================================

TEST_F(TypeCheckerFunctionCallTest, ValidUniquePointerParameter) {
    // Test: fn(unique<int>) -> void called with unique<int>
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createSmartPointerType(createPrimitiveType(TokenType::INT), PointerKind::Unique));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto uniqueIntArg = createSmartPointerType(createPrimitiveType(TokenType::INT), PointerKind::Unique);
    std::vector<Type*> args = {uniqueIntArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isOk());
    EXPECT_TRUE(result.unwrap());
}

TEST_F(TypeCheckerFunctionCallTest, ValidSharedPointerParameter) {
    // Test: fn(shared<int>) -> void called with shared<int>
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createSmartPointerType(createPrimitiveType(TokenType::INT), PointerKind::Shared));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto sharedIntArg = createSmartPointerType(createPrimitiveType(TokenType::INT), PointerKind::Shared);
    std::vector<Type*> args = {sharedIntArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isOk());
    EXPECT_TRUE(result.unwrap());
}

TEST_F(TypeCheckerFunctionCallTest, SmartPointerKindMismatch) {
    // Test: fn(unique<int>) -> void called with shared<int>
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createSmartPointerType(createPrimitiveType(TokenType::INT), PointerKind::Unique));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto sharedIntArg = createSmartPointerType(createPrimitiveType(TokenType::INT), PointerKind::Shared);
    std::vector<Type*> args = {sharedIntArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isErr());
    EXPECT_TRUE(result.error().getMessage().find("Type mismatch") != std::string::npos);
}

TEST_F(TypeCheckerFunctionCallTest, SmartPointerElementTypeMismatch) {
    // Test: fn(unique<int>) -> void called with unique<float>
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createSmartPointerType(createPrimitiveType(TokenType::INT), PointerKind::Unique));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto uniqueFloatArg = createSmartPointerType(createPrimitiveType(TokenType::FLOAT), PointerKind::Unique);
    std::vector<Type*> args = {uniqueFloatArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isErr());
    EXPECT_TRUE(result.error().getMessage().find("Type mismatch") != std::string::npos);
}

// =============================================================================
// VARIADIC FUNCTION TESTS
// =============================================================================

TEST_F(TypeCheckerFunctionCallTest, ValidVariadicFunctionCall_ExactParameterCount) {
    // Test: fn(int, ...) -> void called with (int)
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createPrimitiveType(TokenType::INT));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes), true);

    auto intArg = createPrimitiveType(TokenType::INT);
    std::vector<Type*> args = {intArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isOk());
    EXPECT_TRUE(result.unwrap());
}

TEST_F(TypeCheckerFunctionCallTest, ValidVariadicFunctionCall_ExtraArguments) {
    // Test: fn(int, ...) -> void called with (int, float, bool)
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createPrimitiveType(TokenType::INT));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes), true);

    auto intArg = createPrimitiveType(TokenType::INT);
    auto floatArg = createPrimitiveType(TokenType::FLOAT);
    auto boolArg = createPrimitiveType(TokenType::BOOL);
    std::vector<Type*> args = {intArg.get(), floatArg.get(), boolArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isOk());
    EXPECT_TRUE(result.unwrap());
}

TEST_F(TypeCheckerFunctionCallTest, InvalidVariadicFunctionCall_TooFewArguments) {
    // Test: fn(int, float, ...) -> void called with (int)
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createPrimitiveType(TokenType::INT));
    paramTypes.push_back(createPrimitiveType(TokenType::FLOAT));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes), true);

    auto intArg = createPrimitiveType(TokenType::INT);
    std::vector<Type*> args = {intArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isErr());
    EXPECT_TRUE(result.error().getMessage().find("Argument count mismatch") != std::string::npos);
}

TEST_F(TypeCheckerFunctionCallTest, ValidVariadicFunctionCall_NoRequiredParameters) {
    // Test: fn(...) -> void called with various arguments
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes), true);

    auto intArg = createPrimitiveType(TokenType::INT);
    auto floatArg = createPrimitiveType(TokenType::FLOAT);
    std::vector<Type*> args = {intArg.get(), floatArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isOk());
    EXPECT_TRUE(result.unwrap());
}

TEST_F(TypeCheckerFunctionCallTest, ValidVariadicFunctionCall_NoArguments) {
    // Test: fn(...) -> void called with no arguments
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes), true);

    std::vector<Type*> args;

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isOk());
    EXPECT_TRUE(result.unwrap());
}

// =============================================================================
// NON-FUNCTION TYPE VALIDATION TESTS
// =============================================================================

TEST_F(TypeCheckerFunctionCallTest, InvalidFunctionCall_PrimitiveType) {
    // Test: Calling validateFunctionCall on a primitive type (should fail)
    auto intType = createPrimitiveType(TokenType::INT);
    std::vector<Type*> args;

    auto result = typeChecker->validateFunctionCall(*intType, args, location);

    EXPECT_TRUE(result.isErr());
    EXPECT_TRUE(result.error().getMessage().find("Attempted to call a non-function type") != std::string::npos);
    EXPECT_TRUE(result.error().getMessage().find("int") != std::string::npos);
}

TEST_F(TypeCheckerFunctionCallTest, InvalidFunctionCall_ReferenceType) {
    // Test: Calling validateFunctionCall on a reference type (should fail)
    auto refType = createReferenceType(createPrimitiveType(TokenType::INT), false);
    std::vector<Type*> args;

    auto result = typeChecker->validateFunctionCall(*refType, args, location);

    EXPECT_TRUE(result.isErr());
    EXPECT_TRUE(result.error().getMessage().find("Attempted to call a non-function type") != std::string::npos);
}

TEST_F(TypeCheckerFunctionCallTest, InvalidFunctionCall_PointerType) {
    // Test: Calling validateFunctionCall on a pointer type (should fail)
    auto ptrType = createPointerType(createPrimitiveType(TokenType::INT));
    std::vector<Type*> args;

    auto result = typeChecker->validateFunctionCall(*ptrType, args, location);

    EXPECT_TRUE(result.isErr());
    EXPECT_TRUE(result.error().getMessage().find("Attempted to call a non-function type") != std::string::npos);
}

// =============================================================================
// COMPLEX NESTED TYPE TESTS
// =============================================================================

TEST_F(TypeCheckerFunctionCallTest, NestedReferenceTypes) {
    // Test: fn(&(&int)) -> void (reference to reference)
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    auto innerRef = createReferenceType(createPrimitiveType(TokenType::INT), false);
    paramTypes.push_back(createReferenceType(std::move(innerRef), false));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto argInnerRef = createReferenceType(createPrimitiveType(TokenType::INT), false);
    auto arg = createReferenceType(std::move(argInnerRef), false);
    std::vector<Type*> args = {arg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isOk());
    EXPECT_TRUE(result.unwrap());
}

TEST_F(TypeCheckerFunctionCallTest, PointerToSmartPointer) {
    // Test: fn(*(unique<int>)) -> void
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    auto smartPtr = createSmartPointerType(createPrimitiveType(TokenType::INT), PointerKind::Unique);
    paramTypes.push_back(createPointerType(std::move(smartPtr)));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto argSmartPtr = createSmartPointerType(createPrimitiveType(TokenType::INT), PointerKind::Unique);
    auto arg = createPointerType(std::move(argSmartPtr));
    std::vector<Type*> args = {arg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isOk());
    EXPECT_TRUE(result.unwrap());
}

TEST_F(TypeCheckerFunctionCallTest, SmartPointerToReference) {
    // Test: fn(unique<&int>) -> void
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    auto ref = createReferenceType(createPrimitiveType(TokenType::INT), false);
    paramTypes.push_back(createSmartPointerType(std::move(ref), PointerKind::Unique));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto argRef = createReferenceType(createPrimitiveType(TokenType::INT), false);
    auto arg = createSmartPointerType(std::move(argRef), PointerKind::Unique);
    std::vector<Type*> args = {arg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isOk());
    EXPECT_TRUE(result.unwrap());
}

// =============================================================================
// EDGE CASES AND BOUNDARY CONDITIONS
// =============================================================================

TEST_F(TypeCheckerFunctionCallTest, VoidReturnType) {
    // Test: Functions returning void should still validate parameters correctly
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createPrimitiveType(TokenType::INT));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto intArg = createPrimitiveType(TokenType::INT);
    std::vector<Type*> args = {intArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isOk());
    EXPECT_TRUE(result.unwrap());
}

TEST_F(TypeCheckerFunctionCallTest, FunctionWithManyParameters) {
    // Test: Function with 10 parameters to test parameter iteration
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    std::vector<std::unique_ptr<Type>> argTypes;
    std::vector<Type*> args;

    for (int i = 0; i < 10; ++i) {
        paramTypes.push_back(createPrimitiveType(TokenType::INT));
        argTypes.push_back(createPrimitiveType(TokenType::INT));
        args.push_back(argTypes.back().get());
    }

    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));
    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isOk());
    EXPECT_TRUE(result.unwrap());
}

TEST_F(TypeCheckerFunctionCallTest, FunctionWithManyParameters_OneMismatch) {
    // Test: Function with 10 parameters where the 5th parameter is wrong type
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    std::vector<std::unique_ptr<Type>> argTypes;
    std::vector<Type*> args;

    for (int i = 0; i < 10; ++i) {
        paramTypes.push_back(createPrimitiveType(TokenType::INT));
        if (i == 4) {
            // 5th argument (index 4) is float instead of int
            argTypes.push_back(createPrimitiveType(TokenType::FLOAT));
        } else {
            argTypes.push_back(createPrimitiveType(TokenType::INT));
        }
        args.push_back(argTypes.back().get());
    }

    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));
    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isErr());
    EXPECT_TRUE(result.error().getMessage().find("Type mismatch for argument 5") != std::string::npos);
}

TEST_F(TypeCheckerFunctionCallTest, ZeroSizedTypes) {
    // Test: Function with void parameter (should this be allowed?)
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createPrimitiveType(TokenType::VOID));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    auto voidArg = createPrimitiveType(TokenType::VOID);
    std::vector<Type*> args = {voidArg.get()};

    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    // This might be valid or invalid depending on type system design
    // For now, we expect it to be valid
    EXPECT_TRUE(result.isOk());
}

// =============================================================================
// STRESS TESTS
// =============================================================================

TEST_F(TypeCheckerFunctionCallTest, ExtremelyLargeFunctionSignature) {
    // Test: Function with 100 parameters
    auto returnType = createPrimitiveType(TokenType::INT);
    std::vector<std::unique_ptr<Type>> paramTypes;
    std::vector<std::unique_ptr<Type>> argTypes;
    std::vector<Type*> args;

    for (int i = 0; i < 100; ++i) {
        paramTypes.push_back(createPrimitiveType(TokenType::INT));
        argTypes.push_back(createPrimitiveType(TokenType::INT));
        args.push_back(argTypes.back().get());
    }

    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));
    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isOk());
    EXPECT_TRUE(result.unwrap());
}

TEST_F(TypeCheckerFunctionCallTest, DeeplyNestedTypeStructure) {
    // Test: unique<shared<weak<*(&mut int)>>>
    auto returnType = createPrimitiveType(TokenType::VOID);
    std::vector<std::unique_ptr<Type>> paramTypes;

    auto innerInt = createPrimitiveType(TokenType::INT);
    auto mutRef = createReferenceType(std::move(innerInt), true);
    auto ptr = createPointerType(std::move(mutRef));
    auto weak = createSmartPointerType(std::move(ptr), PointerKind::Weak);
    auto shared = createSmartPointerType(std::move(weak), PointerKind::Shared);
    auto unique = createSmartPointerType(std::move(shared), PointerKind::Unique);
    paramTypes.push_back(std::move(unique));

    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    // Create matching argument
    auto argInnerInt = createPrimitiveType(TokenType::INT);
    auto argMutRef = createReferenceType(std::move(argInnerInt), true);
    auto argPtr = createPointerType(std::move(argMutRef));
    auto argWeak = createSmartPointerType(std::move(argPtr), PointerKind::Weak);
    auto argShared = createSmartPointerType(std::move(argWeak), PointerKind::Shared);
    auto argUnique = createSmartPointerType(std::move(argShared), PointerKind::Unique);

    std::vector<Type*> args = {argUnique.get()};
    auto result = typeChecker->validateFunctionCall(*functionType, args, location);

    EXPECT_TRUE(result.isOk());
    EXPECT_TRUE(result.unwrap());
}

// =============================================================================
// TYPE COMPATIBILITY TESTS (areTypesCompatible method)
// =============================================================================

class TypeCheckerCompatibilityTest : public ::testing::Test {
protected:
    void SetUp() override {
        errorReporter = std::make_unique<ErrorReporter>();
        typeChecker = std::make_unique<TypeChecker>(*errorReporter);
    }

    void TearDown() override {
        errorReporter.reset();
        typeChecker.reset();
    }

    std::unique_ptr<Type> createPrimitiveType(TokenType tokenType) {
        return std::make_unique<PrimitiveType>(tokenType);
    }

    std::unique_ptr<ReferenceType> createReferenceType(std::unique_ptr<Type> pointedType, bool isMutable) {
        return std::make_unique<ReferenceType>(std::move(pointedType), isMutable);
    }

    std::unique_ptr<SmartPointerType> createSmartPointerType(std::unique_ptr<Type> elementType, PointerKind kind) {
        return std::make_unique<SmartPointerType>(std::move(elementType), kind);
    }

    std::unique_ptr<PointerType> createPointerType(std::unique_ptr<Type> pointeeType) {
        return std::make_unique<PointerType>(std::move(pointeeType));
    }

    std::unique_ptr<ErrorReporter> errorReporter;
    std::unique_ptr<TypeChecker> typeChecker;
};

TEST_F(TypeCheckerCompatibilityTest, IdenticalPrimitiveTypes) {
    auto intType1 = createPrimitiveType(TokenType::INT);
    auto intType2 = createPrimitiveType(TokenType::INT);

    auto result = typeChecker->areTypesCompatible(*intType1, *intType2);

    EXPECT_TRUE(result.isOk());
    EXPECT_TRUE(result.unwrap());
}

TEST_F(TypeCheckerCompatibilityTest, DifferentPrimitiveTypes) {
    auto intType = createPrimitiveType(TokenType::INT);
    auto floatType = createPrimitiveType(TokenType::FLOAT);

    auto result = typeChecker->areTypesCompatible(*intType, *floatType);

    // Depending on type system rules, this might be true (implicit conversion) or false
    EXPECT_TRUE(result.isOk());
    // The actual compatibility depends on your type system implementation
}

TEST_F(TypeCheckerCompatibilityTest, AllPrimitiveTypeCombinations) {
    std::vector<TokenType> primitiveTypes = {
        TokenType::INT, TokenType::FLOAT, TokenType::DOUBLE,
        TokenType::BOOL, TokenType::STRING, TokenType::VOID
    };

    for (auto type1 : primitiveTypes) {
        for (auto type2 : primitiveTypes) {
            auto declared = createPrimitiveType(type1);
            auto actual = createPrimitiveType(type2);

            auto result = typeChecker->areTypesCompatible(*declared, *actual);
            EXPECT_TRUE(result.isOk()) << "Failed for types: " << static_cast<int>(type1) << " and " << static_cast<int>(type2);

            if (type1 == type2) {
                EXPECT_TRUE(result.unwrap()) << "Same types should be compatible";
            }
        }
    }
}

TEST_F(TypeCheckerCompatibilityTest, ReferenceTypeCompatibility) {
    auto intType1 = createPrimitiveType(TokenType::INT);
    auto intType2 = createPrimitiveType(TokenType::INT);

    auto immutableRef1 = createReferenceType(std::move(intType1), false);
    auto immutableRef2 = createReferenceType(std::move(intType2), false);

    auto result = typeChecker->areTypesCompatible(*immutableRef1, *immutableRef2);

    EXPECT_TRUE(result.isOk());
    EXPECT_TRUE(result.unwrap());
}

TEST_F(TypeCheckerCompatibilityTest, MutableVsImmutableReferenceCompatibility) {
    auto intType1 = createPrimitiveType(TokenType::INT);
    auto intType2 = createPrimitiveType(TokenType::INT);

    auto immutableRef = createReferenceType(std::move(intType1), false);
    auto mutableRef = createReferenceType(std::move(intType2), true);

    auto result1 = typeChecker->areTypesCompatible(*immutableRef, *mutableRef);
    auto result2 = typeChecker->areTypesCompatible(*mutableRef, *immutableRef);

    EXPECT_TRUE(result1.isOk());
    EXPECT_TRUE(result2.isOk());

    // Mutable reference should be assignable to immutable reference context
    EXPECT_TRUE(result1.unwrap());
    // Immutable reference should NOT be assignable to mutable reference context
    EXPECT_FALSE(result2.unwrap());
}

TEST_F(TypeCheckerCompatibilityTest, SmartPointerCompatibility) {
    auto intType1 = createPrimitiveType(TokenType::INT);
    auto intType2 = createPrimitiveType(TokenType::INT);

    auto unique1 = createSmartPointerType(std::move(intType1), PointerKind::Unique);
    auto unique2 = createSmartPointerType(std::move(intType2), PointerKind::Unique);

    auto result = typeChecker->areTypesCompatible(*unique1, *unique2);

    EXPECT_TRUE(result.isOk());
    EXPECT_TRUE(result.unwrap());
}

TEST_F(TypeCheckerCompatibilityTest, DifferentSmartPointerKinds) {
    auto intType1 = createPrimitiveType(TokenType::INT);
    auto intType2 = createPrimitiveType(TokenType::INT);

    auto unique = createSmartPointerType(std::move(intType1), PointerKind::Unique);
    auto shared = createSmartPointerType(std::move(intType2), PointerKind::Shared);

    auto result = typeChecker->areTypesCompatible(*unique, *shared);

    EXPECT_TRUE(result.isOk());
    EXPECT_FALSE(result.unwrap()); // Different smart pointer kinds should not be compatible
}

// =============================================================================
// COMMON TYPE FINDING TESTS (findCommonType method)
// =============================================================================

class TypeCheckerCommonTypeTest : public ::testing::Test {
protected:
    void SetUp() override {
        errorReporter = std::make_unique<ErrorReporter>();
        typeChecker = std::make_unique<TypeChecker>(*errorReporter);
    }

    void TearDown() override {
        errorReporter.reset();
        typeChecker.reset();
    }

    std::unique_ptr<Type> createPrimitiveType(TokenType tokenType) {
        return std::make_unique<PrimitiveType>(tokenType);
    }

    std::unique_ptr<ErrorReporter> errorReporter;
    std::unique_ptr<TypeChecker> typeChecker;
};

TEST_F(TypeCheckerCommonTypeTest, IdenticalTypesCommonType) {
    auto intType1 = createPrimitiveType(TokenType::INT);
    auto intType2 = createPrimitiveType(TokenType::INT);

    auto result = typeChecker->findCommonType(*intType1, *intType2);

    EXPECT_TRUE(result.isOk());
    auto commonType = std::move(result.unwrap());
    EXPECT_NE(commonType.get(), nullptr);
    EXPECT_EQ(commonType->getKind(), Kind::Primitive);

    auto primitiveCommon = static_cast<PrimitiveType*>(commonType.get());
    EXPECT_EQ(primitiveCommon->getPrimitiveKind(), TokenType::INT);
}

TEST_F(TypeCheckerCommonTypeTest, NumericTypePromotion) {
    auto intType = createPrimitiveType(TokenType::INT);
    auto floatType = createPrimitiveType(TokenType::FLOAT);

    auto result = typeChecker->findCommonType(*intType, *floatType);

    EXPECT_TRUE(result.isOk());
    auto commonType = std::move(result.unwrap());
    EXPECT_NE(commonType.get(), nullptr);

    // Should promote to float (wider type)
    auto primitiveCommon = static_cast<PrimitiveType*>(commonType.get());
    EXPECT_EQ(primitiveCommon->getPrimitiveKind(), TokenType::FLOAT);
}

TEST_F(TypeCheckerCommonTypeTest, DoubleFloatPromotion) {
    auto floatType = createPrimitiveType(TokenType::FLOAT);
    auto doubleType = createPrimitiveType(TokenType::DOUBLE);

    auto result = typeChecker->findCommonType(*floatType, *doubleType);

    EXPECT_TRUE(result.isOk());
    auto commonType = std::move(result.unwrap());
    EXPECT_NE(commonType.get(), nullptr);

    // Should promote to double (wider type)
    auto primitiveCommon = static_cast<PrimitiveType*>(commonType.get());
    EXPECT_EQ(primitiveCommon->getPrimitiveKind(), TokenType::DOUBLE);
}

TEST_F(TypeCheckerCommonTypeTest, IncompatibleTypesNoCommonType) {
    auto intType = createPrimitiveType(TokenType::INT);
    auto stringType = createPrimitiveType(TokenType::STRING);

    auto result = typeChecker->findCommonType(*intType, *stringType);

    // Should either fail or return no common type
    EXPECT_TRUE(result.isErr() || result.unwrap() == nullptr);
}

TEST_F(TypeCheckerCommonTypeTest, BooleanWithOtherTypes) {
    auto boolType = createPrimitiveType(TokenType::BOOL);
    auto intType = createPrimitiveType(TokenType::INT);

    auto result = typeChecker->findCommonType(*boolType, *intType);

    // Depending on type system, bool might promote to int or be incompatible
    EXPECT_TRUE(result.isOk() || result.isErr());
    if (result.isOk() && result.unwrap() != nullptr) {
        // If promotion is allowed, should promote to int
        auto primitiveCommon = static_cast<PrimitiveType*>(result.unwrap().get());
        EXPECT_EQ(primitiveCommon->getPrimitiveKind(), TokenType::INT);
    }
}

// =============================================================================
// ASSIGNMENT VALIDATION TESTS (validateAssignment method)
// =============================================================================

class TypeCheckerAssignmentTest : public ::testing::Test {
protected:
    void SetUp() override {
        errorReporter = std::make_unique<ErrorReporter>();
        typeChecker = std::make_unique<TypeChecker>(*errorReporter);
        location = SourceLocation{"test.forge", 1, 1, 1};
    }

    void TearDown() override {
        errorReporter.reset();
        typeChecker.reset();
    }

    std::unique_ptr<Type> createPrimitiveType(TokenType tokenType) {
        return std::make_unique<PrimitiveType>(tokenType);
    }

    std::unique_ptr<ReferenceType> createReferenceType(std::unique_ptr<Type> pointedType, bool isMutable) {
        return std::make_unique<ReferenceType>(std::move(pointedType), isMutable);
    }

    std::unique_ptr<ErrorReporter> errorReporter;
    std::unique_ptr<TypeChecker> typeChecker;
    SourceLocation location;
};

TEST_F(TypeCheckerAssignmentTest, ValidAssignmentSameType) {
    auto targetType = createPrimitiveType(TokenType::INT);
    auto sourceType = createPrimitiveType(TokenType::INT);

    auto result = typeChecker->validateAssignment(*targetType, *sourceType, location);

    EXPECT_TRUE(result.isOk());
}

TEST_F(TypeCheckerAssignmentTest, ValidAssignmentWithPromotion) {
    auto targetType = createPrimitiveType(TokenType::FLOAT);
    auto sourceType = createPrimitiveType(TokenType::INT);

    auto result = typeChecker->validateAssignment(*targetType, *sourceType, location);

    // int should be assignable to float (promotion)
    EXPECT_TRUE(result.isOk());
}

TEST_F(TypeCheckerAssignmentTest, InvalidAssignmentIncompatibleTypes) {
    auto targetType = createPrimitiveType(TokenType::INT);
    auto sourceType = createPrimitiveType(TokenType::STRING);

    auto result = typeChecker->validateAssignment(*targetType, *sourceType, location);

    EXPECT_TRUE(result.isErr());
}

TEST_F(TypeCheckerAssignmentTest, InvalidAssignmentNarrowingConversion) {
    auto targetType = createPrimitiveType(TokenType::INT);
    auto sourceType = createPrimitiveType(TokenType::FLOAT);

    auto result = typeChecker->validateAssignment(*targetType, *sourceType, location);

    // float to int should require explicit cast (narrowing conversion)
    EXPECT_TRUE(result.isErr());
}

TEST_F(TypeCheckerAssignmentTest, ReferenceAssignmentCompatibility) {
    auto targetIntType = createPrimitiveType(TokenType::INT);
    auto sourceIntType = createPrimitiveType(TokenType::INT);

    auto targetRef = createReferenceType(std::move(targetIntType), false);
    auto sourceRef = createReferenceType(std::move(sourceIntType), false);

    auto result = typeChecker->validateAssignment(*targetRef, *sourceRef, location);

    EXPECT_TRUE(result.isOk());
}

TEST_F(TypeCheckerAssignmentTest, MutableToImmutableReferenceAssignment) {
    auto targetIntType = createPrimitiveType(TokenType::INT);
    auto sourceIntType = createPrimitiveType(TokenType::INT);

    auto targetRef = createReferenceType(std::move(targetIntType), false); // immutable
    auto sourceRef = createReferenceType(std::move(sourceIntType), true);  // mutable

    auto result = typeChecker->validateAssignment(*targetRef, *sourceRef, location);

    // Mutable reference should be assignable to immutable reference
    EXPECT_TRUE(result.isOk());
}

TEST_F(TypeCheckerAssignmentTest, ImmutableToMutableReferenceAssignment) {
    auto targetIntType = createPrimitiveType(TokenType::INT);
    auto sourceIntType = createPrimitiveType(TokenType::INT);

    auto targetRef = createReferenceType(std::move(targetIntType), true);  // mutable
    auto sourceRef = createReferenceType(std::move(sourceIntType), false); // immutable

    auto result = typeChecker->validateAssignment(*targetRef, *sourceRef, location);

    // Immutable reference should NOT be assignable to mutable reference
    EXPECT_TRUE(result.isErr());
}

// =============================================================================
// COMPREHENSIVE INTEGRATION TESTS
// =============================================================================

class TypeCheckerIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        errorReporter = std::make_unique<ErrorReporter>();
        typeChecker = std::make_unique<TypeChecker>(*errorReporter);
        location = SourceLocation{"test.forge", 1, 1, 1};
    }

    void TearDown() override {
        errorReporter.reset();
        typeChecker.reset();
    }

    std::unique_ptr<Type> createPrimitiveType(TokenType tokenType) {
        return std::make_unique<PrimitiveType>(tokenType);
    }

    std::unique_ptr<FunctionType> createFunctionType(
        std::unique_ptr<Type> returnType,
        std::vector<std::unique_ptr<Type>> paramTypes,
        bool isVariadic = false
    ) {
        return std::make_unique<FunctionType>(
            std::move(returnType),
            std::move(paramTypes),
            isVariadic
        );
    }

    std::unique_ptr<ReferenceType> createReferenceType(std::unique_ptr<Type> pointedType, bool isMutable) {
        return std::make_unique<ReferenceType>(std::move(pointedType), isMutable);
    }

    std::unique_ptr<SmartPointerType> createSmartPointerType(std::unique_ptr<Type> elementType, PointerKind kind) {
        return std::make_unique<SmartPointerType>(std::move(elementType), kind);
    }

    std::unique_ptr<ErrorReporter> errorReporter;
    std::unique_ptr<TypeChecker> typeChecker;
    SourceLocation location;
};

TEST_F(TypeCheckerIntegrationTest, ComplexFunctionCallWithTypePromotion) {
    // Test function that takes (float, &int) -> bool with (int, &int) arguments
    auto returnType = createPrimitiveType(TokenType::BOOL);
    std::vector<std::unique_ptr<Type>> paramTypes;
    paramTypes.push_back(createPrimitiveType(TokenType::FLOAT));
    paramTypes.push_back(createReferenceType(createPrimitiveType(TokenType::INT), false));
    auto functionType = createFunctionType(std::move(returnType), std::move(paramTypes));

    // Check if types are compatible first
    auto intType = createPrimitiveType(TokenType::INT);
    auto floatType = createPrimitiveType(TokenType::FLOAT);
    auto compatResult = typeChecker->areTypesCompatible(*floatType, *intType);

    EXPECT_TRUE(compatResult.isOk());

    if (compatResult.unwrap()) {
        // If int is compatible with float, function call should succeed
        auto intArg = createPrimitiveType(TokenType::INT);
        auto intRefArg = createReferenceType(createPrimitiveType(TokenType::INT), false);
        std::vector<Type*> args = {intArg.get(), intRefArg.get()};

        auto result = typeChecker->validateFunctionCall(*functionType, args, location);
        EXPECT_TRUE(result.isOk());
        EXPECT_TRUE(result.unwrap());
    }
}

TEST_F(TypeCheckerIntegrationTest, TypeCompatibilityChaining) {
    // Test chaining compatibility checks
    auto intType = createPrimitiveType(TokenType::INT);
    auto floatType = createPrimitiveType(TokenType::FLOAT);
    auto doubleType = createPrimitiveType(TokenType::DOUBLE);

    // int -> float compatibility
    auto result1 = typeChecker->areTypesCompatible(*floatType, *intType);
    EXPECT_TRUE(result1.isOk());

    // float -> double compatibility
    auto result2 = typeChecker->areTypesCompatible(*doubleType, *floatType);
    EXPECT_TRUE(result2.isOk());

    // Find common type int, double
    auto commonResult = typeChecker->findCommonType(*intType, *doubleType);
    EXPECT_TRUE(commonResult.isOk());

    if (commonResult.unwrap() != nullptr) {
        auto primitiveCommon = static_cast<PrimitiveType*>(commonResult.unwrap().get());
        EXPECT_EQ(primitiveCommon->getPrimitiveKind(), TokenType::DOUBLE);
    }
}

TEST_F(TypeCheckerIntegrationTest, ErrorReportingConsistency) {
    // Test that errors are reported consistently
    // Trigger an error with incompatible assignment
    auto intType = createPrimitiveType(TokenType::INT);
    auto stringType = createPrimitiveType(TokenType::STRING);

    auto result = typeChecker->validateAssignment(*intType, *stringType, location);
    EXPECT_TRUE(result.isErr());

    // Check that error was reported (if the implementation reports to ErrorReporter)
    // Note: This depends on whether your TypeChecker reports errors to the ErrorReporter
    // or just returns error results
}

TEST_F(TypeCheckerIntegrationTest, MemoryManagementStressTest) {
    // Test creating and destroying many type objects
    std::vector<std::unique_ptr<Type>> types;

    for (int i = 0; i < 1000; ++i) {
        types.push_back(createPrimitiveType(TokenType::INT));
        types.push_back(createPrimitiveType(TokenType::FLOAT));
        types.push_back(createReferenceType(createPrimitiveType(TokenType::BOOL), false));
        types.push_back(createSmartPointerType(createPrimitiveType(TokenType::DOUBLE), PointerKind::Unique));
    }

    // Test type compatibility on random pairs
    for (int i = 0; i < 100; ++i) {
        size_t idx1 = i * 40 % types.size();
        size_t idx2 = (i * 40 + 1) % types.size();

        auto result = typeChecker->areTypesCompatible(*types[idx1], *types[idx2]);
        EXPECT_TRUE(result.isOk()); // Should not crash or fail to compute
    }
}