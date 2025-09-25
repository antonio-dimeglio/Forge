# Forge Class System Implementation Roadmap

## Overview
This document defines the complete implementation plan for Forge's class system, including syntax design, implementation phases, and technical requirements.

## Class System Design Decisions

### Core Features
- **Single inheritance** (one base class maximum)
- **Constructor overloading** support
- **No auto-generated default constructors** (explicit user control)
- **Access control**: `private`, `public`, `protected` keywords
- **No friend classes**
- **Method dispatch**: To be determined during implementation
- **Destructors**: To be determined based on memory management needs

### Memory Management
- **Heap allocation** for class instances
- **Smart pointers**: `unique_ptr[T]` and `shared_ptr[T]`
- **RAII** principles for resource management

### Generics Strategy
- **Monomorphization** (concrete type generation per instantiation)
- **Compile-time specialization** for performance
- Future: Trait bounds for generic constraints (post-iterables implementation)

## Target Syntax Examples

### Basic Class Definition
```forge
class Point {
    public x: int
    public y: int
    private id: int

    # Multiple constructors supported
    def Point(x: int, y: int) -> Point {
        self.x = x
        self.y = y
        self.id = generate_id()
    }

    def Point(other: Point) -> Point {  # Copy constructor
        self.x = other.x
        self.y = other.y
        self.id = generate_id()
    }

    public def distance(other: Point) -> double {
        return sqrt((self.x - other.x)^2 + (self.y - other.y)^2)
    }

    private def generate_id() -> int {
        # Implementation
    }
}
```

### Inheritance
```forge
class Shape {
    protected name: str

    def Shape(name: str) -> Shape {
        self.name = name
    }

    public def get_name() -> str {
        return self.name
    }

    # Virtual method (exact syntax TBD)
    public virtual def area() -> double {
        return 0.0
    }
}

class Circle extends Shape {
    private radius: double

    def Circle(radius: double) -> Circle {
        super("Circle")  # Call parent constructor
        self.radius = radius
    }

    public override def area() -> double {
        return 3.14159 * self.radius * self.radius
    }
}
```

### Generic Classes
```forge
class Array[T] {
    private data: *T      # Raw pointer to heap data
    private size: int
    private capacity: int

    def Array(initial_capacity: int) -> Array[T] {
        self.data = allocate[T](initial_capacity)
        self.size = 0
        self.capacity = initial_capacity
    }

    public def get(index: int) -> T {
        return self.data[index]
    }

    public def set(index: int, value: T) -> void {
        self.data[index] = value
    }
}

# Usage
arr: Array[int] = Array[int](10)
arr.set(0, 42)
value: int = arr.get(0)
```

### Smart Pointers Integration
```forge
# Unique ownership
player: unique_ptr[Player] = unique_ptr[Player](Player("Alice", 100))

# Shared ownership
config: shared_ptr[GameConfig] = shared_ptr[GameConfig](GameConfig())
renderer: GameRenderer = GameRenderer(config)  # Shares ownership
ui: GameUI = GameUI(config)                    # Shares ownership
```

## Implementation Roadmap

### Phase 1: Access Control Keywords (Prerequisite)
**Status**: Not Started

#### Lexer Changes
- [ ] Add `private`, `public`, `protected` keywords to TokenType enum
- [ ] Update tokenizer to recognize access control keywords
- [ ] Add tests for access control keyword tokenization

#### Parser Changes
- [ ] Extend field/method parsing to handle access modifiers
- [ ] Update AST nodes (FieldDefinition, MethodDefinition) with access level
- [ ] Add validation for access modifier placement
- [ ] Add tests for access control parsing

### Phase 2: Inheritance Syntax (Prerequisite)
**Status**: Not Started

#### Lexer Changes
- [ ] Add `extends`, `super`, `virtual`, `override` keywords
- [ ] Add tests for inheritance keyword tokenization

#### Parser Changes
- [ ] Extend ClassDefinition to support base class specification
- [ ] Add parsing for `extends BaseClass` syntax
- [ ] Add parsing for `super()` constructor calls
- [ ] Add parsing for `virtual`/`override` method modifiers
- [ ] Update AST nodes for inheritance information
- [ ] Add comprehensive inheritance parsing tests

### Phase 3: Constructor Overloading Support
**Status**: Partially Complete (basic constructor parsing exists)

#### Parser Enhancements
- [ ] Modify parseMethodDefinition to detect constructors (method name == class name)
- [ ] Add constructor-specific validation (return type must match class)
- [ ] Support multiple constructors with different parameter signatures
- [ ] Add constructor overloading tests

#### AST Updates
- [ ] Add ConstructorDefinition AST node (distinct from MethodDefinition)
- [ ] Track constructor overloads per class
- [ ] Add constructor resolution logic

### Phase 4: LLVM Type System Integration
**Status**: Not Started

#### Class Type Management
- [ ] Extend LLVMTypeSystem to handle class types
- [ ] Implement class type registration and lookup
- [ ] Add field offset calculation for memory layout
- [ ] Create LLVM struct types for class representations

#### Memory Layout Design
- [ ] Define object header structure (type info, vtable pointer, etc.)
- [ ] Implement field layout algorithm (considering inheritance)
- [ ] Add alignment and padding calculations
- [ ] Support for vtable pointer injection

### Phase 5: Object Instantiation & Construction
**Status**: Not Started

#### Compiler Integration
- [ ] Implement compileObjectInstantiation in LLVMCompiler
- [ ] Add heap allocation logic for class instances
- [ ] Implement constructor call resolution and invocation
- [ ] Add constructor overload resolution based on argument types

#### Memory Management
- [ ] Integrate with smart pointer allocation
- [ ] Add proper object initialization sequences
- [ ] Implement constructor chaining (super() calls)

### Phase 6: Method Dispatch Implementation
**Status**: Not Started

#### Static Method Calls
- [ ] Implement direct method invocation for non-virtual methods
- [ ] Add method signature resolution
- [ ] Support for overloaded method resolution

#### Virtual Method Support (Future)
- [ ] Implement vtable generation for classes with virtual methods
- [ ] Add dynamic method dispatch logic
- [ ] Support for method overriding validation

### Phase 7: Inheritance Implementation
**Status**: Not Started

#### Type System Updates
- [ ] Add inheritance relationship tracking
- [ ] Implement base class field/method inheritance
- [ ] Add access control enforcement (private/protected/public)
- [ ] Support for method overriding validation

#### Memory Layout for Inheritance
- [ ] Implement base class field layout first
- [ ] Add derived class field appending
- [ ] Ensure proper vtable inheritance
- [ ] Add casting support (derived to base)

### Phase 8: Generic Class Implementation
**Status**: Not Started

#### Template System
- [ ] Implement monomorphization for generic classes
- [ ] Add generic type parameter substitution
- [ ] Create specialized types for each instantiation
- [ ] Add generic constraint validation (future)

#### Compiler Support
- [ ] Implement compileGenericInstantiation
- [ ] Add template instantiation caching
- [ ] Support for nested generic types
- [ ] Add generic method resolution

### Phase 9: Field Access & Method Calls
**Status**: Partially Complete (parsing exists)

#### Code Generation
- [ ] Implement field access code generation
- [ ] Add bounds checking for field access
- [ ] Implement method call code generation
- [ ] Add access control enforcement at compile time

#### Memory Access
- [ ] Calculate field offsets correctly
- [ ] Handle inheritance field access
- [ ] Implement proper `self` parameter passing
- [ ] Add null pointer checking (with smart pointers)

## Testing Strategy

### Unit Tests Required
- [ ] Access control keyword parsing tests
- [ ] Inheritance syntax parsing tests
- [ ] Constructor overloading tests
- [ ] Class type system tests
- [ ] Object instantiation tests
- [ ] Method dispatch tests
- [ ] Field access tests
- [ ] Generic class instantiation tests
- [ ] Inheritance behavior tests

### Integration Tests Required
- [ ] Complete class definition → LLVM IR generation
- [ ] Inheritance hierarchies compilation
- [ ] Generic class usage across multiple types
- [ ] Smart pointer integration with classes
- [ ] Access control enforcement tests

## Success Criteria

### Phase Completion Criteria
Each phase is complete when:
1. All listed tasks are implemented
2. Unit tests pass for the phase
3. Integration with existing codebase is verified
4. No regressions in existing functionality

### Final Success Criteria
The class system is complete when:
1. All target syntax examples compile and run correctly
2. Full test suite passes (targeting 300+ tests total)
3. LLVM IR generation is clean and efficient
4. Memory management integrates properly with smart pointers
5. Performance benchmarks meet expectations

## Notes

### Dependencies
- **Smart Pointers**: Must be implemented before Phase 4
- **Iterables/For Loops**: Can be implemented after basic classes
- **Trait System**: Future enhancement, not required for initial implementation

### Future Enhancements
- Abstract classes and interfaces
- Trait bounds for generics
- Multiple inheritance (if needed)
- Compile-time reflection
- Attribute/annotation system

---

**Document Status**: Draft - Ready for pointer system design
**Last Updated**: 2025-09-25
**Next Steps**: Implement smart pointer system, then begin Phase 1