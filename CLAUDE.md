# Forge Programming Language

## Project Overview
Forge is a Python-like programming language with strong typing, designed for ease of use and performance. The development follows a traditional compiler pipeline approach, starting with a bytecode virtual machine.

## Development Phases
1. **Phase 1: Bytecode VM** - Current focus
2. **Phase 2: JIT Compilation** - Future enhancement
3. **Phase 3: Direct Compilation** - Long-term goal

## Architecture Components

### Current Implementation (VM-based)
- **Lexer**: Tokenizes source code into lexical units
- **Parser**: Builds Abstract Syntax Tree (AST) from tokens
- **Bytecode Compiler**: Converts AST to bytecode instructions
- **Virtual Machine**: Executes bytecode with stack-based operations

### Language Features (Planned)
- Python-like syntax for familiarity
- Strong static typing for performance and safety
- First-class functions
- Object-oriented programming support
- Memory management (garbage collection)

## Build Commands
```bash
# Build the project
make build

# Run tests
make test

# Build and run example
make run

# Clean build artifacts
make clean
```

## Development Guidelines

### Code Style
- Use modern C++17 features
- Follow RAII principles
- Prefer smart pointers over raw pointers
- Use descriptive variable and function names
- Keep functions focused and small

### Testing Strategy
- Unit tests for each component
- Integration tests for the full pipeline
- Example programs to validate language features

### Educational Approach
This project emphasizes learning compiler/interpreter construction:
- Each component is built incrementally
- Clear separation of concerns between phases
- Extensive documentation of design decisions
- Step-by-step implementation with explanations

## Project Structure
```
forge/
├── src/                    # Source code
│   ├── lexer/             # Lexical analysis
│   ├── parser/            # Syntax analysis
│   ├── compiler/          # Bytecode generation
│   ├── vm/                # Virtual machine
│   └── main.cpp           # Entry point
├── include/               # Header files
├── tests/                 # Test suite
├── examples/              # Sample Forge programs
├── docs/                  # Documentation
└── Makefile              # Build configuration
```

## Learning Resources
- "Crafting Interpreters" by Robert Nystrom
- "Engineering a Compiler" by Cooper & Torczon
- "Modern Compiler Implementation" by Appel

## Current Focus
Building the lexer component to understand tokenization fundamentals.