#pragma once
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <string>
#include <vector>
#include <unordered_map>
#include "../parser/ParsedType.hpp"

struct VariableInfo {
    llvm::Value* ptr;
    ParsedType type;
    bool moved;

    VariableInfo() : ptr(nullptr), type(ParsedType{Token(TokenType::VOID, "", 0, 0), {}, 0, false, false, false, false, SmartPointerType::None}), moved(false) {}
    VariableInfo(llvm::Value* ptr, const ParsedType& type, bool moved) : ptr(ptr), type(type), moved(moved) {}
};

class ScopeManager {
    public:
        ScopeManager();

        void enterScope();
        void exitScope();

        void declare(const std::string& name, llvm::Value* ptr);
        void declare(const std::string& name, llvm::Value* ptr, const ParsedType& type);
        llvm::Value* lookup(const std::string& name);
        ParsedType* lookupType(const std::string& name);
        bool isVariableMoved(const std::string& name);
        void markVariableAsMoved(const std::string& name);

        bool isDeclaredInCurrentScope(const std::string& name);
        size_t getCurrentScopeDepth() const { return scopeStack.size(); };

    private:
        std::vector<std::unordered_map<std::string, VariableInfo>> scopeStack;
        std::vector<std::unordered_map<std::string, llvm::Value*>> legacyScopeStack; // For backwards compatibility
};