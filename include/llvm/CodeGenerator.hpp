/*
    Purpose: Helper utilities for common LLVM operations
    Role: LLVM's API is verbose. Instead of repeating "create integer constant" code everywhere, you have
    helpers
    Example: createIntConstant(42) instead of llvm::ConstantInt::get(context, llvm::APInt(32, 42))
*/