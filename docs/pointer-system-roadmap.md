# Forge Pointer System Implementation Roadmap

## Overview
This document defines the complete implementation plan for Forge's pointer system, including raw pointers, smart pointers, borrowing system, and memory management integration.

## Pointer System Design Decisions

### Core Features
- **Raw pointers** (`*T`) for C interop and manual memory management
- **Smart pointers**: `unique`, `shared`, and `weak` with RAII principles
- **Borrowing system**: Immutable (`&T`) and mutable (`&mut T`) references
- **Optional wrapper integration** for null safety
- **Move semantics** with `move` keyword
- **Automatic dereferencing** for smart pointers
- **Heap allocation** for all smart pointer types
- **Separate array types** (not smart pointer to arrays)

### Memory Management Strategy
- **RAII (Resource Acquisition Is Initialization)** for automatic cleanup
- **Reference counting** for shared pointers
- **Weak references** to prevent circular dependencies
- **Move semantics** for zero-cost ownership transfer
- **Borrowing** for safe temporary access without ownership transfer

## Target Syntax Examples

### Raw Pointers & C Interop
```forge
# Raw pointer types
raw_int: *int = malloc(sizeof(int))
raw_void: *void = null
raw_array: *Player = malloc(sizeof(Player) * 10)

# C function bindings
extern def malloc(size: int) -> *void
extern def free(ptr: *void) -> void
extern def memcpy(dest: *void, src: *void, n: int) -> *void

# Manual memory management
data: *int = malloc(sizeof(int) * 100)
data[0] = 42                    # Pointer arithmetic
data[99] = 100
defer free(data)                # Manual cleanup

# Null checking
if data != null {
    process_data(data)
}
```

### Smart Pointers - Unique Ownership
```forge
# Creation syntax
player: unique Player := new Player("Alice", 100)
weapon: unique Weapon := new Weapon("Sword", 50)

# Automatic dereferencing
player.attack()                 # No * or -> needed
player.health = 90             # Direct field access
weapon_name: str = weapon.name  # Direct field access

# Move semantics
player2: unique Player := move player    # player becomes invalid
# player.health = 50  <- Compile error: moved value

# Conversion to shared
shared_player: shared Player := player2.to_shared()

# Reset/clear
weapon.reset()                  # weapon becomes null
weapon = none                   # Alternative syntax

# Null checking with optional
maybe_player: optional[unique Player] = none
maybe_player = some(unique Player := new Player("Bob", 80))

if maybe_player.is_some() {
    actual_player := maybe_player.unwrap()
    actual_player.defend()
}
```

### Smart Pointers - Shared Ownership
```forge
# Creation and sharing
config: shared GameConfig := new GameConfig()
renderer: GameRenderer := new GameRenderer(config)  # Shares ownership
ui: GameUI := new GameUI(config)                    # Shares ownership

# Reference counting
count: int = config.ref_count()  # Returns current reference count

# Weak references to break cycles
parent: shared Node := new Node("parent")
child: shared Node := new Node("child")
parent.add_child(child)

# Create weak reference to prevent cycle
child_weak: weak Node := child.downgrade()

# Access weak reference (returns optional)
maybe_child: optional[shared Node] = child_weak.upgrade()
if maybe_child.is_some() {
    actual_child := maybe_child.unwrap()
    actual_child.process()
}
```

### Borrowing System
```forge
player: unique Player := new Player("Alice", 100)

# Immutable borrowing
name_ref: &str = &player.name           # Borrow name field
health_ref: &int = &player.health       # Borrow health field
print("Player: " + name_ref + ", HP: " + str(health_ref))

# Mutable borrowing
health_mut: &mut int = &mut player.health
*health_mut = 90                        # Explicit dereference for mutation

# Function parameters with borrowing
def heal_player(player_health: &mut int, amount: int) -> void {
    *player_health = *player_health + amount
}

heal_player(&mut player.health, 10)

# Borrowing from shared pointers
shared_player: shared Player := new Player("Bob", 80)
shared_name: &str = &shared_player.name  # Borrow from shared pointer

# Reference lifetime validation (compile-time)
def get_name_ref(p: &Player) -> &str {
    return &p.name  # OK: lifetime of return tied to parameter
}

invalid_ref: &str = get_name_ref(&player)  # Compile-time lifetime checking
```

### Advanced Pointer Operations
```forge
# Pointer arithmetic (raw pointers only)
array: *int = malloc(sizeof(int) * 10)
ptr: *int = array
ptr = ptr + 1                   # Move to next element
*ptr = 42                       # Assign value
ptr[2] = 100                    # Array-style access

# Pointer comparison
ptr1: *int = &some_int
ptr2: *int = &other_int
same_address: bool = (ptr1 == ptr2)     # Address comparison

# Smart pointer comparison
player1: unique Player := new Player("Alice", 100)
player2: unique Player := new Player("Alice", 100)
same_identity: bool = player1.same_as(&player2)  # false (different objects)
same_content: bool = (player1 == player2)        # true (same data, if == implemented)

# Type casting (unsafe)
void_ptr: *void = ptr1 as *void
int_ptr: *int = void_ptr as *int
```

## Implementation Roadmap

### Phase 1: Raw Pointer Foundation
**Status**: ✅ COMPLETED - All lexer tokens, parser functionality, AST nodes, and comprehensive tests (58 total) implemented and passing

#### Lexer Changes
- [X] Add `*` token type for pointer declaration (uses existing MULT token)
- [X] Add `&`, `&mut` tokens for borrowing (RAW_PTR, IMM_REF, MUT_REF)
- [X] Add `move` keyword (MOVE token)
- [X] Add `null` keyword for null pointers (NULL_ token)
- [X] Add `defer` keyword for cleanup (DEFER token)
- [X] Add `extern` keyword for C function bindings (EXTERN token)
- [X] Add `:=` operator for type inference (INFER_ASSIGN token)
- [X] Add tests for all new pointer-related tokens (32 comprehensive tests)

#### Parser Changes
- [X] Add pointer type parsing (`*T`, `&T`, `&mut T`)
- [X] Add pointer declaration syntax (`ptr: *int = malloc(4)`)
- [X] Add pointer arithmetic parsing (`ptr + 1`, `ptr[index]`) - handled by existing BinaryExpression
- [X] Add address-of operator parsing (`&variable`)
- [X] Add dereference operator parsing (`*pointer`)
- [X] Add `move` expression parsing (`move variable`)
- [X] Add `defer` statement parsing (`defer cleanup()`)
- [X] Add `extern` function declaration parsing (`extern def malloc(...)`)
- [X] Add comprehensive pointer syntax tests (26 parser tests + 32 lexer tests = 58 total)

#### AST Updates
- [X] Add PointerType support via ParsedType with isPointer flag (`*T`)
- [X] Add ReferenceType support via ParsedType with isReference flags (`&T`, `&mut T`)
- [X] Pointer arithmetic handled by existing BinaryExpression (`ptr + offset`)
- [X] Add AddressOfExpression via UnaryExpression with & operator (`&variable`)
- [X] Add DereferenceExpression via UnaryExpression with * operator (`*pointer`)
- [X] Add MoveExpression AST node (`move variable`)
- [X] Add DeferStatement AST node (`defer cleanup()`)
- [X] Add ExternStatement AST node (`extern def malloc(...)`)

**Implementation Note**: Our design uses existing AST nodes (UnaryExpression, BinaryExpression, ParsedType with flags) instead of creating separate pointer-specific nodes. This approach is cleaner, reduces code duplication, and maintains semantic correctness while being easier to maintain.

### Phase 2: Smart Pointer Keywords & Syntax
**Status**: Not Started

#### Lexer Changes
- [ ] Add `unique`, `shared`, `weak` keywords
- [ ] Add `some`, `none` keywords for optional types
- [ ] Add tests for smart pointer keyword tokenization

#### Parser Changes
- [ ] Add smart pointer type parsing (`unique T`, `shared T`, `weak T`)
- [ ] Add smart pointer creation syntax (`unique T := new T()`)
- [ ] Add optional type parsing (`optional[T]`)
- [ ] Add `some`/`none` expression parsing
- [ ] Add method call parsing for smart pointer operations
- [ ] Add smart pointer conversion parsing (`to_shared()`, `downgrade()`, etc.)
- [ ] Add comprehensive smart pointer syntax tests

#### AST Updates
- [ ] Add UniquePointerType AST node
- [ ] Add SharedPointerType AST node
- [ ] Add WeakPointerType AST node
- [ ] Add OptionalType AST node
- [ ] Add SmartPointerCreation AST node
- [ ] Add SomeExpression and NoneExpression AST nodes

### Phase 3: LLVM Type System Integration
**Status**: Not Started

#### Type System Extensions
- [ ] Add raw pointer type support to LLVMTypeSystem
- [ ] Add smart pointer type representations
- [ ] Add reference type support
- [ ] Add optional type wrapping
- [ ] Implement type conversion between pointer types
- [ ] Add pointer type validation and compatibility checking

#### Memory Layout Design
- [ ] Define smart pointer internal structure (reference count, data pointer)
- [ ] Implement shared_ptr layout with atomic reference counting
- [ ] Define weak_ptr layout with weak reference tracking
- [ ] Add type metadata for runtime type checking

### Phase 4: Raw Pointer Implementation
**Status**: Not Started

#### Code Generation
- [ ] Implement raw pointer allocation (`malloc` integration)
- [ ] Add pointer arithmetic code generation
- [ ] Implement address-of operator (`&variable`)
- [ ] Implement dereference operator (`*pointer`)
- [ ] Add null pointer checking and validation
- [ ] Implement `defer` statement for cleanup

#### C Interop Support
- [ ] Add `extern` function declaration support
- [ ] Implement C function calling conventions
- [ ] Add type marshalling between Forge and C types
- [ ] Support for C header file integration

### Phase 5: Unique Pointer Implementation
**Status**: Not Started

#### Core Functionality
- [ ] Implement unique pointer creation and initialization
- [ ] Add automatic RAII destruction
- [ ] Implement move semantics for unique pointers
- [ ] Add reset/clear functionality
- [ ] Implement null checking methods
- [ ] Add conversion to shared pointer

#### Memory Management
- [ ] Integrate with heap allocation
- [ ] Implement proper destructor calls
- [ ] Add memory leak detection (debug builds)
- [ ] Ensure exception safety

### Phase 6: Shared Pointer Implementation
**Status**: Not Started

#### Reference Counting
- [ ] Implement atomic reference counting
- [ ] Add thread-safe increment/decrement operations
- [ ] Implement shared pointer creation and copying
- [ ] Add weak pointer downgrade functionality
- [ ] Implement reference count querying

#### Circular Reference Prevention
- [ ] Implement weak pointer upgrade mechanism
- [ ] Add weak reference invalidation on destruction
- [ ] Implement cycle detection (debug builds)
- [ ] Add weak pointer null checking

### Phase 7: Borrowing System Implementation
**Status**: Not Started

#### Borrow Checker (Compile-time)
- [ ] Implement lifetime analysis for references
- [ ] Add mutable/immutable borrow conflict detection
- [ ] Implement reference invalidation tracking
- [ ] Add lifetime parameter inference
- [ ] Implement borrow checker error messages

#### Code Generation
- [ ] Generate efficient reference passing (no runtime overhead)
- [ ] Implement automatic dereferencing for borrowed values
- [ ] Add bounds checking for reference access
- [ ] Ensure memory safety guarantees

### Phase 8: Optional Type Integration
**Status**: Not Started

#### Optional Wrapper
- [ ] Implement optional type creation and wrapping
- [ ] Add `some`/`none` value handling
- [ ] Implement `is_some()`, `is_none()` methods
- [ ] Add `unwrap()` with runtime safety checks
- [ ] Implement optional chaining methods

#### Pattern Matching Integration
- [ ] Add basic pattern matching for optional types
- [ ] Implement `match` expressions (basic version)
- [ ] Add compile-time exhaustiveness checking
- [ ] Support for nested optional unwrapping

### Phase 9: Advanced Features & Optimizations
**Status**: Not Started

#### Performance Optimizations
- [ ] Implement small object optimization for unique pointers
- [ ] Add compile-time null pointer elimination
- [ ] Optimize reference counting with intrusive pointers (advanced)
- [ ] Add custom allocator support

#### Debugging & Tooling
- [ ] Add memory leak detection tools
- [ ] Implement reference cycle detection
- [ ] Add pointer debugging information
- [ ] Create memory profiling hooks

## Testing Strategy

### Unit Tests Required
- [ ] Raw pointer operations tests (50+ tests)
- [ ] Smart pointer creation and destruction tests (30+ tests)
- [ ] Move semantics tests (20+ tests)
- [ ] Borrowing system tests (40+ tests)
- [ ] Optional type tests (25+ tests)
- [ ] Pointer arithmetic tests (15+ tests)
- [ ] C interop tests (20+ tests)
- [ ] Memory safety tests (30+ tests)

### Integration Tests Required
- [ ] Complete pointer type → LLVM IR generation
- [ ] Cross-pointer-type interactions
- [ ] Smart pointer + class system integration
- [ ] Memory leak prevention verification
- [ ] Performance benchmarking vs manual memory management

### Memory Safety Tests
- [ ] Use-after-free detection (compile-time)
- [ ] Double-free prevention
- [ ] Null pointer dereference prevention
- [ ] Dangling reference detection
- [ ] Memory leak prevention
- [ ] Thread safety tests (for shared pointers)

## Success Criteria

### Phase Completion Criteria
Each phase is complete when:
1. All listed tasks are implemented
2. Unit tests pass for the phase
3. Integration with existing codebase is verified
4. Memory safety guarantees are maintained
5. Performance benchmarks meet expectations

### Final Success Criteria
The pointer system is complete when:
1. All target syntax examples compile and run correctly
2. Full test suite passes (targeting 400+ tests total)
3. Memory safety is guaranteed at compile-time
4. C interop works seamlessly
5. Performance overhead is minimal (<5% compared to raw C)
6. Integration with class system works perfectly

## Memory Safety Guarantees

### Compile-time Guarantees
- **No use-after-free**: Borrow checker prevents accessing moved values
- **No double-free**: RAII ensures single deallocation
- **No dangling references**: Lifetime analysis prevents invalid references
- **No null pointer dereference**: Optional types force explicit null handling

### Runtime Guarantees
- **Reference counting safety**: Atomic operations for thread safety
- **Weak pointer safety**: Upgrade mechanism prevents dangling weak references
- **Exception safety**: RAII ensures cleanup even with exceptions

## Performance Targets

### Zero-cost Abstractions
- **Unique pointers**: Same performance as raw pointers + manual delete
- **References/borrowing**: No runtime overhead, compile-time only
- **Move semantics**: Zero-copy ownership transfer

### Acceptable Overhead
- **Shared pointers**: <10% overhead vs manual reference counting
- **Optional types**: <5% overhead vs manual null checking
- **Weak pointers**: <15% overhead vs manual weak reference tracking

## Dependencies & Prerequisites

### Required Before Starting
- [ ] Basic type system functioning
- [ ] Function calling mechanism working
- [ ] Memory allocation primitives available
- [ ] Error handling system in place

### Blocks Class System
- Smart pointers must be completed before class system Phase 4 (LLVM Type System Integration)
- Borrowing system should be ready before class method implementation

---

**Document Status**: Ready for Implementation
**Last Updated**: 2025-09-25
**Estimated Timeline**: 6-8 weeks for full implementation
**Next Steps**: Begin Phase 1 - Raw Pointer Foundation