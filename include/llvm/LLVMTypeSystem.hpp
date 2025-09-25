#pragma once

#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include "../parser/Statement.hpp"
#include "../parser/ParsedType.hpp"

class LLVMTypeSystem {
    public:
        static llvm::Type* getLLVMType(llvm::LLVMContext& context, TokenType tokenType);
        static llvm::Type* getLLVMType(llvm::LLVMContext& context, const ParsedType& parsedType);
        static llvm::Value* evaluateConstantNumerical(llvm::LLVMContext& context, std::string value);
        static llvm::Type* inferExpressionType(llvm::LLVMContext& context, const Expression& expr);
        static bool canPromoteType(llvm::Type* from, llvm::Type* to);
        static llvm::Type* getPromotedType(llvm::Type* left, llvm::Type* right);
        static void setFunctionReturnType(llvm::IRBuilder<>& builder, llvm::Function* func);
        static llvm::StructType* getUniquePointerType(llvm::LLVMContext& context, TokenType elementType);
        static llvm::StructType* getSharedPointerType(llvm::LLVMContext& context, TokenType elementType);
        static llvm::StructType* getWeakPointerType(llvm::LLVMContext& context, TokenType elementType);
        static bool isPointerCompatbile();
        static llvm::Type* inferPointerElementType(llvm::LLVMContext& context, const UnaryExpression& node);
};