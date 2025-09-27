# 🚀 **FORGE BACKEND REWRITE - PROJECT MASTER PLAN**

*Version 1.0 - Complete Architecture & Implementation Specification*

---

## 📋 **PROJECT OVERVIEW**

### **Mission Statement**
Rewrite the Forge language backend from scratch with a clean, maintainable architecture that supports advanced memory safety features including strict borrowing, comprehensive type checking, and robust error handling.

### **Success Criteria**
- ✅ 100% feature parity with current backend
- ✅ Zero type safety vulnerabilities
- ✅ Comprehensive borrow checking system
- ✅ 95%+ test coverage
- ✅ Clean, documented APIs
- ✅ 4-week delivery timeline

### **Technical Requirements**
- **Language**: C++17
- **LLVM Version**: 15.x
- **Testing**: Google Test framework
- **Error Handling**: Result<T, Error> pattern
- **Memory Model**: Rust-inspired strict borrowing
- **Architecture**: Dependency injection, single responsibility

---

## 🏗️ **SYSTEM ARCHITECTURE**

### **Core Modules**

```
forge::backend/
├── types/           # Type system and checking
├── memory/          # Memory management and borrowing
├── codegen/         # LLVM code generation
├── errors/          # Error handling and reporting
├── analysis/        # Static analysis passes
└── runtime/         # Runtime support functions
```

### **Dependency Graph**

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│    Parser   │────▶│ TypeChecker │────▶│   Codegen   │
│   (Exists)  │     │             │     │             │
└─────────────┘     └─────────────┘     └─────────────┘
                            │                   │
                            ▼                   │
                    ┌─────────────┐             │
                    │BorrowChecker│             │
                    │             │             │
                    └─────────────┘             │
                            │                   │
                            ▼                   ▼
                    ┌─────────────┐     ┌─────────────┐
                    │MemoryModel  │     │ ErrorReporter│
                    │             │     │             │
                    └─────────────┘     └─────────────┘
```

---

## 📚 **COMPONENT SPECIFICATIONS**

## **1. TYPE SYSTEM (`forge::types`)**

### **Core Classes**

#### **`Type` (Abstract Base)**
```cpp
namespace forge::types {

class Type {
public:
    enum class Kind {
        Primitive,    // int, float, bool, string
        Pointer,      // *T
        Reference,    // &T, &mut T
        SmartPointer, // unique<T>, shared<T>, weak<T>
        Array,        // [T; N], [T]
        Function,     // (T1, T2) -> T3
        Struct,       // Custom user types
        Generic       // T, U (template params)
    };

    // Core Interface
    virtual Kind getKind() const = 0;
    virtual std::string toString() const = 0;
    virtual size_t getSizeBytes() const = 0;
    virtual llvm::Type* toLLVMType(llvm::LLVMContext& ctx) const = 0;

    // Type Relationships
    virtual bool isAssignableFrom(const Type& other) const = 0;
    virtual bool canImplicitlyConvertTo(const Type& other) const = 0;
    virtual std::optional<std::unique_ptr<Type>> promoteWith(const Type& other) const = 0;

    // Memory Properties
    virtual bool requiresCleanup() const = 0;
    virtual bool isCopyable() const = 0;
    virtual bool isMovable() const = 0;

    virtual ~Type() = default;
};

class PrimitiveType : public Type {
private:
    TokenType primitiveKind_;  // INT, FLOAT, BOOL, STRING

public:
    explicit PrimitiveType(TokenType kind);

    TokenType getPrimitiveKind() const { return primitiveKind_; }

    // Type interface implementation
    Kind getKind() const override { return Kind::Primitive; }
    std::string toString() const override;
    size_t getSizeBytes() const override;
    llvm::Type* toLLVMType(llvm::LLVMContext& ctx) const override;
    bool isAssignableFrom(const Type& other) const override;
    bool canImplicitlyConvertTo(const Type& other) const override;
    std::optional<std::unique_ptr<Type>> promoteWith(const Type& other) const override;
    bool requiresCleanup() const override { return false; }
    bool isCopyable() const override { return true; }
    bool isMovable() const override { return true; }
};

class ReferenceType : public Type {
private:
    std::unique_ptr<Type> pointedType_;
    bool isMutable_;

public:
    ReferenceType(std::unique_ptr<Type> pointedType, bool isMutable);

    const Type& getPointedType() const { return *pointedType_; }
    bool isMutable() const { return isMutable_; }

    // Type interface implementation
    Kind getKind() const override { return Kind::Reference; }
    std::string toString() const override;
    size_t getSizeBytes() const override { return sizeof(void*); }
    llvm::Type* toLLVMType(llvm::LLVMContext& ctx) const override;
    bool isAssignableFrom(const Type& other) const override;
    bool canImplicitlyConvertTo(const Type& other) const override;
    std::optional<std::unique_ptr<Type>> promoteWith(const Type& other) const override;
    bool requiresCleanup() const override { return false; }
    bool isCopyable() const override { return true; }
    bool isMovable() const override { return true; }
};

class SmartPointerType : public Type {
public:
    enum class PointerKind { Unique, Shared, Weak };

private:
    PointerKind pointerKind_;
    std::unique_ptr<Type> elementType_;

public:
    SmartPointerType(PointerKind kind, std::unique_ptr<Type> elementType);

    PointerKind getPointerKind() const { return pointerKind_; }
    const Type& getElementType() const { return *elementType_; }

    // Type interface implementation
    Kind getKind() const override { return Kind::SmartPointer; }
    std::string toString() const override;
    size_t getSizeBytes() const override;
    llvm::Type* toLLVMType(llvm::LLVMContext& ctx) const override;
    bool isAssignableFrom(const Type& other) const override;
    bool canImplicitlyConvertTo(const Type& other) const override;
    std::optional<std::unique_ptr<Type>> promoteWith(const Type& other) const override;
    bool requiresCleanup() const override { return true; }
    bool isCopyable() const override;
    bool isMovable() const override { return true; }
};

} // namespace forge::types
```

#### **`TypeChecker`**
```cpp
namespace forge::types {

class TypeChecker {
public:
    explicit TypeChecker(errors::ErrorReporter& errorReporter);

    // Type Analysis
    Result<std::unique_ptr<Type>, errors::TypeError>
    inferExpressionType(const ast::Expression& expr, const SymbolTable& symbols);

    Result<std::unique_ptr<Type>, errors::TypeError>
    analyzeType(const ast::ParsedType& parsedType);

    // Compatibility Checking
    Result<bool, errors::TypeError>
    areTypesCompatible(const Type& declared, const Type& actual);

    Result<std::unique_ptr<Type>, errors::TypeError>
    findCommonType(const Type& left, const Type& right);

    // Assignment Validation
    Result<void, errors::TypeError>
    validateAssignment(const Type& target, const Type& source, SourceLocation location);

    Result<void, errors::TypeError>
    validateFunctionCall(const Type& function, const std::vector<Type*>& args, SourceLocation location);

private:
    errors::ErrorReporter& errorReporter_;

    // Internal helpers
    Result<std::unique_ptr<Type>, errors::TypeError>
    inferBinaryExpressionType(const ast::BinaryExpression& expr, const SymbolTable& symbols);

    Result<std::unique_ptr<Type>, errors::TypeError>
    inferUnaryExpressionType(const ast::UnaryExpression& expr, const SymbolTable& symbols);

    bool isImplicitConversionAllowed(const Type& from, const Type& to);
    std::unique_ptr<Type> createPromotedType(const Type& left, const Type& right);
};

} // namespace forge::types
```

---

## **2. MEMORY MANAGEMENT (`forge::memory`)**

### **Core Classes**

#### **`MemoryModel`**
```cpp
namespace forge::memory {

class MemoryModel {
public:
    enum class Ownership {
        Owned,          // Variable owns the value exclusively
        Borrowed,       // Temporary immutable access
        MutBorrowed,    // Temporary mutable access
        Moved           // Value has been transferred
    };

    enum class Lifetime {
        Static,         // 'static lifetime
        Function,       // Lives for function duration
        Block,          // Lives for block duration
        Expression      // Lives for expression duration
    };

    explicit MemoryModel(errors::ErrorReporter& errorReporter);

    // Ownership Tracking
    Result<void, errors::BorrowError>
    registerVariable(VariableId id, const types::Type& type, Lifetime lifetime);

    Result<void, errors::BorrowError>
    registerBorrow(VariableId target, bool isMutable, SourceLocation location);

    Result<void, errors::BorrowError>
    registerMove(VariableId source, SourceLocation location);

    Result<void, errors::BorrowError>
    endBorrow(VariableId target, SourceLocation location);

    // Lifetime Analysis
    Result<Lifetime, errors::BorrowError>
    computeLifetime(const ast::Expression& expr);

    Result<void, errors::BorrowError>
    validateLifetimes(const ast::Statement& stmt);

    // Query Interface
    Ownership getOwnership(VariableId id) const;
    Lifetime getLifetime(VariableId id) const;
    bool hasActiveBorrows(VariableId id) const;
    std::vector<BorrowInfo> getActiveBorrows(VariableId id) const;

private:
    errors::ErrorReporter& errorReporter_;

    struct VariableInfo {
        types::Type type;
        Ownership ownership;
        Lifetime lifetime;
        std::vector<BorrowInfo> activeBorrows;
        SourceLocation declaration;
    };

    std::unordered_map<VariableId, VariableInfo> variables_;
    LifetimeAnalyzer lifetimeAnalyzer_;
};

struct BorrowInfo {
    VariableId borrower;
    bool isMutable;
    SourceLocation location;
    Lifetime expectedLifetime;
};

} // namespace forge::memory
```

#### **`BorrowChecker`**
```cpp
namespace forge::memory {

class BorrowChecker {
public:
    explicit BorrowChecker(MemoryModel& memoryModel, errors::ErrorReporter& errorReporter);

    // Main Analysis Interface
    Result<void, errors::BorrowError>
    analyzeProgram(const ast::Program& program);

    Result<void, errors::BorrowError>
    analyzeStatement(const ast::Statement& stmt);

    Result<void, errors::BorrowError>
    analyzeExpression(const ast::Expression& expr);

private:
    MemoryModel& memoryModel_;
    errors::ErrorReporter& errorReporter_;

    // Core Borrow Checking Rules
    Result<void, errors::BorrowError>
    checkBorrowRules(VariableId target, bool isMutable, SourceLocation location);

    Result<void, errors::BorrowError>
    checkMoveRules(VariableId source, SourceLocation location);

    Result<void, errors::BorrowError>
    checkLifetimeRules(const ast::Expression& expr);

    // Statement-specific analysis
    Result<void, errors::BorrowError>
    analyzeVariableDeclaration(const ast::VariableDeclaration& decl);

    Result<void, errors::BorrowError>
    analyzeAssignment(const ast::Assignment& assignment);

    Result<void, errors::BorrowError>
    analyzeFunctionCall(const ast::FunctionCall& call);

    // Expression-specific analysis
    Result<void, errors::BorrowError>
    analyzeAddressOf(const ast::UnaryExpression& expr);

    Result<void, errors::BorrowError>
    analyzeDereference(const ast::UnaryExpression& expr);

    Result<void, errors::BorrowError>
    analyzeMove(const ast::MoveExpression& expr);
};

} // namespace forge::memory
```

---

## **3. CODE GENERATION (`forge::codegen`)**

### **Core Classes**

#### **`LLVMBackend`**
```cpp
namespace forge::codegen {

class LLVMBackend {
public:
    explicit LLVMBackend(llvm::LLVMContext& context);

    // Main Compilation Interface
    Result<std::unique_ptr<llvm::Module>, errors::CodegenError>
    compileProgram(const ast::Program& program);

    // Individual Component Access (for testing)
    const types::TypeChecker& getTypeChecker() const { return typeChecker_; }
    const memory::BorrowChecker& getBorrowChecker() const { return borrowChecker_; }
    const errors::ErrorReporter& getErrorReporter() const { return errorReporter_; }

private:
    // Core Components
    llvm::LLVMContext& context_;
    std::unique_ptr<llvm::Module> module_;
    llvm::IRBuilder<> builder_;

    // Analysis & Checking
    errors::ErrorReporter errorReporter_;
    types::TypeChecker typeChecker_;
    memory::MemoryModel memoryModel_;
    memory::BorrowChecker borrowChecker_;

    // Code Generation
    ExpressionGenerator exprGenerator_;
    StatementGenerator stmtGenerator_;
    FunctionGenerator funcGenerator_;

    // Symbol Management
    SymbolTable symbolTable_;

    // Compilation Pipeline
    Result<void, errors::CodegenError> runAnalysisPasses(const ast::Program& program);
    Result<void, errors::CodegenError> generateCode(const ast::Program& program);
    Result<void, errors::CodegenError> optimizeModule();
};

} // namespace forge::codegen
```

#### **`ExpressionGenerator`**
```cpp
namespace forge::codegen {

class ExpressionGenerator {
public:
    ExpressionGenerator(
        llvm::LLVMContext& context,
        llvm::IRBuilder<>& builder,
        const types::TypeChecker& typeChecker,
        const memory::MemoryModel& memoryModel,
        SymbolTable& symbolTable,
        errors::ErrorReporter& errorReporter
    );

    // Main Generation Interface
    Result<llvm::Value*, errors::CodegenError>
    generate(const ast::Expression& expr);

private:
    // Dependencies
    llvm::LLVMContext& context_;
    llvm::IRBuilder<>& builder_;
    const types::TypeChecker& typeChecker_;
    const memory::MemoryModel& memoryModel_;
    SymbolTable& symbolTable_;
    errors::ErrorReporter& errorReporter_;

    // Expression Type Handlers
    Result<llvm::Value*, errors::CodegenError>
    generateLiteral(const ast::LiteralExpression& expr);

    Result<llvm::Value*, errors::CodegenError>
    generateIdentifier(const ast::IdentifierExpression& expr);

    Result<llvm::Value*, errors::CodegenError>
    generateBinary(const ast::BinaryExpression& expr);

    Result<llvm::Value*, errors::CodegenError>
    generateUnary(const ast::UnaryExpression& expr);

    Result<llvm::Value*, errors::CodegenError>
    generateFunctionCall(const ast::FunctionCall& expr);

    Result<llvm::Value*, errors::CodegenError>
    generateArrayLiteral(const ast::ArrayLiteralExpression& expr);

    Result<llvm::Value*, errors::CodegenError>
    generateIndexAccess(const ast::IndexAccessExpression& expr);

    Result<llvm::Value*, errors::CodegenError>
    generateMemberAccess(const ast::MemberAccessExpression& expr);

    Result<llvm::Value*, errors::CodegenError>
    generateMove(const ast::MoveExpression& expr);

    // Memory Operations
    Result<llvm::Value*, errors::CodegenError>
    generateAddressOf(const ast::UnaryExpression& expr);

    Result<llvm::Value*, errors::CodegenError>
    generateDereference(const ast::UnaryExpression& expr);

    // Type Conversion Helpers
    Result<llvm::Value*, errors::CodegenError>
    insertImplicitConversion(llvm::Value* value, const types::Type& fromType, const types::Type& toType);

    Result<llvm::Value*, errors::CodegenError>
    createTypePromotion(llvm::Value* left, llvm::Value* right, const types::Type& targetType);
};

} // namespace forge::codegen
```

#### **`StatementGenerator`**
```cpp
namespace forge::codegen {

class StatementGenerator {
public:
    StatementGenerator(
        llvm::LLVMContext& context,
        llvm::IRBuilder<>& builder,
        const types::TypeChecker& typeChecker,
        const memory::MemoryModel& memoryModel,
        ExpressionGenerator& exprGenerator,
        SymbolTable& symbolTable,
        errors::ErrorReporter& errorReporter
    );

    // Main Generation Interface
    Result<void, errors::CodegenError>
    generate(const ast::Statement& stmt);

private:
    // Dependencies
    llvm::LLVMContext& context_;
    llvm::IRBuilder<>& builder_;
    const types::TypeChecker& typeChecker_;
    const memory::MemoryModel& memoryModel_;
    ExpressionGenerator& exprGenerator_;
    SymbolTable& symbolTable_;
    errors::ErrorReporter& errorReporter_;

    // Statement Type Handlers
    Result<void, errors::CodegenError>
    generateVariableDeclaration(const ast::VariableDeclaration& stmt);

    Result<void, errors::CodegenError>
    generateAssignment(const ast::Assignment& stmt);

    Result<void, errors::CodegenError>
    generateExpressionStatement(const ast::ExpressionStatement& stmt);

    Result<void, errors::CodegenError>
    generateBlock(const ast::BlockStatement& stmt);

    Result<void, errors::CodegenError>
    generateIf(const ast::IfStatement& stmt);

    Result<void, errors::CodegenError>
    generateWhile(const ast::WhileStatement& stmt);

    Result<void, errors::CodegenError>
    generateReturn(const ast::ReturnStatement& stmt);

    Result<void, errors::CodegenError>
    generateFunctionDefinition(const ast::FunctionDefinition& stmt);

    Result<void, errors::CodegenError>
    generateDefer(const ast::DeferStatement& stmt);

    // Memory Management
    Result<void, errors::CodegenError>
    insertCleanupCode(const SymbolTable::Scope& scope);

    Result<void, errors::CodegenError>
    generateSmartPointerCleanup(llvm::Value* smartPtr, const types::SmartPointerType& type);
};

} // namespace forge::codegen
```

---

## **4. ERROR HANDLING (`forge::errors`)**

### **Core Classes**

#### **`Result<T, E>`**
```cpp
namespace forge::errors {

template<typename T, typename E>
class Result {
public:
    // Factory Methods
    static Result Ok(T value) {
        return Result(std::move(value), true);
    }

    static Result Err(E error) {
        return Result(std::move(error), false);
    }

    // State Checking
    bool isOk() const { return isOk_; }
    bool isErr() const { return !isOk_; }

    // Value Access
    T& unwrap() {
        if (!isOk_) {
            throw std::runtime_error("Called unwrap() on error Result");
        }
        return value_;
    }

    const T& unwrap() const {
        if (!isOk_) {
            throw std::runtime_error("Called unwrap() on error Result");
        }
        return value_;
    }

    T unwrapOr(T defaultValue) {
        return isOk_ ? std::move(value_) : std::move(defaultValue);
    }

    E& error() {
        if (isOk_) {
            throw std::runtime_error("Called error() on ok Result");
        }
        return error_;
    }

    const E& error() const {
        if (isOk_) {
            throw std::runtime_error("Called error() on ok Result");
        }
        return error_;
    }

    // Functional Combinators
    template<typename F>
    auto map(F func) -> Result<decltype(func(std::declval<T>())), E> {
        using ReturnType = decltype(func(std::declval<T>()));
        if (isOk_) {
            return Result<ReturnType, E>::Ok(func(value_));
        } else {
            return Result<ReturnType, E>::Err(error_);
        }
    }

    template<typename F>
    auto andThen(F func) -> decltype(func(std::declval<T>())) {
        if (isOk_) {
            return func(value_);
        } else {
            using ReturnType = decltype(func(std::declval<T>()));
            return ReturnType::Err(error_);
        }
    }

private:
    union {
        T value_;
        E error_;
    };
    bool isOk_;

    Result(T value, bool) : value_(std::move(value)), isOk_(true) {}
    Result(E error, bool) : error_(std::move(error)), isOk_(false) {}

public:
    ~Result() {
        if (isOk_) {
            value_.~T();
        } else {
            error_.~E();
        }
    }

    // Move constructor and assignment
    Result(Result&& other) noexcept : isOk_(other.isOk_) {
        if (isOk_) {
            new(&value_) T(std::move(other.value_));
        } else {
            new(&error_) E(std::move(other.error_));
        }
    }

    Result& operator=(Result&& other) noexcept {
        if (this != &other) {
            this->~Result();
            isOk_ = other.isOk_;
            if (isOk_) {
                new(&value_) T(std::move(other.value_));
            } else {
                new(&error_) E(std::move(other.error_));
            }
        }
        return *this;
    }

    // Delete copy operations
    Result(const Result&) = delete;
    Result& operator=(const Result&) = delete;
};

} // namespace forge::errors
```

#### **Error Types**
```cpp
namespace forge::errors {

struct SourceLocation {
    std::string filename;
    size_t line;
    size_t column;
    size_t length;

    std::string toString() const;
};

class CompilerError {
public:
    enum class Level {
        Error,
        Warning,
        Note
    };

    CompilerError(Level level, std::string message, SourceLocation location);

    Level getLevel() const { return level_; }
    const std::string& getMessage() const { return message_; }
    const SourceLocation& getLocation() const { return location_; }

    virtual std::string formatError() const;

protected:
    Level level_;
    std::string message_;
    SourceLocation location_;
};

class TypeError : public CompilerError {
public:
    TypeError(std::string message, SourceLocation location);

    std::string formatError() const override;
};

class BorrowError : public CompilerError {
public:
    enum class Kind {
        UseAfterMove,
        MutableBorrowWhileImmutableBorrows,
        MultipleMutableBorrows,
        LifetimeTooShort,
        InvalidBorrow
    };

    BorrowError(Kind kind, std::string message, SourceLocation location);

    Kind getKind() const { return kind_; }
    std::string formatError() const override;

private:
    Kind kind_;
};

class CodegenError : public CompilerError {
public:
    CodegenError(std::string message, SourceLocation location);

    std::string formatError() const override;
};

} // namespace forge::errors
```

#### **`ErrorReporter`**
```cpp
namespace forge::errors {

class ErrorReporter {
public:
    ErrorReporter() = default;

    // Error Reporting
    void reportError(std::unique_ptr<CompilerError> error);
    void reportTypeError(std::string message, SourceLocation location);
    void reportBorrowError(BorrowError::Kind kind, std::string message, SourceLocation location);
    void reportCodegenError(std::string message, SourceLocation location);
    void reportWarning(std::string message, SourceLocation location);

    // Query Interface
    bool hasErrors() const;
    bool hasWarnings() const;
    size_t getErrorCount() const;
    size_t getWarningCount() const;

    const std::vector<std::unique_ptr<CompilerError>>& getErrors() const { return errors_; }

    // Output
    void printErrors(std::ostream& out) const;
    void printSummary(std::ostream& out) const;

    // Clear state
    void clear();

private:
    std::vector<std::unique_ptr<CompilerError>> errors_;

    void sortErrorsByLocation();
};

} // namespace forge::errors
```

---

## **5. SUPPORTING SYSTEMS**

### **Symbol Table**
```cpp
namespace forge::codegen {

class SymbolTable {
public:
    struct Symbol {
        std::string name;
        std::unique_ptr<types::Type> type;
        llvm::Value* llvmValue;
        memory::MemoryModel::Ownership ownership;
        SourceLocation declaration;
    };

    class Scope {
    public:
        void declare(std::string name, Symbol symbol);
        Symbol* lookup(const std::string& name);
        const Symbol* lookup(const std::string& name) const;

        std::vector<Symbol*> getAllSymbols();

    private:
        std::unordered_map<std::string, std::unique_ptr<Symbol>> symbols_;
    };

    // Scope Management
    void enterScope();
    void exitScope();
    size_t getCurrentScopeDepth() const;

    // Symbol Management
    Result<void, errors::TypeError> declare(std::string name, Symbol symbol);
    Symbol* lookup(const std::string& name);
    const Symbol* lookup(const std::string& name) const;

    // Query Interface
    std::vector<Symbol*> getSymbolsInCurrentScope();
    std::vector<Symbol*> getAllVisibleSymbols();

private:
    std::vector<std::unique_ptr<Scope>> scopes_;

    Scope& getCurrentScope();
    const Scope& getCurrentScope() const;
};

} // namespace forge::codegen
```

---

## **📅 IMPLEMENTATION TIMELINE**

### **Week 1: Foundation (Days 1-7)**

#### **Day 1-2: Project Setup & Core Infrastructure**
- [ ] Create clean project structure
- [ ] Implement `Result<T, E>` template
- [ ] Create base error classes
- [ ] Set up Google Test framework
- [ ] Implement `SourceLocation` utilities

**Deliverables:**
- Working `Result<T, E>` with unit tests
- Basic error hierarchy
- Project structure documentation

#### **Day 3-4: Type System Core**
- [ ] Implement `Type` base class and hierarchy
- [ ] Create `PrimitiveType`, `ReferenceType`, `SmartPointerType`
- [ ] Implement type compatibility checking
- [ ] Add comprehensive unit tests

**Deliverables:**
- Complete type hierarchy
- Type compatibility algorithm
- 95%+ test coverage for type system

#### **Day 5-7: TypeChecker Implementation**
- [ ] Implement `TypeChecker` class
- [ ] Add expression type inference
- [ ] Implement type promotion rules
- [ ] Integration tests with existing parser

**Deliverables:**
- Working type checker
- Integration with existing AST
- Type inference for all expression types

### **Week 2: Memory Management (Days 8-14)**

#### **Day 8-9: Memory Model Foundation**
- [ ] Implement `MemoryModel` class
- [ ] Create ownership tracking system
- [ ] Implement lifetime analysis basics
- [ ] Unit tests for memory model

**Deliverables:**
- Working memory model
- Ownership state tracking
- Lifetime computation algorithms

#### **Day 10-12: Borrow Checker**
- [ ] Implement `BorrowChecker` class
- [ ] Add core borrowing rules
- [ ] Implement move semantics checking
- [ ] Comprehensive borrow checking tests

**Deliverables:**
- Complete borrow checker
- All Rust-like borrowing rules implemented
- Extensive test suite

#### **Day 13-14: Integration & Testing**
- [ ] Integrate memory system with type checker
- [ ] End-to-end memory safety tests
- [ ] Performance optimization
- [ ] Documentation

**Deliverables:**
- Integrated type + memory checking
- Performance benchmarks
- Complete API documentation

### **Week 3: Code Generation (Days 15-21)**

#### **Day 15-16: LLVM Backend Foundation**
- [ ] Implement `LLVMBackend` class
- [ ] Create `SymbolTable` implementation
- [ ] Basic module generation
- [ ] Integration with analysis passes

**Deliverables:**
- Working LLVM backend skeleton
- Symbol table management
- Analysis pass integration

#### **Day 17-19: Expression Generation**
- [ ] Implement `ExpressionGenerator` class
- [ ] Generate code for all expression types
- [ ] Type conversion and promotion
- [ ] Memory operation code generation

**Deliverables:**
- Complete expression code generation
- Type conversion system
- Memory-safe code generation

#### **Day 20-21: Statement Generation**
- [ ] Implement `StatementGenerator` class
- [ ] Variable declaration code generation
- [ ] Control flow implementation
- [ ] Function definition support

**Deliverables:**
- Complete statement code generation
- Control flow support
- Function call/definition system

### **Week 4: Integration & Polish (Days 22-28)**

#### **Day 22-24: End-to-End Integration**
- [ ] Complete backend integration
- [ ] Full compilation pipeline
- [ ] Error handling polish
- [ ] Memory cleanup implementation

**Deliverables:**
- Working end-to-end compilation
- Comprehensive error messages
- Memory management integration

#### **Day 25-26: Testing & Optimization**
- [ ] Comprehensive integration tests
- [ ] Performance optimization
- [ ] Memory leak detection
- [ ] Stress testing

**Deliverables:**
- 95%+ test coverage
- Performance benchmarks
- Memory safety validation

#### **Day 27-28: Documentation & Handoff**
- [ ] Complete API documentation
- [ ] Architecture decision documentation
- [ ] Performance analysis
- [ ] Future enhancement roadmap

**Deliverables:**
- Complete documentation
- Architecture guide
- Performance report
- Enhancement roadmap

---

## **🧪 TESTING STRATEGY**

### **Unit Testing (95% Coverage Target)**
- **Type System**: Every type operation, compatibility rule, and edge case
- **Memory Model**: All ownership transitions and borrowing scenarios
- **Code Generation**: Each expression/statement type with various inputs
- **Error Handling**: All error conditions and recovery paths

### **Integration Testing**
- **Type + Memory**: Interaction between type checking and borrow checking
- **Analysis + Codegen**: End-to-end compilation pipeline
- **Error Reporting**: Comprehensive error scenario testing

### **Property-Based Testing**
- **Type Compatibility**: Generate random type pairs and verify transitivity
- **Memory Safety**: Generate random programs and verify no memory errors
- **Code Generation**: Verify LLVM IR correctness for generated code

### **Performance Testing**
- **Compilation Speed**: Benchmark against current implementation
- **Memory Usage**: Monitor memory consumption during compilation
- **Generated Code Quality**: Compare performance of generated binaries

---

## **📋 DEFINITION OF DONE**

### **Individual Components**
- [ ] 95%+ unit test coverage
- [ ] Complete API documentation
- [ ] Integration tests passing
- [ ] Performance benchmarks meeting targets
- [ ] Code review completed

### **Overall Project**
- [ ] 100% feature parity with current backend
- [ ] All borrowing system features working
- [ ] Zero memory safety vulnerabilities
- [ ] Comprehensive test suite (unit + integration)
- [ ] Complete documentation
- [ ] Performance equal to or better than current implementation

---

## **🚧 RISK MITIGATION**

### **Technical Risks**
- **LLVM Integration Complexity**: Prototype early, incremental testing
- **Borrow Checker Complexity**: Reference Rust implementation, start simple
- **Performance Degradation**: Continuous benchmarking, optimization passes

### **Schedule Risks**
- **Feature Creep**: Strict scope adherence, defer non-essential features
- **Integration Issues**: Daily integration testing, early issue detection
- **Debugging Time**: Comprehensive logging, debugger-friendly code

### **Quality Risks**
- **Incomplete Testing**: Automated coverage reporting, continuous testing
- **Documentation Drift**: Documentation-driven development, regular reviews
- **Memory Leaks**: Automated memory testing, RAII enforcement

---

This comprehensive plan provides the complete blueprint for your backend rewrite. Each component has clear interfaces, the timeline is realistic, and the testing strategy ensures quality. Ready to start implementing? Which component would you like to tackle first?