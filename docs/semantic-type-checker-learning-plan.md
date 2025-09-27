# Semantic Type Checker - Learning Implementation Plan

## 🎯 **Learning Objectives**
By the end of this implementation, you'll understand:
- How to design type systems that work with LLVM's opaque pointers
- Dependency injection patterns in compiler design
- How to bridge semantic analysis with code generation
- Testing strategies for type systems
- Incremental development of complex systems

---

## 📋 **Implementation Roadmap**

### **PHASE 1: Foundation (Week 1)** 🟢 *Start Here*
**Goal**: Understand the problem and implement basic type compatibility

#### **Day 1: Understanding the Problem**
**Learning Focus**: What makes types compatible?

**Your Task**: Implement `areSemanticTypesCompatible()` for the simplest cases:
```cpp
bool SemanticTypeChecker::areSemanticTypesCompatible(
    const ParsedType& declared,
    const ParsedType& actual
) {
    // Case 1: Exact match
    // Case 2: Basic type promotion (int -> float)
}
```

**Test Cases to Write**:
```forge
x: int = 42        // int = int ✅
y: float = 42      // float = int ✅ (promotion)
z: int = 3.14      // int = float ❌ (no demotion)
```

**APIs You'll Learn**:
- `ParsedType` structure and fields
- Token type comparisons
- Basic type promotion rules

#### **Day 2: Reference Type Rules**
**Learning Focus**: How references work in type systems

**Your Task**: Implement `areReferencesCompatible()`:
```cpp
// Rules to implement:
// &int = &int ✅
// &mut int = &mut int ✅
// &int = &mut int ❌ (can't assign mutable to immutable)
// &mut int = &int ❌ (can't make immutable mutable)
```

**Test Cases**:
```forge
x: int = 42
ref1: &int = &x        // ✅
ref2: &mut int = &x    // Need to check if this should work
```

#### **Day 3: Basic Integration**
**Learning Focus**: How to connect semantic checking to existing code

**Your Task**: Wire up the simplest integration in `LLVMTypeSystem::areTypesCompatible()`

**Modification Strategy**:
```cpp
// In LLVMTypeSystem.cpp
bool LLVMTypeSystem::areTypesCompatible(llvm::Type* declared, llvm::Value* actualValue) {
    // ... existing code ...

    if (declared->isPointerTy() && actual->isPointerTy()) {
        // NEW: Try semantic checking if we can get the info
        // For now, just try a simple case
        return LLVMTypeSystemEnhanced::areTypesCompatible(declared, actualValue, nullptr, nullptr);
    }

    // ... rest of existing code ...
}
```

---

### **PHASE 2: Value Analysis (Week 2)** 🟡 *Intermediate*
**Goal**: Learn to analyze LLVM IR to infer semantic types

#### **Day 4-5: LLVM IR Detective Work**
**Learning Focus**: How to trace LLVM values back to their source

**Your Task**: Implement `analyzeVariableAccess()`:
```cpp
std::optional<ParsedType> SemanticTypeChecker::analyzeVariableAccess(
    llvm::Value* value,
    ScopeManager& scopeManager
) {
    // 1. Check if value is an AllocaInst (variable storage)
    // 2. Check if value has a name we can look up
    // 3. Use ScopeManager to find the semantic type
}
```

**LLVM APIs You'll Learn**:
- `llvm::dyn_cast<llvm::AllocaInst>()`
- `value->getName()`
- `scopeManager.lookupType()`

#### **Day 6-7: Address-of Analysis**
**Learning Focus**: Understanding reference creation expressions

**Your Task**: Implement `analyzeAddressOfExpression()`:
```cpp
// For expressions like &x, we need to:
// 1. Find what 'x' is
// 2. Create a reference type to x's type
// 3. Handle &mut vs & cases
```

**Test Cases**:
```forge
x: int = 42
ref: &int = &x     // analyzeAddressOfExpression should infer &int from &x
```

---

### **PHASE 3: Advanced Features (Week 3)** 🟠 *Advanced*
**Goal**: Handle complex pointer types and smart pointers

#### **Day 8-9: Pointer Nesting**
**Learning Focus**: Multi-level pointers and references

**Your Task**: Implement `arePointersCompatible()`:
```cpp
// Handle cases like:
// **int vs **int ✅
// **int vs *int ❌
// ***int vs **int ❌
```

#### **Day 10: Smart Pointer Integration**
**Learning Focus**: How smart pointers fit into the type system

**Your Task**: Implement `areSmartPointersCompatible()`:
```cpp
// Handle cases like:
// unique_ptr<int> vs unique_ptr<int> ✅
// unique_ptr<int> vs shared_ptr<int> ❌
// unique_ptr<int> vs unique_ptr<float> ❌
```

---

### **PHASE 4: Integration & Testing (Week 4)** 🔴 *Production Ready*
**Goal**: Make it production-ready with comprehensive testing

#### **Day 11-12: Full Integration**
**Your Task**: Complete the main `areTypesCompatible()` function

#### **Day 13-14: Comprehensive Testing**
**Your Task**: Write stress tests and edge cases

---

## 🎓 **Learning Methodology**

### **For Each Day**:
1. **🧠 Understand** - I'll explain the concept and show examples
2. **✏️ Design** - You sketch the algorithm/approach
3. **⌨️ Implement** - You write the code with my guidance
4. **🧪 Test** - We write tests to verify it works
5. **🔄 Iterate** - We refine based on what we learn

### **Testing Strategy**:
- **Unit tests** for each helper function
- **Integration tests** for the full flow
- **Edge case tests** for error conditions

### **Key Learning Moments**:
- **LLVM IR structure** and how to navigate it
- **Type system design** principles
- **Error handling** in compilers
- **Performance considerations** in type checking

---

## 🚀 **Getting Started Today**

### **Step 1**: Build the Foundation
```bash
# Add to Makefile to include new file
# Build and run existing tests to make sure nothing is broken
make clean && make test
```

### **Step 2**: Write Your First Test
Create a simple test to drive development:
```cpp
TEST_F(SemanticTypeCheckerTest, BasicTypeCompatibility) {
    // Test: int = int should be compatible
    ParsedType intType = createIntType();
    EXPECT_TRUE(SemanticTypeChecker::areSemanticTypesCompatible(intType, intType));
}
```

### **Step 3**: Implement `areSemanticTypesCompatible()` - Exact Match Case
Start with the simplest possible implementation:
```cpp
bool SemanticTypeChecker::areSemanticTypesCompatible(
    const ParsedType& declared,
    const ParsedType& actual
) {
    // Step 1: Just handle exact matches
    if (declared.primaryType.getType() == actual.primaryType.getType() &&
        declared.isReference == actual.isReference &&
        declared.isMutReference == actual.isMutReference) {
        return true;
    }
    return false;
}
```

---

## 🤝 **Ready to Start?**

**Your first assignment**: Implement the exact match case in `areSemanticTypesCompatible()` and write a test for it.

**Questions to think about**:
1. What fields in `ParsedType` do we need to compare for exact matches?
2. How should we handle the case where both types are `int` but one is `&int`?
3. What test cases would convince you that exact matching works?

**Let's start with Day 1!** Are you ready to implement the basic type compatibility checking? 🎯