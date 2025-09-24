#include "../../include/llvm/BinaryOperationHandler.hpp"

llvm::Value* BinaryOperationHandler::handleOperation(
    llvm::IRBuilder<>& builder,
    TokenType op,
    llvm::Value* lhs,
    llvm::Value* rhs) {

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