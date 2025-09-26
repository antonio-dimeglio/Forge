#pragma once

#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/TargetSelect.h> 
#include <memory>
#include <unordered_map>
#include <string>

#include "../parser/Statement.hpp"
#include "ScopeManager.hpp"
#include "RuntimeFunctionRegistry.hpp"
#include "ExpressionCodeGenerator.hpp"
#include "StatementCodeGenerator.hpp"
#include "MemoryManager.hpp"
#include "LLVMTypeSystem.hpp"
#include "ErrorReporter.hpp"

class LLVMCompiler {
    private:
        llvm::LLVMContext context;
        std::unique_ptr<llvm::Module> _module;
        llvm::IRBuilder<> builder;
        std::unordered_map<std::string, llvm::Value*> symbolTable;
        ScopeManager scopeManager;
        std::unique_ptr<RuntimeFunctionRegistry> rfRegistry;
        std::unique_ptr<ExpressionCodeGenerator> expressionCodeGen;
        std::unique_ptr<StatementCodeGenerator> statementCodeGen;
        std::unique_ptr<MemoryManager> memoryManager;
    public:
        LLVMCompiler();
        void compile(const Statement& ast);
        llvm::Module* getModule() const;
        void printModule() const;
        void generateObjectFile(const std::string& filename);

        void visit(const Statement& node);
        llvm::Value* visit(const Expression& node); 
};