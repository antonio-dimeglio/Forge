#pragma once 
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include "../lexer/Token.hpp"

class BinaryOperationHandler {
    public:
        static llvm::Value* handleOperation(
            llvm::IRBuilder<>& builder,
            TokenType op, 
            llvm::Value* lhs, 
            llvm::Value* rhs);

    private:
        static llvm::Value* handleIntegerOp(llvm::IRBuilder<>& builder, TokenType op, llvm::Value* lhs, llvm::Value* rhs); 
        static llvm::Value* handleFloatOp(llvm::IRBuilder<>& builder, TokenType op, llvm::Value* lhs, llvm::Value* rhs);
        static llvm::Value* handleBoolOp(llvm::IRBuilder<>& builder, TokenType op, llvm::Value* lhs, llvm::Value* rhs);
};