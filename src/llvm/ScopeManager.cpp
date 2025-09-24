#include "../../include/llvm/ScopeManager.hpp"

ScopeManager::ScopeManager() {
    // Global scope init
    scopeStack.push_back({});
}


void ScopeManager::enterScope() {
    scopeStack.push_back({});
}
void ScopeManager::exitScope() { 
    if (scopeStack.size() > 1) {
        scopeStack.pop_back();
    }
}

void ScopeManager::declare(const std::string& name, llvm::Value* ptr) {
    scopeStack.back()[name] = ptr;
}

llvm::Value* ScopeManager::lookup(const std::string& name) {
    for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return found->second;
        }
    }

    return nullptr; 
}

bool ScopeManager::isDeclaredInCurrentScope(const std::string& name) {
    auto currentScope = scopeStack.back();
    return currentScope.find(name) != currentScope.end();
}
