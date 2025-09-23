# Array Implementation Guide

This guide outlines what needs to be implemented to support dynamic arrays in Forge, based on the comprehensive test suite that has been added.

## Test Coverage Added

### Lexer Tests (8 tests)
- `ArrayTypeDeclaration` - `Array[int]`
- `ArrayLiteralEmpty` - `[]`
- `ArrayLiteralWithNumbers` - `[1, 2, 3]`
- `ArrayLiteralWithStrings` - `["hello", "world"]`
- `ArrayIndexAccess` - `arr[0]`
- `ArrayVariableDeclaration` - `numbers: Array[int] = Array.new()`
- `ArrayMethodCalls` - `numbers.push(42)`
- `ArrayMethodChaining` - `arr.push(1).push(2).length()`
- `NestedArrayTypes` - `Array[Array[int]]`
- `ComplexArrayExpression` - `matrix[i][j] = arr[0] + 5`

### Parser Tests (7 tests)
- `ParseArrayLiteralEmpty` - `[]`
- `ParseArrayLiteralWithElements` - `[1, 2, 3]`
- `ParseArrayIndexAccess` - `arr[0]`
- `ParseArrayMemberAccess` - `arr.push(42)` and `Array.new()`
- `ParseNestedArrayAccess` - `matrix[i][j]`
- `ParseChainedMethodCalls` - `arr.push(1).push(2)`

### BytecodeCompiler Tests (9 tests)
- `CompileArrayLiteralEmpty`
- `CompileArrayLiteralWithElements`
- `CompileArrayVariableDeclaration`
- `CompileArrayIndexAccess`
- `CompileArrayIndexAssignment`
- `CompileArrayMethodCall`
- `CompileArrayLengthAccess`
- `CompileNestedArrayAccess`
- `CompileArrayInFunction`

### VM Execution Tests (16 tests)
- `ExecuteArrayCreation`
- `ExecuteArrayLiteralEmpty`
- `ExecuteArrayLiteralWithElements`
- `ExecuteArrayPush`
- `ExecuteArrayPop`
- `ExecuteArrayIndexAccess`
- `ExecuteArrayIndexAssignment`
- `ExecuteArrayLength`
- `ExecuteArrayWithStrings`
- `ExecuteArrayInFunction`
- `ExecuteNestedArrays`
- `ExecuteArrayMethodChaining`
- `ExecuteArrayBoundsCheck`
- `ExecuteArrayGarbageCollection`
- `ExecuteArrayWithMixedOperations`

## Implementation Requirements

### 1. Lexer - Add DOT Token Support
**File:** `src/lexer/Tokenizer.cpp`

Need to add `.` character recognition:
```cpp
case '.':
    return Token(TokenType::DOT, current_line, current_column);
```

### 2. Parser - New Expression Types
**Files:** `include/parser/Expression.hpp`, `src/parser/Expression.cpp`

Add new AST node classes:
- `ArrayLiteralExpression` - for `[1, 2, 3]`
- `IndexAccessExpression` - for `arr[0]`
- `MemberAccessExpression` - for both `obj.method(args)` and `Class.method(args)`

**Note:** Use a single `MemberAccessExpression` class instead of separate method call types. The distinction between instance methods (`obj.method()`) and static methods (`Class.method()`) should be made during semantic analysis/compilation, not parsing, since they are syntactically identical.

### 3. Parser - Update Expression Parsing
**File:** `src/parser/Parser.cpp`

Modify `parsePrimaryExpression()` to handle:
- `[` - Array literals
- Identifier followed by `[` - Index access
- Identifier followed by `.` - Member access (both method calls and static calls)

**Example MemberAccessExpression class:**
```cpp
class MemberAccessExpression : public Expression {
public:
    std::unique_ptr<Expression> object;  // Left side (obj or Class)
    std::string memberName;              // Method/property name
    std::vector<std::unique_ptr<Expression>> arguments; // For method calls
    bool isMethodCall;                   // true if followed by ()
};
```

**Parser logic:**
```cpp
if (currentToken == IDENTIFIER && peek() == DOT) {
    auto leftExpr = parseIdentifier();
    advance(); // consume DOT
    auto memberName = advance().getValue();

    if (current() == LPAREN) {
        auto args = parseArgumentList();
        return std::make_unique<MemberAccessExpression>(
            std::move(leftExpr), memberName, std::move(args), true);
    } else {
        return std::make_unique<MemberAccessExpression>(
            std::move(leftExpr), memberName);
    }
}
```

### 4. Object System - Add ArrayObject
**Files:** `include/backends/vm/Object.hpp`, `src/backends/vm/Object.cpp`

```cpp
enum ObjectType {
    STRING,
    FUNCTION,
    ARRAY
};

struct ArrayObject : Object {
    std::vector<Value> elements;
    size_t capacity;
    ValueType elementType;  // For type checking
};

bool isArray(Value val);
ArrayObject* asArray(Value val);
```

### 5. VM - New OpCodes
**File:** `include/backends/vm/OPCode.hpp`

Add array operation opcodes:
```cpp
enum class OPCode {
    // ... existing opcodes ...

    // Array operations
    ARRAY_NEW,              // Create empty array
    ARRAY_NEW_WITH_SIZE,    // Create array with initial size
    ARRAY_GET,              // Get element by index
    ARRAY_SET,              // Set element by index
    ARRAY_LENGTH,           // Get array length
    ARRAY_PUSH,             // Push element
    ARRAY_POP,              // Pop element
};
```

### 6. VM - Array Instruction Implementation
**File:** `src/backends/vm/VirtualMachine.cpp`

Implement array operation cases in the VM execution loop:
```cpp
case OPCode::ARRAY_NEW: {
    ArrayObject* arr = heap.allocateArray();
    pushValue(createObject(arr));
    break;
}
case OPCode::ARRAY_GET: {
    Value index = popValue();
    Value array = popValue();
    // Bounds checking and element access
    break;
}
// ... etc for other opcodes
```

### 7. Heap - Array Allocation and GC
**Files:** `include/backends/vm/Heap.hpp`, `src/backends/vm/Heap.cpp`

Add array allocation and garbage collection support:
```cpp
ArrayObject* allocateArray();
void markArray(ArrayObject* array);
```

### 8. BytecodeCompiler - Array Compilation
**File:** `src/backends/vm/BytecodeCompiler.cpp`

Add compilation methods for new expression types:
```cpp
void BytecodeCompiler::compileArrayLiteral(ArrayLiteralExpression* expr);
void BytecodeCompiler::compileIndexAccess(IndexAccessExpression* expr);
void BytecodeCompiler::compileMemberAccess(MemberAccessExpression* expr);
```

**Semantic analysis in compiler:**
```cpp
void BytecodeCompiler::compileMemberAccess(MemberAccessExpression* expr) {
    // Determine if it's static or instance method call
    if (isTypeName(expr->object)) {
        // Static method: Array.new()
        compileStaticMethodCall(expr);
    } else {
        // Instance method: arr.push(42)
        compileInstanceMethodCall(expr);
    }
}

bool BytecodeCompiler::isTypeName(Expression* expr) {
    if (auto id = dynamic_cast<IdentifierExpression*>(expr)) {
        return id->name == "Array" || id->name == "String"; // etc.
    }
    return false;
}
```

### 9. Type System - Array Type Support
Add support for `Array[T]` type declarations in variable parsing and type checking.

## Syntax Supported

### Array Creation
```forge
arr: Array[int] = Array.new()         // Empty array
numbers: Array[int] = [1, 2, 3]       // Array literal
matrix: Array[Array[int]] = [[1, 2]]  // Nested arrays
```

### Array Operations
```forge
arr.push(42)          // Add element
value: int = arr.pop() // Remove and return last element
size: int = arr.length() // Get length
element: int = arr[0]    // Index access
arr[1] = 99             // Index assignment
```

### Method Chaining
```forge
arr.push(1).push(2).push(3)  // Chaining
```

## Implementation Order

1. **Lexer**: Add DOT token recognition
2. **Parser**: Add new expression AST nodes
3. **Parser**: Update expression parsing logic
4. **Object System**: Add ArrayObject and related functions
5. **VM**: Add array opcodes to OPCode enum
6. **Heap**: Add array allocation and GC support
7. **VM**: Implement array operation execution
8. **BytecodeCompiler**: Add array compilation methods
9. **Type System**: Add Array[T] type support

## Testing

Run the tests incrementally as you implement:

```bash
# Test lexer
./test_forge --gtest_filter="TokenizerTest.Array*"

# Test parser
./test_forge --gtest_filter="ParserTest.ParseArray*"

# Test compiler
./test_forge --gtest_filter="BytecodeCompilerTest.CompileArray*"

# Test VM
./test_forge --gtest_filter="VirtualMachineTest.ExecuteArray*"
```

The tests will guide you through exactly what needs to be implemented and will pass once the corresponding features are complete.