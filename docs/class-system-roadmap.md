# Forge Class System Implementation Roadmap

## Overview
This document defines the complete implementation plan for Forge's class system, building on the existing design while adding inheritance, access control, memory management integration, and advanced OOP features. The class system emphasizes zero-cost abstractions and seamless integration with Forge's smart pointer and borrowing systems.

## Design Philosophy

### Core Principles
- **Zero-cost abstractions**: No runtime overhead for unused features
- **Memory flexibility**: Stack allocation by default, explicit heap allocation with `new`
- **Type safety**: Compile-time method resolution and type checking
- **Explicit control**: Clear syntax for virtual dispatch, access control, and ownership
- **Smart pointer integration**: Classes work seamlessly with `unique`, `shared`, and borrowing
- **No lifetime annotations**: Borrowing without Rust-style `<'a>` complexity

## Target Syntax Examples

### Basic Class Definition
```forge
class Player {
    // Fields with access control
    public name: str
    private health: int
    protected level: int

    // Constructor (required)
    def __init__(name: str, starting_health: int) {
        self.name = name
        self.health = starting_health
        self.level = 1
    }

    // Regular methods (static dispatch)
    public def get_health() -> int {
        return self.health
    }

    // Virtual methods (dynamic dispatch for inheritance)
    public virtual def attack() -> int {
        return self.level * 10
    }

    // Static/class methods
    public static def max_health() -> int {
        return 1000
    }

    // Method overloading
    public def heal(amount: int) -> void {
        self.health = self.health + amount
    }

    public def heal(potion: HealthPotion) -> void {
        self.health = self.health + potion.healing_power
    }

    // Private helper methods
    private def validate_health() -> bool {
        return self.health >= 0 && self.health <= Player.max_health()
    }
}
```

### Memory Management Integration
```forge
// Stack allocation (default)
player1: Player = Player("Alice", 100)      // Lives on stack
player1.attack()                            // Direct method call

// Heap allocation with smart pointers
player2: unique Player = new Player("Bob", 150)    // Heap via unique_ptr
player3: shared Player = new Player("Carol", 200)  // Heap via shared_ptr

// Method calls work the same regardless of allocation
damage1: int = player2.attack()             // Automatic dereferencing
damage2: int = player3.attack()             // Automatic dereferencing

// Borrowing from classes (simplified - no lifetime annotations)
health_ref: &int = &player1.health          // Borrow field from stack object
name_ref: &str = &player2.name              // Borrow field from unique_ptr
```

### Inheritance and Polymorphism
```forge
// Base class with virtual methods
class Animal {
    protected name: str

    def __init__(name: str) {
        self.name = name
    }

    // Virtual method for polymorphism
    public virtual def speak() -> str {
        return "Some animal sound"
    }

    // Non-virtual method (static dispatch)
    public def get_name() -> str {
        return self.name
    }
}

// Derived class
class Dog extends Animal {
    private breed: str

    def __init__(name: str, breed: str) {
        super.__init__(name)                 // Call parent constructor
        self.breed = breed
    }

    // Override virtual method
    public virtual def speak() -> str {
        return "Woof! I'm " + self.name
    }

    // Additional methods
    public def get_breed() -> str {
        return self.breed
    }
}

// Abstract classes
abstract class Shape {
    // Abstract method - must be implemented by subclasses
    public abstract def area() -> float

    // Concrete method - can be used by subclasses
    public def describe() -> str {
        return "A shape with area " + str(self.area())
    }
}

class Circle extends Shape {
    private radius: float

    def __init__(radius: float) {
        self.radius = radius
    }

    // Must implement abstract method
    public def area() -> float {
        return 3.14159 * self.radius * self.radius
    }
}
```

### Polymorphism and Dynamic Dispatch
```forge
// Polymorphic usage
animals: [shared Animal; 3] = [
    new Dog("Rex", "Golden Retriever"),
    new Cat("Whiskers", "Persian"),
    new Bird("Tweety", "Canary")
]

// Virtual method calls (dynamic dispatch)
for animal in animals {
    sound: str = animal.speak()             // Calls correct overridden method
    print(sound)
}

// Non-virtual method calls (static dispatch - faster)
for animal in animals {
    name: str = animal.get_name()           // Direct call, no vtable lookup
    print(name)
}
```

### Generic Classes (Enhanced)
```forge
class Container[T] {
    private items: [T; 10]
    private count: int

    def __init__() {
        self.count = 0
    }

    public def add(item: T) -> bool {
        if self.count >= 10 {
            return false
        }
        self.items[self.count] = item
        self.count = self.count + 1
        return true
    }

    public def get(index: int) -> &T {       // Return reference to avoid copy
        return &self.items[index]
    }

    // Static method with generics
    public static def create_with_capacity(capacity: int) -> Container[T] {
        // Implementation for dynamic sizing
    }
}

// Usage with different allocation strategies
stack_container: Container[int] = Container[int]()           // Stack allocated
heap_container: unique Container[str] = new Container[str]() // Heap allocated
```

### Method Overloading Examples
```forge
class Math {
    // Overloaded static methods
    public static def max(a: int, b: int) -> int {
        return if a > b { a } else { b }
    }

    public static def max(a: float, b: float) -> float {
        return if a > b { a } else { b }
    }

    public static def max(a: double, b: double) -> double {
        return if a > b { a } else { b }
    }
}

class Vector {
    public x: float
    public y: float

    def __init__(x: float, y: float) {
        self.x = x
        self.y = y
    }

    // Overloaded instance methods
    public def distance() -> float {
        return sqrt(self.x * self.x + self.y * self.y)
    }

    public def distance(other: Vector) -> float {
        dx: float = self.x - other.x
        dy: float = self.y - other.y
        return sqrt(dx * dx + dy * dy)
    }
}
```

### Integration with Borrowing System (Simplified)
```forge
class GameState {
    public players: [Player; 4]
    public current_turn: int

    def __init__() {
        self.current_turn = 0
        // Initialize players
    }

    // Methods that work with borrowing
    public def get_current_player() -> &Player {
        return &self.players[self.current_turn]
    }

    public def get_current_player_mut() -> &mut Player {
        return &mut self.players[self.current_turn]
    }
}

// Usage with borrowing (no lifetime annotations needed)
game: GameState = GameState()
current_player: &Player = game.get_current_player()        // Immutable borrow
player_name: str = current_player.name                     // Access through borrow

// Mutable borrowing
current_player_mut: &mut Player = game.get_current_player_mut()
current_player_mut.health = 50                             // Modify through borrow
```

## Implementation Roadmap

### Phase 1: Core Class Foundation
**Status**: Partially Complete (Basic syntax exists)
**Estimated Time**: 2-3 weeks

#### Lexer/Parser Extensions
- [X] Basic `class` keyword and syntax
- [ ] Access control keywords: `public`, `private`, `protected`
- [ ] Inheritance keywords: `extends`, `abstract`, `virtual`
- [ ] Static method keyword: `static`
- [ ] Super call syntax: `super.method()` or `super.__init__()`

#### AST Enhancements
- [X] Basic ClassDefinition node
- [ ] Access modifier support in fields and methods
- [ ] Inheritance relationship tracking
- [ ] Virtual method marking
- [ ] Static method distinction
- [ ] Method overloading resolution

#### Type System Updates
- [ ] Class type hierarchy tracking
- [ ] Inheritance compatibility checking
- [ ] Access control validation
- [ ] Method signature comparison for overloading
- [ ] Abstract class validation

### Phase 2: Memory Management Integration
**Status**: Not Started
**Estimated Time**: 2-3 weeks

#### Stack vs Heap Allocation
- [ ] Default stack allocation for class instances
- [ ] `new` keyword integration for heap allocation
- [ ] Smart pointer compatibility (`unique`, `shared`)
- [ ] Automatic dereferencing for smart pointer method calls

#### Constructor Integration
- [ ] Stack constructor calls: `Player("Alice", 100)`
- [ ] Heap constructor calls: `new Player("Alice", 100)`
- [ ] Constructor chaining with `super.__init__()`
- [ ] Memory layout optimization for inheritance

#### LLVM Code Generation
- [ ] Struct layout generation for classes
- [ ] Constructor function generation
- [ ] Method table setup for virtual dispatch
- [ ] Static vs dynamic dispatch implementation

### Phase 3: Inheritance and Polymorphism
**Status**: Not Started
**Estimated Time**: 3-4 weeks

#### Single Inheritance Implementation
- [ ] Class hierarchy validation
- [ ] Field and method inheritance
- [ ] Method overriding rules
- [ ] Access control in inheritance

#### Virtual Dispatch System
- [ ] Virtual method table (vtable) generation
- [ ] Virtual method call resolution
- [ ] Performance optimization for non-virtual calls
- [ ] Abstract class and method enforcement

#### Super Calls and Constructor Chaining
- [ ] `super.method()` call generation
- [ ] Constructor chaining validation
- [ ] Initialization order enforcement

### Phase 4: Advanced Features
**Status**: Not Started
**Estimated Time**: 2-3 weeks

#### Method Overloading
- [ ] Compile-time overload resolution
- [ ] Type-based method selection
- [ ] Ambiguity detection and error reporting
- [ ] Integration with inheritance

#### Static Methods and Class-Level Features
- [ ] Static method compilation
- [ ] Class-level variable support (future)
- [ ] Static initialization (future)

#### Generic Class Enhancements
- [ ] Inheritance with generics
- [ ] Generic constraint validation
- [ ] Template specialization optimization

### Phase 5: Borrowing System Integration
**Status**: Not Started
**Estimated Time**: 2-3 weeks

#### Field Borrowing
- [ ] Direct field borrowing: `&object.field`
- [ ] Mutable field borrowing: `&mut object.field`
- [ ] Borrow conflict detection for fields
- [ ] Method calls during active borrows

#### Method Borrowing Integration
- [ ] Methods accepting borrowed parameters
- [ ] Methods returning borrowed references
- [ ] Borrow checker integration with method calls
- [ ] Lifetime inference without explicit annotations

#### Smart Pointer Borrowing
- [ ] Borrowing from `unique` class instances
- [ ] Borrowing from `shared` class instances
- [ ] Borrow validation across smart pointer boundaries

### Phase 6: Optimization and Polish
**Status**: Not Started
**Estimated Time**: 1-2 weeks

#### Performance Optimizations
- [ ] Virtual call optimization
- [ ] Inline method calls where possible
- [ ] Dead code elimination for unused methods
- [ ] Memory layout optimization

#### Developer Experience
- [ ] Comprehensive error messages for class-related errors
- [ ] Debugging support for class instances
- [ ] IDE-friendly type information export

## Memory Layout and Implementation Details

### Class Instance Layout
```forge
class Player {
    public name: str      // Offset 0
    private health: int   // Offset 8 (assuming 64-bit str)
    protected level: int  // Offset 12
}

// LLVM representation
%Player = type {
    %str,    ; name (offset 0)
    i32,     ; health (offset 8)
    i32      ; level (offset 12)
}
```

### Virtual Method Tables
```forge
class Animal {
    virtual def speak() -> str
    def get_name() -> str
}

// Generated vtable
%Animal_vtable = type {
    ptr,  ; speak function pointer
    ptr   ; RTTI information (optional)
}

// Instance layout with vtable
%Animal = type {
    ptr,     ; vtable pointer
    %str     ; name field
}
```

### Inheritance Layout
```forge
class Dog extends Animal {
    private breed: str
}

// Memory layout (compatible with Animal)
%Dog = type {
    ptr,     ; vtable pointer (Animal-compatible)
    %str,    ; name field (inherited)
    %str     ; breed field (Dog-specific)
}
```

## Integration with Existing Systems

### Smart Pointer Compatibility
- Class instances work seamlessly with `unique`, `shared`, and weak pointers
- Automatic dereferencing for method calls on smart pointers
- Reference counting integration for shared class instances

### Borrowing System Integration
- Field-level borrowing without lifetime annotations
- Automatic lifetime inference based on scope
- Borrow checker validation for method calls and field access
- No Rust-style `<'a>` syntax complexity

### Type System Integration
- Classes integrate with existing type checking
- Generic classes work with type inference
- Method overloading resolution at compile time

## Success Criteria

### Phase Completion Criteria
Each phase is complete when:
1. All listed features are implemented and tested
2. Integration tests pass with existing systems
3. Performance benchmarks meet zero-cost abstraction goals
4. Error messages provide clear guidance to developers

### Final Success Criteria
The class system is complete when:
1. All target syntax examples compile and run correctly
2. Full test suite passes (targeting 200+ class-specific tests)
3. Zero runtime overhead for non-virtual method calls
4. Seamless integration with smart pointers and borrowing
5. Inheritance works correctly with memory management
6. Method overloading resolves correctly at compile time

## Performance Targets

### Zero-Cost Abstractions
- **Non-virtual method calls**: Same performance as C function calls
- **Field access**: Direct memory access, no indirection
- **Static methods**: Identical to regular function calls
- **Generic instantiation**: Monomorphization, no runtime type checks

### Acceptable Overhead
- **Virtual method calls**: <5% overhead vs function pointers
- **Inheritance**: <2% memory overhead for vtable pointers
- **Constructor calls**: <3% overhead vs manual initialization

## Testing Strategy

### Unit Tests Required
- [ ] Class definition and parsing tests (30+ tests)
- [ ] Inheritance and polymorphism tests (40+ tests)
- [ ] Method overloading tests (25+ tests)
- [ ] Access control tests (20+ tests)
- [ ] Memory management integration tests (35+ tests)
- [ ] Borrowing system integration tests (30+ tests)
- [ ] Generic class tests (25+ tests)
- [ ] Virtual dispatch tests (20+ tests)

### Integration Tests Required
- [ ] Class + smart pointer integration
- [ ] Class + borrowing system integration
- [ ] Class + generic system integration
- [ ] Cross-phase feature interaction
- [ ] Performance benchmarking vs manual code

### Memory Safety Tests
- [ ] Constructor/destructor correctness
- [ ] Inheritance memory layout validation
- [ ] Virtual dispatch safety
- [ ] Borrow checking with class instances
- [ ] Smart pointer lifecycle with classes

## Dependencies & Prerequisites

### Required Before Starting
- [X] Basic type system functioning
- [X] Function calling mechanism working
- [X] Smart pointer system complete
- [ ] Borrowing system foundation (Phase 7 from pointer roadmap)

### Blocks Other Systems
- Generic system enhancements depend on class generics
- Module system should integrate with class access control
- Pattern matching (future) should work with class instances

---

**Document Status**: Ready for Implementation
**Last Updated**: 2025-09-27
**Estimated Timeline**: 12-15 weeks for full implementation
**Next Steps**: Begin Phase 1 - Core Class Foundation