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
#include "LLVMTypeSystem.hpp"
#include "ErrorReporter.hpp"

class LLVMCompiler {
    private:
        llvm::LLVMContext context;
        std::unique_ptr<llvm::Module> _module;
        llvm::IRBuilder<> builder;
        std::unordered_map<std::string, llvm::Value*> symbolTable;
        ScopeManager scopeManager;
        
    public:
        LLVMCompiler();
        std::unique_ptr<llvm::Module> compile(const Statement& ast);

        void visit(const Program& node);
        void visit(const Statement& node);
        void visit(const ExpressionStatement& node);
        void visit(const VariableDeclaration& node);
        void visit(const Assignment& node);
        void visit(const BlockStatement& node);
        void visit(const IfStatement& node);
        void visit(const WhileStatement& node);
        void visit(const FunctionDefinition& node);
        void visit(const ReturnStatement& node);

        llvm::Value* visit(const Expression& node); 
        llvm::Value* visit(const LiteralExpression& node);
        llvm::Value* visit(const BinaryExpression& node);
        llvm::Value* visit(const UnaryExpression& node);
        llvm::Value* visit(const IdentifierExpression& node);
        llvm::Value* visit(const FunctionCall& node);         
        llvm::Value* visit(const ArrayLiteralExpression& node);     
        llvm::Value* visit(const IndexAccessExpression& node);
        llvm::Value* visit(const MemberAccessExpression& node);
};