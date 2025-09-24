# Forge Class System Design

## Basic Class Syntax
```forge
class Person {
    name: string
    age: int

    def __init__(name: string, age: int) {
        self.name = name
        self.age = age
    }

    def greet() -> string {
        return "Hello, I'm " + self.name
    }

    def get_age() -> int {
        return self.age
    }
}
```

## Generic Classes
```forge
class Array[T] {
    data: T*
    size: int
    capacity: int

    def __init__(capacity: int) {
        self.data = malloc(capacity * sizeof(T))
        self.size = 0
        self.capacity = capacity
    }

    def get(index: int) -> T {
        return self.data[index]
    }

    def set(index: int, value: T) {
        self.data[index] = value
    }

    def push(value: T) {
        if self.size >= self.capacity {
            // Resize logic
        }
        self.data[self.size] = value
        self.size = self.size + 1
    }
}
```

## Multiple Type Parameters
```forge
class Map[K, V] {
    keys: Array[K]
    values: Array[V]

    def __init__(capacity: int) {
        self.keys = Array[K](capacity)
        self.values = Array[V](capacity)
    }

    def put(key: K, value: V) {
        // Implementation
    }

    def get(key: K) -> V {
        // Implementation
    }
}
```

## Object Usage
```forge
// Basic instantiation
person: Person = Person("Alice", 25)
greeting: string = person.greet()

// Generic instantiation
numbers: Array[int] = Array[int](10)
numbers.push(42)
value: int = numbers.get(0)

// Multiple generics
cache: Map[string, int] = Map[string, int](100)
cache.put("answer", 42)
```

## Key Language Features

### Constructor Requirements
- Every class must have `__init__` method
- Constructor parameters can differ from field types
- `self` is implicit first parameter in all methods

### Generic Type Parameters
- Declared in brackets after class name: `[T]`, `[K, V]`
- Used throughout class definition
- Instantiated with concrete types: `Array[int]`

### Method Definitions
- All methods have implicit `self` parameter
- Return types are mandatory
- Field access via `self.fieldname`

### Memory Model
- Objects are heap-allocated
- Fields are stored in struct layout
- Methods are statically dispatched (no virtual dispatch initially)

## LLVM Representation

### Classes → Struct Types
```llvm
%Person = type { i8*, i32 }  ; { string name, int age }
%"Array[int]" = type { i32*, i32, i32 }  ; { int* data, int size, int capacity }
```

### Methods → Functions with Self
```llvm
define i8* @Person_greet(%Person* %self) {
    ; Method implementation
}

define i32 @"Array[int]_get"(%"Array[int]"* %self, i32 %index) {
    ; Array get implementation
}
```

### Generic Instantiation
- `Array[int]` and `Array[string]` become separate struct types
- Methods are monomorphized for each type instantiation
- Type checking ensures generic constraints are met