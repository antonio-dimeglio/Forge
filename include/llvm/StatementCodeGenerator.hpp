#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <memory>

#include "../parser/Statement.hpp"
#include "ScopeManager.hpp"
#include "RuntimeFunctionRegistry.hpp"

// Forward declarations to avoid circular dependencies
class ExpressionCodeGenerator;
class MemoryManager;

class StatementCodeGenerator {
private:
    llvm::LLVMContext& context;
    llvm::IRBuilder<>& builder;
    llvm::Module& _module;
    ScopeManager& scopeManager;
    RuntimeFunctionRegistry& rfRegistry;
    ExpressionCodeGenerator& expressionCodeGen;
    MemoryManager& memoryManager;

public:
    StatementCodeGenerator(llvm::LLVMContext& context,
                          llvm::IRBuilder<>& builder,
                          llvm::Module& _module,
                          ScopeManager& scopeManager,
                          RuntimeFunctionRegistry& rfRegistry,
                          ExpressionCodeGenerator& expressionCodeGen,
                          MemoryManager& memoryManager);

    // Core statement generation
    void generate(const Statement& node);

    // Specific statement types
    void generateProgram(const Program& node);
    void generateExpressionStatement(const ExpressionStatement& node);
    void generateVariableDeclaration(const VariableDeclaration& node);
    void generateAssignment(const Assignment& node);
    void generateBlockStatement(const BlockStatement& node);
    void generateIfStatement(const IfStatement& node);
    void generateWhileStatement(const WhileStatement& node);
    void generateFunctionDefinition(const FunctionDefinition& node);
    void generateReturnStatement(const ReturnStatement& node);
    void generateExternStatement(const ExternStatement& node);
    void generateDeferStatement(const DeferStatement& node);
};