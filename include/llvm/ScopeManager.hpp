#pragma once
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <string>
#include <vector>
#include <unordered_map>

class ScopeManager {
    public:
        ScopeManager();

        void enterScope();
        void exitScope();

        void declare(const std::string& name, llvm::Value* ptr);
        llvm::Value* lookup(const std::string& name);

        bool isDeclaredInCurrentScope(const std::string& name);
        size_t getCurrentScopeDepth() const { return scopeStack.size(); };

    private:
        std::vector<std::unordered_map<std::string, llvm::Value*>> scopeStack;
};