/*
    Purpose: Manages LLVM's global state and configuration
    Role: LLVM needs a "context" object to track types, constants, and metadata. This wraps that
    complexity
    Why separate: You might want different compilation contexts (debug vs release, different target
    architectures)
*/