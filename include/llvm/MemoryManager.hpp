#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <vector>
#include <utility>

#include "../parser/Statement.hpp"
#include "../parser/ParsedType.hpp"
#include "ScopeManager.hpp"
#include "RuntimeFunctionRegistry.hpp"
#include "LLVMTypeSystem.hpp"

class MemoryManager {
private:
    llvm::LLVMContext& context;
    llvm::IRBuilder<>& builder;
    llvm::Module& _module;
    ScopeManager& scopeManager;
    RuntimeFunctionRegistry& rfRegistry;

    // Smart pointer tracking - stack of scopes
    std::vector<std::vector<std::pair<llvm::Value*, SmartPointerType>>> smartPointerScopes;

    // Defer management
    struct DeferredCall {
        llvm::Function* function;
        std::vector<llvm::Value*> args;
    };
    std::vector<DeferredCall> deferredCalls;

public:
    MemoryManager(llvm::LLVMContext& context,
                  llvm::IRBuilder<>& builder,
                  llvm::Module& _module,
                  ScopeManager& scopeManager,
                  RuntimeFunctionRegistry& rfRegistry);

    // Smart pointer management
    void createSmartPointerVariable(const VariableDeclaration& node, llvm::Value* expressionValue);
    void generateSmartPointerCleanup();
    void trackSmartPointer(llvm::Value* smartPtr, SmartPointerType type);
    void copySharedPointerVariable(const VariableDeclaration& node, llvm::Value* sourceSharedPtr);
    // Defer management
    void addDeferredCall(llvm::Function* func, std::vector<llvm::Value*> args);
    void emitDeferredCalls();

    // Scope management helpers
    void enterScope();
    void exitScope();
};