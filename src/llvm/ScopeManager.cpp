#include "../../include/llvm/ScopeManager.hpp"

ScopeManager::ScopeManager() {
    // Global scope init
    scopeStack.push_back({});
    legacyScopeStack.push_back({});
}


void ScopeManager::enterScope() {
    scopeStack.push_back({});
    legacyScopeStack.push_back({});
}
void ScopeManager::exitScope() {
    if (scopeStack.size() > 1) {
        scopeStack.pop_back();
    }
    if (legacyScopeStack.size() > 1) {
        legacyScopeStack.pop_back();
    }
}

void ScopeManager::declare(const std::string& name, llvm::Value* ptr) {
    // Legacy method for backwards compatibility
    legacyScopeStack.back()[name] = ptr;
}

void ScopeManager::declare(const std::string& name, llvm::Value* ptr, const ParsedType& type) {
    // New method with type information
    scopeStack.back()[name] = VariableInfo(ptr, type, false);
    // Also add to legacy for backwards compatibility
    legacyScopeStack.back()[name] = ptr;
}

llvm::Value* ScopeManager::lookup(const std::string& name) {
    // First try the new scope stack
    for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return found->second.ptr;
        }
    }

    // Fall back to legacy scope stack
    for (auto it = legacyScopeStack.rbegin(); it != legacyScopeStack.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return found->second;
        }
    }

    return nullptr;
}

ParsedType* ScopeManager::lookupType(const std::string& name) {
    for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return &found->second.type;
        }
    }
    return nullptr;
}

bool ScopeManager::isDeclaredInCurrentScope(const std::string& name) {
    // Check both new and legacy scopes
    auto& currentScope = scopeStack.back();
    auto& currentLegacyScope = legacyScopeStack.back();
    return currentScope.find(name) != currentScope.end() ||
           currentLegacyScope.find(name) != currentLegacyScope.end();
}

bool ScopeManager::isVariableMoved(const std::string& name) {
    for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            return found->second.moved;
        }
    }
    return false;
}

void ScopeManager::markVariableAsMoved(const std::string& name) {
    for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) {
            found->second.moved = true;
        }
    }
}
