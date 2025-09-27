#pragma once

namespace forge::types {
    enum class Kind {
        Primitive, // Only primitive types are int, float, double, string and bool.
        Pointer, // Raw pointer type, *T
        Reference, // Constant and mutable reference, &/&mut
        SmartPointer, // Unique<T>, Shared<T>, Weak<T>
        Array, // [T; N], [T]
        Function,     // (T1, T2) -> T3
        Class,       // Custom user types
        Generic,       // T, U (template params)
    };
}