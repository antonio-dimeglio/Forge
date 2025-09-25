#include "../../include/llvm/BinaryOperationHandler.hpp"

llvm::Value* BinaryOperationHandler::handleOperation(
    llvm::IRBuilder<>& builder,
    TokenType op,
    llvm::Value* lhs,
    llvm::Value* rhs) {

    // Check for pointer arithmetic first
    if (lhs->getType()->isPointerTy() && rhs->getType()->isIntegerTy()) {
        return handlePointerArithmetic(builder, op, lhs, rhs);
    } else if (lhs->getType()->isIntegerTy() && rhs->getType()->isPointerTy()) {
        // Handle integer + pointer (commutative for addition)
        if (op == TokenType::PLUS) {
            return handlePointerArithmetic(builder, op, rhs, lhs);
        }
    } else if (lhs->getType()->isPointerTy() && rhs->getType()->isPointerTy()) {
        return handlePointerComparison(builder, op, lhs, rhs);
    }

    // Determine operation type based on operand types
    if (lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy()) {
        if (lhs->getType()->isIntegerTy(1) && rhs->getType()->isIntegerTy(1)) {
            return handleBoolOp(builder, op, lhs, rhs);
        } else {
            return handleIntegerOp(builder, op, lhs, rhs);
        }
    } else if (lhs->getType()->isFloatingPointTy() || rhs->getType()->isFloatingPointTy()) {
        return handleFloatOp(builder, op, lhs, rhs);
    }

    return nullptr; // Unsupported type combination
}

llvm::Value* BinaryOperationHandler::handleIntegerOp(llvm::IRBuilder<>& builder, TokenType op, llvm::Value* lhs, llvm::Value* rhs) {
    switch (op) {
        case TokenType::PLUS: return builder.CreateAdd(lhs, rhs);
        case TokenType::MINUS: return builder.CreateSub(lhs, rhs);
        case TokenType::MULT: return builder.CreateMul(lhs, rhs);
        case TokenType::DIV: return builder.CreateSDiv(lhs, rhs);
        case TokenType::BITWISE_AND: return builder.CreateAnd(lhs, rhs);
        case TokenType::BITWISE_OR: return builder.CreateOr(lhs, rhs);
        case TokenType::BITWISE_XOR: return builder.CreateXor(lhs, rhs);
        case TokenType::EQUAL_EQUAL: return builder.CreateICmpEQ(lhs, rhs);
        case TokenType::NOT_EQUAL: return builder.CreateICmpNE(lhs, rhs);
        case TokenType::GREATER: return builder.CreateICmpSGT(lhs, rhs);
        case TokenType::LESS: return builder.CreateICmpSLT(lhs, rhs);
        case TokenType::LEQ: return builder.CreateICmpSLE(lhs, rhs);
        case TokenType::GEQ: return builder.CreateICmpSGE(lhs, rhs);
        default:
            return nullptr; // Unsupported;
    }
}

llvm::Value* BinaryOperationHandler::handleFloatOp(llvm::IRBuilder<>& builder, TokenType op, llvm::Value* lhs, llvm::Value* rhs) {
    switch (op) {
        case TokenType::PLUS: return builder.CreateFAdd(lhs, rhs);
        case TokenType::MINUS: return builder.CreateFSub(lhs, rhs);
        case TokenType::MULT: return builder.CreateFMul(lhs, rhs);
        case TokenType::DIV: return builder.CreateFDiv(lhs, rhs);
        case TokenType::EQUAL_EQUAL: return builder.CreateFCmpOEQ(lhs, rhs);
        case TokenType::NOT_EQUAL: return builder.CreateFCmpONE(lhs, rhs);
        case TokenType::GREATER: return builder.CreateFCmpOGT(lhs, rhs);
        case TokenType::LESS: return builder.CreateFCmpOLT(lhs, rhs);
        case TokenType::LEQ: return builder.CreateFCmpOLE(lhs, rhs);
        case TokenType::GEQ: return builder.CreateFCmpOGE(lhs, rhs);
        default:
            return nullptr; // Unsupported;
    }
}

llvm::Value* BinaryOperationHandler::handleBoolOp(llvm::IRBuilder<>& builder, TokenType op, llvm::Value* lhs, llvm::Value* rhs) {
    switch (op) {
        case TokenType::BITWISE_AND: return builder.CreateAnd(lhs, rhs);
        case TokenType::BITWISE_OR: return builder.CreateOr(lhs, rhs);
        case TokenType::BITWISE_XOR: return builder.CreateXor(lhs, rhs);
        case TokenType::EQUAL_EQUAL: return builder.CreateICmpEQ(lhs, rhs);
        case TokenType::NOT_EQUAL: return builder.CreateICmpNE(lhs, rhs);
        case TokenType::GREATER: return builder.CreateICmpSGT(lhs, rhs);
        case TokenType::LESS: return builder.CreateICmpSLT(lhs, rhs);
        case TokenType::LEQ: return builder.CreateICmpSLE(lhs, rhs);
        case TokenType::GEQ: return builder.CreateICmpSGE(lhs, rhs);
        default:
            return nullptr; // Unsupported;
    }
}

llvm::Value* BinaryOperationHandler::handlePointerArithmetic(
    llvm::IRBuilder<>& builder,
    TokenType op,
    llvm::Value* ptr,
    llvm::Value* offset) {

    // For opaque pointers, we assume byte-level arithmetic
    // TODO: This should be improved to use actual element types
    llvm::Type* elementType = llvm::Type::getInt8Ty(builder.getContext());

    switch (op) {
        case TokenType::PLUS: {
            // ptr + offset using GEP (Get Element Pointer)
            return builder.CreateGEP(elementType, ptr, offset, "ptr_add");
        }
        case TokenType::MINUS: {
            // ptr - offset: negate offset then add
            llvm::Value* negOffset = builder.CreateNeg(offset, "neg_offset");
            return builder.CreateGEP(elementType, ptr, negOffset, "ptr_sub");
        }
        default:
            return nullptr; // Unsupported pointer arithmetic operation
    }
}

llvm::Value* BinaryOperationHandler::handlePointerComparison(
    llvm::IRBuilder<>& builder,
    TokenType op,
    llvm::Value* lhs,
    llvm::Value* rhs) {

    switch (op) {
        case TokenType::EQUAL_EQUAL:
            return builder.CreateICmpEQ(lhs, rhs, "ptr_eq");
        case TokenType::NOT_EQUAL:
            return builder.CreateICmpNE(lhs, rhs, "ptr_ne");
        case TokenType::LESS:
            return builder.CreateICmpULT(lhs, rhs, "ptr_lt");
        case TokenType::LEQ:
            return builder.CreateICmpULE(lhs, rhs, "ptr_le");
        case TokenType::GREATER:
            return builder.CreateICmpUGT(lhs, rhs, "ptr_gt");
        case TokenType::GEQ:
            return builder.CreateICmpUGE(lhs, rhs, "ptr_ge");
        default:
            return nullptr; // Unsupported pointer comparison
    }
}