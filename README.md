# Forge Programming Language

> A modern systems programming language designed for simplicity, safety, and performance

## 🚀 Overview

Forge is a modern systems programming language that combines clean syntax with manual memory management and strong typing. It targets developers who want C++-level performance without the complexity, featuring Rust-inspired safety features and LLVM-based compilation for serious performance.

## ✨ Language Design Goals

### Core Philosophy
- **Simple syntax with braces** (C-style, already implemented)
- **Strong static typing with optionals** (Rust-inspired: `Option<T>`, `Result<T,E>`)
- **Manual memory management with safety** (smart pointers: `unique<T>`, `shared<T>`)
- **High performance** (LLVM-based compilation)
- **Portfolio showcase project** demonstrating advanced compiler techniques

### Current Implementation
- ✅ **Lexer**: Complete tokenization with comprehensive operator support, comment handling, and error reporting
- ✅ **Parser**: Full recursive descent parser with operator precedence, supporting expressions, statements, control flow, functions, and arrays
- ✅ **AST**: Template-based visitor pattern architecture enabling multiple compilation backends
- ✅ **Testing**: 335+ comprehensive tests ensuring reliability across all components

### Target Features
**Immediate Goals:**
- Functions with proper calling conventions
- Basic types (int, float, bool, string, pointers)
- LLVM code generation for expressions and statements

**Medium-term Goals:**
- Safe pointer types and memory management
- Pattern matching on Option/Result types
- Module system and imports
- OOP

**Long-term Goals:**
- C interoperability (extern functions)
- Threading and async/await
- Package manager
- Standard library implementation

## 🏗️ Architecture

Forge uses a modern template-based visitor pattern architecture:

```
Source Code → Lexer → Parser → AST → LLVM Backend → Native Code
```

### Architecture Highlights
- **Frontend**: Completed lexer/parser generating AST
- **Backend**: Template visitor pattern supporting multiple compilation targets
- **Current Implementation**: All components use modern C++17 with comprehensive error handling
- **Build System**: Makefile with separate test/build targets

### Development Phases
1. **Phase 1: LLVM Backend** *(Current Focus)*
   - ✅ Lexical analysis (Complete)
   - ✅ Syntax analysis (Complete)
   - 🎯 LLVM code generation (Next)
   - 🎯 Native compilation pipeline

2. **Phase 2: Advanced Features** *(Future)*
   - Safe pointer types and memory management
   - Pattern matching and advanced type system

3. **Phase 3: Ecosystem** *(Long-term)*
   - C interoperability and standard library
   - Package manager and tooling

## 🛠️ Building

### Prerequisites
- C++17 compatible compiler (GCC 7+ or Clang 5+)
- LLVM 15+ development libraries
- Google Test library
- Make

### Ubuntu/Debian Setup
```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y build-essential llvm-15-dev libllvm15 llvm-15-tools clang-15 libgtest-dev libgmock-dev

# Verify LLVM installation
llvm-config --version

# Clone and build
git clone https://github.com/antonio-dimeglio/forge.git
cd forge
make
```

### LLVM Integration
The project uses LLVM for code generation. The Makefile automatically detects LLVM configuration:
```makefile
LLVM_FLAGS = $(shell llvm-config --cxxflags --ldflags --libs core executionengine mcjit interpreter analysis native bitwriter)
```

### Commands
```bash
# Build the project
make

# Run tests
make test
make run-tests

# Clean build artifacts
make clean

# View project structure
make structure
```

## 🧪 Testing

Forge uses Google Test for comprehensive testing:

```bash
$ make run-tests
Running main() from ./googletest/src/gtest_main.cc
[==========] Running 6 tests from 2 test suites.
[----------] 3 tests from TokenTypeTest
[ RUN      ] TokenTypeTest.BasicTokenToString
[       OK ] TokenTypeTest.BasicTokenToString (0 ms)
[----------] 3 tests from TokenTest
[ RUN      ] TokenTest.ConstructorWithValue
[       OK ] TokenTest.ConstructorWithValue (0 ms)
[==========] 6 tests from 2 test suites ran. (1 ms total)
[  PASSED  ] 6 tests.
```

## 📝 Language Syntax Examples

### Current Supported Features
```rust
// Basic expressions and arithmetic
42 + 3.14 * (10 - 5)
true && (x > 0 || y < 100)

// Variables and assignments
x: int = 42
name := "Alice" // Type inferred

// Functions
def fibonacci(n: int) -> int {
    if (n <= 1) {
        return n
    }
    return fibonacci(n-1) + fibonacci(n-2)
}

// Option and Result types
def divide(a: int, b: int) -> Maybe<int> {
    if (b == 0) { return None }
    return Some(a / b)
}

// Control flow
if (x > 0) {
    print("Positive")
} else {
    print("Non-positive")
}

while (x > 0) {
    x = x - 1
}
```

### Planned Advanced Features
```rust
// Safe pointer types
buffer unique Buffer = new Buffer(1024)
conn shared Connection = share(connection)

// Pattern matching
match divide(10, 2) {
    Some(result) => print("Result: " + result)
    None => print("Division by zero!")
}

// Modules and C interop
extern "C" fn malloc(size: int) -> *void
import std.io
```

## 🏗️ Project Structure

```
forge/
├── src/                    # Source code
│   ├── lexer/             # Lexical analysis
│   ├── parser/            # Syntax analysis (planned)
│   ├── compiler/          # Bytecode generation (planned)
│   ├── vm/                # Virtual machine (planned)
│   └── main.cpp           # Entry point (planned)
├── include/               # Header files
├── tests/                 # Comprehensive test suite
├── examples/              # Sample Forge programs (planned)
├── docs/                  # Documentation
├── Makefile              # Build configuration
└── README.md             # This file
```

## 🎯 Current Status

- ✅ **Token System** - Complete with 24 token types
- ✅ **Testing Framework** - Google Test integration
- ✅ **Build System** - Makefile with proper linking
- ✅ **Project Structure** - Clean organization
- 🚧 **Lexer** - In development
- ⏳ **Parser** - Planned
- ⏳ **Compiler** - Planned
- ⏳ **Virtual Machine** - Planned

## 🎓 Educational Goals

This project serves as a comprehensive learning experience in:
- **Compiler Design** - Traditional compilation pipeline
- **C++ Development** - Modern C++17 practices
- **Software Architecture** - Clean separation of concerns
- **Test-Driven Development** - Comprehensive testing strategy
- **Language Design** - Syntax and semantics decisions

## 📚 Resources

- [Crafting Interpreters](https://craftinginterpreters.com/) by Robert Nystrom
- [Engineering a Compiler](https://www.elsevier.com/books/engineering-a-compiler/cooper/978-0-12-088478-0) by Cooper & Torczon
- [Modern Compiler Implementation](https://www.cs.princeton.edu/~appel/modern/) by Andrew Appel

## 🤝 Contributing

This is currently an educational project. Feel free to:
- Open issues for discussion
- Suggest language features
- Propose architectural improvements
- Submit educational resources

## 📄 License

This project is open source. See the LICENSE file for details.

---

**Built with ❤️ as a learning journey into compiler construction**