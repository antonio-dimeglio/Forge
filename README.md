# Forge Programming Language

> A Python-like programming language with strong typing, built from the ground up in C++

## 🚀 Overview

Forge is an educational programming language project that combines the simplicity of Python syntax with the safety of strong static typing. The project follows a traditional compiler architecture, starting with a bytecode virtual machine and evolving toward JIT compilation and direct compilation.

## ✨ Features

### Current Implementation
- **Complete Token System** - Comprehensive lexical analysis foundation
- **Strong Type System** - Static typing for performance and safety
- **Python-like Syntax** - Familiar and clean language design
- **Professional Testing** - Google Test framework with comprehensive coverage

### Planned Features
- **Control Flow** - `if`, `else`, `while`, `for` statements
- **Functions** - First-class function support with `def` and `return`
- **Type Declarations** - Built-in types: `int`, `str`, `bool`, `float`, `double`
- **Object-Oriented** - Classes and inheritance (future)
- **Memory Management** - Garbage collection (future)

## 🏗️ Architecture

Forge follows a multi-phase compilation pipeline:

```
Source Code → Lexer → Parser → Bytecode Compiler → Virtual Machine
```

### Development Phases
1. **Phase 1: Bytecode VM** *(Current)*
   - Lexical analysis ✅
   - Syntax analysis *(In Progress)*
   - Bytecode generation *(Planned)*
   - Virtual machine *(Planned)*

2. **Phase 2: JIT Compilation** *(Future)*
   - Just-in-time compilation for performance

3. **Phase 3: Direct Compilation** *(Long-term)*
   - Direct machine code generation

## 🛠️ Building

### Prerequisites
- C++17 compatible compiler (GCC 7+ or Clang 5+)
- Google Test library
- Make

### Ubuntu/Debian Setup
```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y build-essential libgtest-dev libgmock-dev

# Clone and build
git clone https://github.com/yourusername/forge.git
cd forge
make
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

## 📝 Language Syntax (Planned)

```python
# Variables with type inference
x: int = 42
name: str = "Alice"
active: bool = true

# Control flow
if x > 0:
    print("Positive number")
else:
    print("Non-positive")

# Functions
def fibonacci(n: int) -> int:
    if n <= 1:
        return n
    return fibonacci(n-1) + fibonacci(n-2)

# Loops
for i in range(10):
    print(i)
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