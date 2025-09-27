#pragma once
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Verifier.h>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <optional>

#include "../memory/MemoryModel.hpp"
#include "../types/Type.hpp"
#include "../errors/Result.hpp"
#include "../errors/ErrorTypes.hpp"

namespace forge::codegen {

    class SymbolTable {
        public:
            struct Symbol {
                std::string name;
                std::unique_ptr<forge::types::Type> type;
                llvm::Value* llvmValue;
                forge::memory::MemoryModel::Ownership ownership;
                forge::errors::SourceLocation declaration;
            };

            class Scope {
            public:
                void declare(std::string name, Symbol symbol);
                Symbol* lookup(const std::string& name);
                const Symbol* lookup(const std::string& name) const;

                std::vector<Symbol*> getAllSymbols();

            private:
                std::unordered_map<std::string, std::unique_ptr<Symbol>> symbols_;
            };

            // Scope Management
            void enterScope();
            void exitScope();
            size_t getCurrentScopeDepth() const;

            // Symbol Management
            forge::errors::Result<void, forge::errors::TypeError> declare(std::string name, Symbol symbol);
            std::optional<Symbol*> lookup(const std::string& name) const;

            

            // Query Interface
            std::vector<Symbol*> getSymbolsInCurrentScope();
            std::vector<Symbol*> getAllVisibleSymbols();

        private:
            std::vector<std::unique_ptr<Scope>> scopes_;

            Scope& getCurrentScope();
            const Scope& getCurrentScope() const;
    };

}