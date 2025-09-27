// Fixed LLVM type conversion for LLVM 15.x
#include <llvm/IR/Type.h>
#include <llvm/IR/LLVMContext.h>

// Your TokenType enum (assuming this exists)
enum class TokenType {
    INT, FLOAT, DOUBLE, BOOL, STRING, VOID
};

llvm::Type* toLLVMType(llvm::LLVMContext& ctx, TokenType primitiveKind_) {
    switch (primitiveKind_) {
        case TokenType::INT:
            return llvm::Type::getInt32Ty(ctx);
        case TokenType::FLOAT:
            return llvm::Type::getFloatTy(ctx);
        case TokenType::DOUBLE:
            return llvm::Type::getDoubleTy(ctx);
        case TokenType::BOOL:
            return llvm::Type::getInt1Ty(ctx);
        case TokenType::STRING:
            // LLVM 15+ opaque pointer style
            return llvm::PointerType::getUnqual(ctx);
        case TokenType::VOID:
            return llvm::Type::getVoidTy(ctx);
        default:
            throw std::runtime_error("Unsupported primitive type for LLVM conversion");
    }
}