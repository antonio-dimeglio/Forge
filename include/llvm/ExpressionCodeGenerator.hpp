#pragma once 

#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/GlobalVariable.h>
#include "LLVMTypeSystem.hpp"
#include "ScopeManager.hpp"
#include "RuntimeFunctionRegistry.hpp"
#include "../parser/Expression.hpp"
#include "ErrorReporter.hpp"
#include "BinaryOperationHandler.hpp"


class ExpressionCodeGenerator {
    private:
        llvm::LLVMContext& context;
        llvm::IRBuilder<>& builder;
        ScopeManager& scopeManager;
        RuntimeFunctionRegistry& rfRegistry;
        llvm::Module& _module;

    public:
        ExpressionCodeGenerator(llvm::LLVMContext& context,
                            llvm::IRBuilder<>& builder,
                            ScopeManager& scopeManager,
                            RuntimeFunctionRegistry& rfRegistry,
                            llvm::Module& _module);

        // Core expression generation
        llvm::Value* generate(const Expression& node);

        // Specific expression types
        llvm::Value* generateLiteral(const LiteralExpression& node);
        llvm::Value* generateBinary(const BinaryExpression& node);
        llvm::Value* generateUnary(const UnaryExpression& node);
        llvm::Value* generateIdentifier(const IdentifierExpression& node);
        llvm::Value* generateFunctionCall(const FunctionCall& node);
        llvm::Value* generateArrayLiteral(const ArrayLiteralExpression& node);
        llvm::Value* generateIndexAccess(const IndexAccessExpression& node);
        llvm::Value* generateMemberAccess(const MemberAccessExpression& node);
        llvm::Value* generateNew(const NewExpression& node);
        llvm::Value* generateMove(const MoveExpression& node);
};