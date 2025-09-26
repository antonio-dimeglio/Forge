#include "../../include/llvm/StatementCodeGenerator.hpp"
#include "../../include/llvm/ExpressionCodeGenerator.hpp"
#include "../../include/llvm/MemoryManager.hpp"

StatementCodeGenerator::StatementCodeGenerator(llvm::LLVMContext& context,
                                             llvm::IRBuilder<>& builder,
                                             llvm::Module& _module,
                                             ScopeManager& scopeManager,
                                             RuntimeFunctionRegistry& rfRegistry,
                                             ExpressionCodeGenerator& expressionCodeGen,
                                             MemoryManager& memoryManager)
    : context(context), builder(builder), _module(_module), scopeManager(scopeManager),
      rfRegistry(rfRegistry), expressionCodeGen(expressionCodeGen), memoryManager(memoryManager) {
}

void StatementCodeGenerator::generate(const Statement& node) {
    if (const auto* program = dynamic_cast<const Program*>(&node)) {
        generateProgram(*program);
    } else if (const auto* exprStmt = dynamic_cast<const ExpressionStatement*>(&node)) {
        generateExpressionStatement(*exprStmt);
    } else if (const auto* declStmt = dynamic_cast<const VariableDeclaration*>(&node)) {
        generateVariableDeclaration(*declStmt);
    } else if (const auto* asigStmt = dynamic_cast<const Assignment*>(&node)) {
        generateAssignment(*asigStmt);
    } else if (const auto* blckStmt = dynamic_cast<const BlockStatement*>(&node)) {
        generateBlockStatement(*blckStmt);
    } else if (const auto* ifelStmt = dynamic_cast<const IfStatement*>(&node)) {
        generateIfStatement(*ifelStmt);
    } else if (const auto* whleStmt = dynamic_cast<const WhileStatement*>(&node)) {
        generateWhileStatement(*whleStmt);
    } else if (const auto* fndfStmt = dynamic_cast<const FunctionDefinition*>(&node)) {
        generateFunctionDefinition(*fndfStmt);
    } else if (const auto* rtrnStmt = dynamic_cast<const ReturnStatement*>(&node)) {
        generateReturnStatement(*rtrnStmt);
    } else if (const auto* extrnStmt = dynamic_cast<const ExternStatement*>(&node)) {
        generateExternStatement(*extrnStmt);
    } else if (const auto* deferStmt = dynamic_cast<const DeferStatement*>(&node)) {
        generateDeferStatement(*deferStmt);
    }
    else { 
        ErrorReporter::compilationError("Unimplemented statement type: " + std::string(typeid(node).name()));
    }

    return;
}

void StatementCodeGenerator::generateProgram(const Program& node) {
 llvm::Type* returnType = llvm::Type::getInt32Ty(context); // Default to int

    if (!node.statements.empty()) {
        const auto& lastStmt = node.statements.back();
        if (const auto* exprStmt = dynamic_cast<const ExpressionStatement*>(lastStmt.get())) {
            returnType = LLVMTypeSystem::inferExpressionType(context, *exprStmt->expression);
        }
    }

    // Create function with dynamic return type
    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, false);
    llvm::Function* mainFunc = llvm::Function::Create(
        funcType,
        llvm::Function::ExternalLinkage,
        "main",
        _module
    );

    llvm::BasicBlock* entry = llvm::BasicBlock::Create(context, "entry", mainFunc);
    builder.SetInsertPoint(entry);

    // Process all statements
    llvm::Value* lastValue = nullptr;
    for (const auto& stmt : node.statements) {
        if (const auto* exprStmt = dynamic_cast<const ExpressionStatement*>(stmt.get())) {
            lastValue = expressionCodeGen.generate(*exprStmt->expression);
        } else {
            generate(*stmt);
        }
    }

    // Return appropriate value
    // emitDeferredCalls();  // Execute deferred calls before main returns
    memoryManager.generateSmartPointerCleanup();  // Clean up global scope smart pointers
    if (lastValue && !lastValue->getType()->isVoidTy()) {
        builder.CreateRet(lastValue);
    } else {
        // Return default value of the correct type
        if (returnType->isIntegerTy()) {
            builder.CreateRet(llvm::ConstantInt::get(returnType, 0));
        } else if (returnType->isFloatingPointTy()) {
            builder.CreateRet(llvm::ConstantFP::get(returnType, 0.0));
        }
    }
}

void StatementCodeGenerator::generateExpressionStatement(const ExpressionStatement& node) {
    expressionCodeGen.generate(*node.expression);
}

void StatementCodeGenerator::generateVariableDeclaration(const VariableDeclaration& node) {
    // For now the getLLVMType does not consider arrays
    auto varName = node.variable.getValue();

    if (scopeManager.isDeclaredInCurrentScope(varName)) {
        ErrorReporter::variableRedeclaration(varName);
    }

    if (node.type.smartPointerType != SmartPointerType::None) {
        auto varValue = expressionCodeGen.generate(*node.expr);
        memoryManager.createSmartPointerVariable(node, varValue);
        return;
    }
    
    auto varType = LLVMTypeSystem::getLLVMType(context, node.type);
    auto varValue = expressionCodeGen.generate(*node.expr);
    llvm::AllocaInst* ptr = builder.CreateAlloca(varType, nullptr, varName);
    builder.CreateStore(varValue, ptr);
    scopeManager.declare(varName, ptr, node.type);
}

void StatementCodeGenerator::generateAssignment(const Assignment& node) {
    // TODO: Full implementation needed to handle all LHS expression types:
    // - arr[i] = value (array index assignment)
    // - obj.field = value (member access assignment)

    // For now, handle simple identifier assignment
    
    if (auto identifierExpr = dynamic_cast<const IdentifierExpression*>(node.lvalue.get())) {
        auto varName = identifierExpr->name;
        auto varPtr = scopeManager.lookup(varName);

        if (!varPtr) {
            ErrorReporter::undefinedVariable(varName);
            return;
        }

        auto newValue = expressionCodeGen.generate(*node.rvalue);
        builder.CreateStore(newValue, varPtr);
    } else if (auto unaryExpr = dynamic_cast<const UnaryExpression*>(node.lvalue.get())) {
        if (unaryExpr->operator_.getType() == TokenType::MULT) {
            auto ptrValue = expressionCodeGen.generate(*unaryExpr->operand);
            if (!ptrValue->getType()->isPointerTy()) {
                ErrorReporter::compilationError("Attempting to dereference a non-pointer type in assignment");
                return;
            }
            auto newValue = expressionCodeGen.generate(*node.rvalue);
            builder.CreateStore(newValue, ptrValue);
        } else {
            ErrorReporter::compilationError("Unsupported unary operator in LHS of assignment");
        }
    } else {
        ErrorReporter::compilationError("Unsupported LHS in assignment - only simple identifiers supported for now");
    }
}

void StatementCodeGenerator::generateBlockStatement(const BlockStatement& node) {
    scopeManager.enterScope();
    memoryManager.enterScope();

    for (const auto& stmt : node.statements) {
        generate(*stmt);
    }

    memoryManager.exitScope();
    scopeManager.exitScope();
}

void StatementCodeGenerator::generateIfStatement(const IfStatement& node) {
    // Get current function block    
    llvm::Function* currentFunction = builder.GetInsertBlock()->getParent();

    // Blocks definiton
    llvm::BasicBlock* thenBlock = llvm::BasicBlock::Create(context, LLVMLabels::IF_THEN, currentFunction);
    llvm::BasicBlock* elseBlock = nullptr;
    llvm::BasicBlock* mergeBlock = nullptr; // merge only if one of the two branches doesnt return

    if (node.elseBlock) {
        elseBlock = llvm::BasicBlock::Create(context, LLVMLabels::IF_ELSE, currentFunction);
    }    


    // Condition evaluation
    llvm::Value* condition = expressionCodeGen.generate(*node.condition);
    if (!condition) return; // Something failed when evaluating the condition

    if (!condition->getType()->isIntegerTy(1)) {
        // TODO: Conversion of non bool type to bool
    }

    // Lazy merge-block creation
    if (elseBlock) {
        mergeBlock = llvm::BasicBlock::Create(context, LLVMLabels::IF_MERGE, currentFunction);
        builder.CreateCondBr(condition, thenBlock, elseBlock);
    } else {
        mergeBlock = llvm::BasicBlock::Create(context, LLVMLabels::IF_MERGE, currentFunction);
        builder.CreateCondBr(condition, thenBlock, mergeBlock);
    }
    
    // ---- THEN ----
    builder.SetInsertPoint(thenBlock);
    generate(*node.thenBlock);
    bool thenReturns = thenBlock->getTerminator() != nullptr;
    if (!thenReturns) {
        builder.CreateBr(mergeBlock);
    }

     // ---- ELSE ----
    bool elseReturns = false;
    if (elseBlock) {
        builder.SetInsertPoint(elseBlock);
        generate(*node.elseBlock);
        elseReturns = elseBlock->getTerminator() != nullptr;
        if (!elseReturns) {
            builder.CreateBr(mergeBlock);
        }
    }

    if (!thenReturns || (elseBlock && !elseReturns) || (!elseBlock)) {
        builder.SetInsertPoint(mergeBlock);
    } else {
        // merge block unused, erase it
        mergeBlock->eraseFromParent();
    }
}

void StatementCodeGenerator::generateWhileStatement(const WhileStatement& node) {
    llvm::Function* currentFunction = builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* conditionBlock = llvm::BasicBlock::Create(context, LLVMLabels::WHILE_CONDITION, currentFunction);
    llvm::BasicBlock* bodyBlock = llvm::BasicBlock::Create(context, LLVMLabels::WHILE_BODY, currentFunction);
    llvm::BasicBlock* exitBlock = llvm::BasicBlock::Create(context, LLVMLabels::WHILE_EXIT, currentFunction);

    builder.CreateBr(conditionBlock);

    builder.SetInsertPoint(conditionBlock);
    llvm::Value* condition = expressionCodeGen.generate(*node.condition);
    if (!condition) return;
    builder.CreateCondBr(condition, bodyBlock, exitBlock);

    builder.SetInsertPoint(bodyBlock);
    generate(*node.body);
    builder.CreateBr(conditionBlock);

    builder.SetInsertPoint(exitBlock);
}

void StatementCodeGenerator::generateFunctionDefinition(const FunctionDefinition& node) {
    std::vector<llvm::Type*> paramTypes;
    for (const auto& param : node.parameters) {
        llvm::Type* paramType = LLVMTypeSystem::getLLVMType(context, param.type);
        paramTypes.push_back(paramType);
    }


    llvm::FunctionType* funcType = llvm::FunctionType::get(
        LLVMTypeSystem::getLLVMType(context, node.functionReturnType.getType()),
        paramTypes,
        false     // TODO: Implement variadic arguments for functions in the future
    );

    llvm::Function* func = llvm::Function::Create(
        funcType,
        llvm::Function::ExternalLinkage,
        node.functionName.getValue(),
        _module
    );

    // Technically there's always a previous insertion block
    llvm::BasicBlock* prevInsertBlock = nullptr;
    if (builder.GetInsertBlock()) {
        prevInsertBlock = builder.GetInsertBlock();
    }

    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(context, LLVMLabels::FUNC_ENTRY, func);
    builder.SetInsertPoint(entryBlock);

    scopeManager.enterScope();

    auto paramIt = func->arg_begin();
    for (const auto& param: node.parameters){
        llvm::Argument* arg = &(*paramIt++);
        arg->setName(param.name.getValue());

        llvm::AllocaInst* ptr = builder.CreateAlloca(
            LLVMTypeSystem::getLLVMType(context, param.type),
            nullptr, 
            param.name.getValue());

        builder.CreateStore(arg, ptr);
        scopeManager.declare(param.name.getValue(), ptr);
    }
    
    generate(*node.body);

    // If the function doesnt have a return statement one is added
    if (!builder.GetInsertBlock()->getTerminator()) {
        LLVMTypeSystem::setFunctionReturnType(builder, func);
    }
    scopeManager.exitScope();

    if (prevInsertBlock) {
        builder.SetInsertPoint(prevInsertBlock);
    } else {
        // set insert point at end of module.
    }
}

void StatementCodeGenerator::generateReturnStatement(const ReturnStatement& node) {
    llvm::Function* currentFunction = nullptr;
    
    if (builder.GetInsertBlock())
        currentFunction = builder.GetInsertBlock()->getParent();

    if (node.returnValue) {
        llvm::Value* returnValue = expressionCodeGen.generate(*node.returnValue);
        // emitDeferredCalls();  // Execute deferred calls before return - COMMENTED OUT FOR DEBUGGING
        builder.CreateRet(returnValue);
    } else {
        // No explicit return value, produce a return matching the function's type
        if (!currentFunction) {
            // should not happen, but guard
            // emitDeferredCalls();  // Execute deferred calls before return - COMMENTED OUT FOR DEBUGGING
            builder.CreateRetVoid();
            return;
        }
        // emitDeferredCalls();  // Execute deferred calls before return - COMMENTED OUT FOR DEBUGGING
        LLVMTypeSystem::setFunctionReturnType(builder, currentFunction);
    }

}

void StatementCodeGenerator::generateExternStatement(const ExternStatement& node) {
    // External function declaration: extern def malloc(size: int) -> *void

    // Convert parameter types
    std::vector<llvm::Type*> paramTypes;
    for (const auto& param : node.parameters) {
        llvm::Type* paramType = LLVMTypeSystem::getLLVMType(context, param.type);
        if (!paramType) {
            ErrorReporter::compilationError("Unknown parameter type in extern function: " + param.name.getValue());
            return;
        }
        paramTypes.push_back(paramType);
    }

    // Convert return type
    llvm::Type* returnType = LLVMTypeSystem::getLLVMType(context, node.returnType);
    if (!returnType) {
        ErrorReporter::compilationError("Unknown return type in extern function: " + node.functionName.getValue());
        return;
    }

    // Create function type
    llvm::FunctionType* funcType = llvm::FunctionType::get(returnType, paramTypes, false);

    // Declare external function
    llvm::Function* externFunc = llvm::Function::Create(
        funcType,
        llvm::Function::ExternalLinkage,  // External linkage for C functions
        node.functionName.getValue(),
        _module
    );

    // Set parameter names (helpful for debugging)
    auto argIt = externFunc->arg_begin();
    for (const auto& param : node.parameters) {
        argIt->setName(param.name.getValue());
        ++argIt;
    }
}

void StatementCodeGenerator::generateDeferStatement(const DeferStatement& node) {
    // Store the defer statement for later execution at scope end
    // For now, we assume the expression is a function call

    // This is a simplified implementation - in a full implementation,
    // we'd need to evaluate the expression and extract the function call
    ErrorReporter::compilationError("Defer statements not fully implemented yet - placeholder added");
}