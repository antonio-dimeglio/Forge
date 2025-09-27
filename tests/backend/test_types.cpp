#include <gtest/gtest.h>
#include <memory>
#include <chrono>

// Test individual components that are public
#include "../../include/backend/types/PrimitiveType.hpp"
#include "../../include/backend/types/ReferenceType.hpp"
#include "../../include/backend/types/SmartPointerType.hpp"

using namespace forge::types;
using forge::types::Kind;
using SmartPointerType = forge::types::SmartPointerType;

// =====================================================
// PRIMITIVE TYPE TESTS
// =====================================================

class PrimitiveTypeTest : public ::testing::Test {};

TEST_F(PrimitiveTypeTest, CreateIntType) {
    auto intType = std::make_unique<PrimitiveType>(TokenType::INT);

    EXPECT_EQ(intType->getKind(), Kind::Primitive);
    EXPECT_EQ(intType->toString(), "int");
    EXPECT_EQ(intType->getSizeBytes(), 4);
    EXPECT_TRUE(intType->isCopyable());
    EXPECT_TRUE(intType->isMovable());
    EXPECT_FALSE(intType->requiresCleanup());
}

TEST_F(PrimitiveTypeTest, CreateFloatType) {
    auto floatType = std::make_unique<PrimitiveType>(TokenType::FLOAT);

    EXPECT_EQ(floatType->getKind(), Kind::Primitive);
    EXPECT_EQ(floatType->getPrimitiveKind(), TokenType::FLOAT);
    EXPECT_EQ(floatType->toString(), "float");
    EXPECT_EQ(floatType->getSizeBytes(), 4);
}

TEST_F(PrimitiveTypeTest, CreateDoubleType) {
    auto doubleType = std::make_unique<PrimitiveType>(TokenType::DOUBLE);

    EXPECT_EQ(doubleType->getKind(), Kind::Primitive);
    EXPECT_EQ(doubleType->getPrimitiveKind(), TokenType::DOUBLE);
    EXPECT_EQ(doubleType->toString(), "double");
    EXPECT_EQ(doubleType->getSizeBytes(), 8);
}

TEST_F(PrimitiveTypeTest, CreateBoolType) {
    auto boolType = std::make_unique<PrimitiveType>(TokenType::BOOL);

    EXPECT_EQ(boolType->getKind(), Kind::Primitive);
    EXPECT_EQ(boolType->getPrimitiveKind(), TokenType::BOOL);
    EXPECT_EQ(boolType->toString(), "bool");
    EXPECT_EQ(boolType->getSizeBytes(), 1);
}

TEST_F(PrimitiveTypeTest, CreateStringType) {
    auto stringType = std::make_unique<PrimitiveType>(TokenType::STRING);

    EXPECT_EQ(stringType->getKind(), Kind::Primitive);
    EXPECT_EQ(stringType->getPrimitiveKind(), TokenType::STRING);
    EXPECT_EQ(stringType->toString(), "char");
    EXPECT_EQ(stringType->getSizeBytes(), sizeof(char*));
}

TEST_F(PrimitiveTypeTest, CreateVoidType) {
    auto voidType = std::make_unique<PrimitiveType>(TokenType::VOID);

    EXPECT_EQ(voidType->getKind(), Kind::Primitive);
    EXPECT_EQ(voidType->getPrimitiveKind(), TokenType::VOID);
    EXPECT_EQ(voidType->toString(), "void");
    EXPECT_EQ(voidType->getSizeBytes(), 0);
}

TEST_F(PrimitiveTypeTest, CloneType) {
    auto originalType = std::make_unique<PrimitiveType>(TokenType::INT);
    auto clonedType = originalType->clone();

    ASSERT_NE(clonedType.get(), nullptr);
    EXPECT_EQ(clonedType->getKind(), Kind::Primitive);

    auto clonedPrimitive = static_cast<PrimitiveType*>(clonedType.get());
    EXPECT_EQ(clonedPrimitive->getPrimitiveKind(), TokenType::INT);
    EXPECT_EQ(clonedPrimitive->toString(), "int");
}

// =====================================================
// TYPE COMPATIBILITY TESTS
// =====================================================

class TypeCompatibilityTest : public ::testing::Test {};

TEST_F(TypeCompatibilityTest, SameTypeAssignability) {
    auto intType1 = std::make_unique<PrimitiveType>(TokenType::INT);
    auto intType2 = std::make_unique<PrimitiveType>(TokenType::INT);

    EXPECT_TRUE(intType1->isAssignableFrom(*intType2));
    EXPECT_TRUE(intType2->isAssignableFrom(*intType1));
}

TEST_F(TypeCompatibilityTest, DifferentTypeAssignability) {
    auto intType = std::make_unique<PrimitiveType>(TokenType::INT);
    auto floatType = std::make_unique<PrimitiveType>(TokenType::FLOAT);

    // int should be assignable to float (implicit conversion)
    EXPECT_TRUE(floatType->isAssignableFrom(*intType));

    // but float should NOT be assignable to int (precision loss)
    EXPECT_FALSE(intType->isAssignableFrom(*floatType));
}

TEST_F(TypeCompatibilityTest, ImplicitConversions) {
    auto intType = std::make_unique<PrimitiveType>(TokenType::INT);
    auto floatType = std::make_unique<PrimitiveType>(TokenType::FLOAT);
    auto doubleType = std::make_unique<PrimitiveType>(TokenType::DOUBLE);

    // int -> float should be allowed
    EXPECT_TRUE(intType->canImplicitlyConvertTo(*floatType));

    // int -> double should be allowed
    EXPECT_TRUE(intType->canImplicitlyConvertTo(*doubleType));

    // float -> double should be allowed
    EXPECT_TRUE(floatType->canImplicitlyConvertTo(*doubleType));

    // Reverse conversions should not be allowed
    EXPECT_FALSE(floatType->canImplicitlyConvertTo(*intType));
    EXPECT_FALSE(doubleType->canImplicitlyConvertTo(*intType));
    EXPECT_FALSE(doubleType->canImplicitlyConvertTo(*floatType));
}

TEST_F(TypeCompatibilityTest, TypePromotion) {
    auto intType = std::make_unique<PrimitiveType>(TokenType::INT);
    auto floatType = std::make_unique<PrimitiveType>(TokenType::FLOAT);
    auto doubleType = std::make_unique<PrimitiveType>(TokenType::DOUBLE);

    // int + float -> float
    auto promoted1 = intType->promoteWith(*floatType);
    ASSERT_TRUE(promoted1.has_value());
    auto prim1 = static_cast<PrimitiveType*>(promoted1->get());
    EXPECT_EQ(prim1->getPrimitiveKind(), TokenType::FLOAT);

    // int + double -> double
    auto promoted2 = intType->promoteWith(*doubleType);
    ASSERT_TRUE(promoted2.has_value());
    auto prim2 = static_cast<PrimitiveType*>(promoted2->get());
    EXPECT_EQ(prim2->getPrimitiveKind(), TokenType::DOUBLE);

    // float + double -> double
    auto promoted3 = floatType->promoteWith(*doubleType);
    ASSERT_TRUE(promoted3.has_value());
    auto prim3 = static_cast<PrimitiveType*>(promoted3->get());
    EXPECT_EQ(prim3->getPrimitiveKind(), TokenType::DOUBLE);
}

TEST_F(TypeCompatibilityTest, NoPromotionBetweenIncompatibleTypes) {
    auto intType = std::make_unique<PrimitiveType>(TokenType::INT);
    auto boolType = std::make_unique<PrimitiveType>(TokenType::BOOL);
    auto stringType = std::make_unique<PrimitiveType>(TokenType::STRING);

    // No promotion between int and bool
    auto noPromotion1 = intType->promoteWith(*boolType);
    EXPECT_FALSE(noPromotion1.has_value());

    // No promotion between int and string
    auto noPromotion2 = intType->promoteWith(*stringType);
    EXPECT_FALSE(noPromotion2.has_value());

    // No promotion between bool and string
    auto noPromotion3 = boolType->promoteWith(*stringType);
    EXPECT_FALSE(noPromotion3.has_value());
}

// =====================================================
// REFERENCE TYPE TESTS
// =====================================================

class ReferenceTypeTest : public ::testing::Test {};

TEST_F(ReferenceTypeTest, CreateImmutableReference) {
    auto pointedType = std::make_unique<PrimitiveType>(TokenType::INT);
    auto refType = std::make_unique<ReferenceType>(std::move(pointedType), false);

    EXPECT_EQ(refType->getKind(), Kind::Reference);
    EXPECT_FALSE(refType->isMutable());
    EXPECT_EQ(refType->getPointedType().getKind(), Kind::Primitive);
    EXPECT_EQ(refType->toString(), "& int");
    EXPECT_EQ(refType->getSizeBytes(), sizeof(void*));
    EXPECT_TRUE(refType->isCopyable());
    EXPECT_TRUE(refType->isMovable());
    EXPECT_FALSE(refType->requiresCleanup());
}

TEST_F(ReferenceTypeTest, CreateMutableReference) {
    auto pointedType = std::make_unique<PrimitiveType>(TokenType::FLOAT);
    auto refType = std::make_unique<ReferenceType>(std::move(pointedType), true);

    EXPECT_EQ(refType->getKind(), Kind::Reference);
    EXPECT_TRUE(refType->isMutable());
    EXPECT_EQ(refType->getPointedType().getKind(), Kind::Primitive);
    EXPECT_EQ(refType->toString(), "&mut float");
}

TEST_F(ReferenceTypeTest, ReferenceCompatibility) {
    auto intType1 = std::make_unique<PrimitiveType>(TokenType::INT);
    auto intType2 = std::make_unique<PrimitiveType>(TokenType::INT);

    auto immutableRef = std::make_unique<ReferenceType>(std::move(intType1), false);
    auto mutableRef = std::make_unique<ReferenceType>(std::move(intType2), true);

    // Mutable reference can be assigned to immutable reference
    EXPECT_TRUE(immutableRef->isAssignableFrom(*mutableRef));

    // Immutable reference cannot be assigned to mutable reference
    EXPECT_FALSE(mutableRef->isAssignableFrom(*immutableRef));
}

TEST_F(ReferenceTypeTest, CloneReference) {
    auto pointedType = std::make_unique<PrimitiveType>(TokenType::DOUBLE);
    auto originalRef = std::make_unique<ReferenceType>(std::move(pointedType), true);
    auto clonedRef = originalRef->clone();

    ASSERT_NE(clonedRef.get(), nullptr);
    EXPECT_EQ(clonedRef->getKind(), Kind::Reference);

    auto clonedRefType = static_cast<ReferenceType*>(clonedRef.get());
    EXPECT_TRUE(clonedRefType->isMutable());
    EXPECT_EQ(clonedRefType->getPointedType().getKind(), Kind::Primitive);
}

// =====================================================
// SMART POINTER TYPE TESTS
// =====================================================

class SmartPointerTypeTest : public ::testing::Test {};

TEST_F(SmartPointerTypeTest, CreateUniquePointer) {
    auto elementType = std::make_unique<PrimitiveType>(TokenType::INT);
    auto uniquePtr = std::make_unique<SmartPointerType>(std::move(elementType), PointerKind::Unique);
    
    EXPECT_EQ(uniquePtr->getKind(), Kind::SmartPointer);
    EXPECT_EQ(uniquePtr->getPointerKind(), PointerKind::Unique);
    EXPECT_EQ(uniquePtr->getElementType().getKind(), Kind::Primitive);
    EXPECT_EQ(uniquePtr->toString(), "SmartPointer<int, Unique>");
    EXPECT_EQ(uniquePtr->getSizeBytes(), sizeof(void*));
    EXPECT_TRUE(uniquePtr->requiresCleanup());
    EXPECT_FALSE(uniquePtr->isCopyable()); // Unique pointers are not copyable
    EXPECT_TRUE(uniquePtr->isMovable());
}

TEST_F(SmartPointerTypeTest, CreateSharedPointer) {
    auto elementType = std::make_unique<PrimitiveType>(TokenType::STRING);
    auto sharedPtr = std::make_unique<SmartPointerType>(std::move(elementType), PointerKind::Shared);
    
    EXPECT_EQ(sharedPtr->getKind(), Kind::SmartPointer);
    EXPECT_EQ(sharedPtr->getPointerKind(), PointerKind::Shared);
    EXPECT_EQ(sharedPtr->toString(), "SmartPointer<char, Shared>");
    EXPECT_TRUE(sharedPtr->isCopyable()); // Shared pointers are copyable
    EXPECT_TRUE(sharedPtr->isMovable());
}

TEST_F(SmartPointerTypeTest, CreateWeakPointer) {
    auto elementType = std::make_unique<PrimitiveType>(TokenType::BOOL);
    auto weakPtr = std::make_unique<SmartPointerType>(std::move(elementType), PointerKind::Weak);

    EXPECT_EQ(weakPtr->getKind(), Kind::SmartPointer);
    EXPECT_EQ(weakPtr->getPointerKind(), PointerKind::Weak);
    EXPECT_EQ(weakPtr->toString(), "SmartPointer<bool, Weak>");
    EXPECT_TRUE(weakPtr->isCopyable()); // Weak pointers are copyable
    EXPECT_TRUE(weakPtr->isMovable());
}

TEST_F(SmartPointerTypeTest, SmartPointerCompatibility) {
    auto intType1 = std::make_unique<PrimitiveType>(TokenType::INT);
    auto intType2 = std::make_unique<PrimitiveType>(TokenType::INT);
    auto intType3 = std::make_unique<PrimitiveType>(TokenType::INT);
    auto floatType = std::make_unique<PrimitiveType>(TokenType::FLOAT);

    auto uniqueInt = std::make_unique<SmartPointerType>(std::move(intType1), PointerKind::Unique);
    auto sharedInt = std::make_unique<SmartPointerType>(std::move(intType2), PointerKind::Shared);
    auto weakInt = std::make_unique<SmartPointerType>(std::move(intType3), PointerKind::Weak);
    auto uniqueFloat = std::make_unique<SmartPointerType>(std::move(floatType), PointerKind::Unique);

    // Shared can accept unique (move semantics)
    EXPECT_TRUE(sharedInt->isAssignableFrom(*uniqueInt));

    // Unique cannot accept shared (ownership conflict)
    EXPECT_FALSE(uniqueInt->isAssignableFrom(*sharedInt));

    // Weak can accept shared
    EXPECT_TRUE(weakInt->isAssignableFrom(*sharedInt));

    // Different element types should not be assignable
    EXPECT_FALSE(uniqueInt->isAssignableFrom(*uniqueFloat));
}

TEST_F(SmartPointerTypeTest, SmartPointerPromotion) {
    auto intType1 = std::make_unique<PrimitiveType>(TokenType::INT);
    auto intType2 = std::make_unique<PrimitiveType>(TokenType::INT);
    auto intType3 = std::make_unique<PrimitiveType>(TokenType::INT);
    auto intType4 = std::make_unique<PrimitiveType>(TokenType::INT);

    auto uniqueInt = std::make_unique<SmartPointerType>(std::move(intType1), PointerKind::Unique);
    auto sharedInt = std::make_unique<SmartPointerType>(std::move(intType2), PointerKind::Shared);
    auto weakInt = std::make_unique<SmartPointerType>(std::move(intType3), PointerKind::Weak);

    // Unique + Shared -> Shared
    auto promoted1 = uniqueInt->promoteWith(*sharedInt);
    ASSERT_TRUE(promoted1.has_value());
    auto smartPtr1 = static_cast<SmartPointerType*>(promoted1->get());
    EXPECT_EQ(smartPtr1->getPointerKind(), PointerKind::Shared);

    // Shared + Weak -> Weak
    auto promoted2 = sharedInt->promoteWith(*weakInt);
    ASSERT_TRUE(promoted2.has_value());
    auto smartPtr2 = static_cast<SmartPointerType*>(promoted2->get());
    EXPECT_EQ(smartPtr2->getPointerKind(), PointerKind::Weak);

    // Unique + Weak should not promote (no direct relationship)
    auto intType5 = std::make_unique<PrimitiveType>(TokenType::INT);
    auto uniqueInt2 = std::make_unique<SmartPointerType>(std::move(intType5), PointerKind::Unique);
    auto noPromotion = uniqueInt2->promoteWith(*weakInt);
    EXPECT_FALSE(noPromotion.has_value());
}

TEST_F(SmartPointerTypeTest, CloneSmartPointer) {
    auto elementType = std::make_unique<PrimitiveType>(TokenType::DOUBLE);
    auto originalPtr = std::make_unique<SmartPointerType>(std::move(elementType), PointerKind::Shared);
    auto clonedPtr = originalPtr->clone();

    ASSERT_NE(clonedPtr.get(), nullptr);
    EXPECT_EQ(clonedPtr->getKind(), Kind::SmartPointer);

    auto clonedSmartPtr = static_cast<SmartPointerType*>(clonedPtr.get());
    EXPECT_EQ(clonedSmartPtr->getPointerKind(), PointerKind::Shared);
    EXPECT_EQ(clonedSmartPtr->getElementType().getKind(), Kind::Primitive);
}

// =====================================================
// COMPLEX TYPE NESTING TESTS
// =====================================================

class ComplexTypeTest : public ::testing::Test {};

TEST_F(ComplexTypeTest, ReferenceToSmartPointer) {
    auto elementType = std::make_unique<PrimitiveType>(TokenType::DOUBLE);
    auto smartPtr = std::make_unique<SmartPointerType>(std::move(elementType), PointerKind::Unique);
    auto refToSmartPtr = std::make_unique<ReferenceType>(std::move(smartPtr), false);

    EXPECT_EQ(refToSmartPtr->getKind(), Kind::Reference);
    EXPECT_FALSE(refToSmartPtr->isMutable());
    EXPECT_EQ(refToSmartPtr->getPointedType().getKind(), Kind::SmartPointer);

    auto nestedSmartPtr = static_cast<const SmartPointerType*>(&refToSmartPtr->getPointedType());
    EXPECT_EQ(nestedSmartPtr->getPointerKind(), PointerKind::Unique);
    EXPECT_EQ(nestedSmartPtr->getElementType().getKind(), Kind::Primitive);
}

TEST_F(ComplexTypeTest, SmartPointerToReference) {
    auto pointedType = std::make_unique<PrimitiveType>(TokenType::STRING);
    auto refType = std::make_unique<ReferenceType>(std::move(pointedType), true);
    auto smartPtrToRef = std::make_unique<SmartPointerType>(std::move(refType), PointerKind::Shared);

    EXPECT_EQ(smartPtrToRef->getKind(), Kind::SmartPointer);
    EXPECT_EQ(smartPtrToRef->getPointerKind(), PointerKind::Shared);
    EXPECT_EQ(smartPtrToRef->getElementType().getKind(), Kind::Reference);

    auto nestedRef = static_cast<const ReferenceType*>(&smartPtrToRef->getElementType());
    EXPECT_TRUE(nestedRef->isMutable());
    EXPECT_EQ(nestedRef->getPointedType().getKind(), Kind::Primitive);
}

TEST_F(ComplexTypeTest, DeepTypeNesting) {
    // Create: Shared<Unique<&mut int>>
    auto intType = std::make_unique<PrimitiveType>(TokenType::INT);
    auto mutRef = std::make_unique<ReferenceType>(std::move(intType), true);
    auto uniquePtr = std::make_unique<SmartPointerType>(std::move(mutRef), PointerKind::Unique);
    auto sharedPtr = std::make_unique<SmartPointerType>(std::move(uniquePtr), PointerKind::Shared);

    EXPECT_EQ(sharedPtr->getKind(), Kind::SmartPointer);
    EXPECT_EQ(sharedPtr->getPointerKind(), PointerKind::Shared);
    EXPECT_EQ(sharedPtr->getElementType().getKind(), Kind::SmartPointer);

    auto nestedUnique = static_cast<const SmartPointerType*>(&sharedPtr->getElementType());
    EXPECT_EQ(nestedUnique->getPointerKind(), PointerKind::Unique);
    EXPECT_EQ(nestedUnique->getElementType().getKind(), Kind::Reference);

    auto nestedRef = static_cast<const ReferenceType*>(&nestedUnique->getElementType());
    EXPECT_TRUE(nestedRef->isMutable());
    EXPECT_EQ(nestedRef->getPointedType().getKind(), Kind::Primitive);
}

// =====================================================
// EDGE CASE AND STRESS TESTS
// =====================================================

class EdgeCaseTest : public ::testing::Test {};

TEST_F(EdgeCaseTest, ManyTypeClones) {
    auto originalType = std::make_unique<PrimitiveType>(TokenType::INT);

    // Clone many times to test memory management
    std::vector<std::unique_ptr<Type>> clones;
    for (int i = 0; i < 1000; ++i) {
        clones.push_back(originalType->clone());
    }

    // Verify all clones are correct
    for (const auto& clone : clones) {
        EXPECT_EQ(clone->getKind(), Kind::Primitive);
        auto primClone = static_cast<PrimitiveType*>(clone.get());
        EXPECT_EQ(primClone->getPrimitiveKind(), TokenType::INT);
    }
}

TEST_F(EdgeCaseTest, ComplexTypeCloning) {
    // Create complex nested type and clone it
    auto intType = std::make_unique<PrimitiveType>(TokenType::INT);
    auto refType = std::make_unique<ReferenceType>(std::move(intType), false);
    auto smartPtr = std::make_unique<SmartPointerType>(std::move(refType), PointerKind::Unique);

    auto cloned = smartPtr->clone();

    EXPECT_EQ(cloned->getKind(), Kind::SmartPointer);
    auto clonedSmartPtr = static_cast<SmartPointerType*>(cloned.get());
    EXPECT_EQ(clonedSmartPtr->getPointerKind(), PointerKind::Unique);
    EXPECT_EQ(clonedSmartPtr->getElementType().getKind(), Kind::Reference);

    auto clonedRef = static_cast<const ReferenceType*>(&clonedSmartPtr->getElementType());
    EXPECT_FALSE(clonedRef->isMutable());
    EXPECT_EQ(clonedRef->getPointedType().getKind(), Kind::Primitive);
}

// =====================================================
// PERFORMANCE TESTS
// =====================================================

class PerformanceTest : public ::testing::Test {};

TEST_F(PerformanceTest, TypeCreationPerformance) {
    auto start = std::chrono::high_resolution_clock::now();

    // Create many types
    std::vector<std::unique_ptr<Type>> types;
    for (int i = 0; i < 10000; ++i) {
        types.push_back(std::make_unique<PrimitiveType>(TokenType::INT));
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Should be reasonably fast
    EXPECT_LT(duration.count(), 100); // Less than 100ms

    // Verify all types are correct
    for (const auto& type : types) {
        EXPECT_EQ(type->getKind(), Kind::Primitive);
    }
}

TEST_F(PerformanceTest, TypeCompatibilityPerformance) {
    auto intType = std::make_unique<PrimitiveType>(TokenType::INT);
    auto floatType = std::make_unique<PrimitiveType>(TokenType::FLOAT);

    auto start = std::chrono::high_resolution_clock::now();

    // Perform many compatibility checks
    for (int i = 0; i < 100000; ++i) {
        bool compatible = intType->canImplicitlyConvertTo(*floatType);
        EXPECT_TRUE(compatible); // Should always be true
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Should be very fast
    EXPECT_LT(duration.count(), 50); // Less than 50ms
}