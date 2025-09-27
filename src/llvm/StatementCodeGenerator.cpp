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
    // TODO: Fix getLLVMType does not consider arrays
    auto varName = node.variable.getValue();

    if (scopeManager.isDeclaredInCurrentScope(varName)) {
        ErrorReporter::variableRedeclaration(varName);
    }

    if (node.type.smartPointerType != SmartPointerType::None) {
        // Smart pointers can only be assigned from 'new' expressions
        if (const auto* newExpr = dynamic_cast<const NewExpression*>(node.expr.get())) {
            auto varValue = expressionCodeGen.generate(*node.expr);
            memoryManager.createSmartPointerVariable(node, varValue);
            return;
        } else if (const auto* identExpr = dynamic_cast<const IdentifierExpression*>(node.expr.get())) {
            auto identValue = scopeManager.lookupType(identExpr->name);
            
            if (identValue->smartPointerType == SmartPointerType::Shared) {
                if (node.type.smartPointerType != SmartPointerType::Shared) {
                    ErrorReporter::compilationError("Cannot assign between different smart pointer types (unique/shared/weak).");
                    return;
                }
                auto sourcePtr = scopeManager.lookup(identExpr->name);
                memoryManager.copySharedPointerVariable(node, sourcePtr);
                return;
            } else if (identValue->smartPointerType == SmartPointerType::Weak){
                ErrorReporter::compilationError("Unimplemented operation for weak pointers");
                return;
            } else {
                ErrorReporter::compilationError("Cannot assign non safe pointer type to smart pointer.");
                return; 
            }
        } else {
            ErrorReporter::compilationError("Smart pointers must be assigned from 'new' expressions, not literals or other values");
            return;
        }
    }

    // Regular variables cannot be assigned from 'new' expressions
    if (const auto* newExpr = dynamic_cast<const NewExpression*>(node.expr.get())) {
        ErrorReporter::compilationError("Regular variables cannot be assigned from 'new' expressions. Use smart pointer types (unique, shared, weak) instead.");
        return;
    }

    auto varType = LLVMTypeSystem::getLLVMType(context, node.type);
    auto varValue = expressionCodeGen.generate(*node.expr);
    auto actualType = varValue->getType();

    if (!LLVMTypeSystem::areTypesCompatible(varType, varValue)) {
        ErrorReporter::typeMismatch(varType->isVoidTy() ? "void" : (varType->isIntegerTy() ? "int" : (varType->isFloatingPointTy() ? "float" : "other")),
                actualType->isVoidTy() ? "void" : (actualType->isIntegerTy() ? "int" : (actualType->isFloatingPointTy() ? "float" : "other")));
        return;
    }
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

        // Get the type of the LHS variable
        auto lhsType = scopeManager.lookupType(varName);
        if (!lhsType) {
            ErrorReporter::compilationError("Could not determine type of variable: " + varName);
            return;
        }

        // Handle smart pointer assignments
        if (lhsType->smartPointerType != SmartPointerType::None) {
            handleSmartPointerAssignment(varName, varPtr, *lhsType, node.rvalue.get());
            return;
        }

        // Regular variable assignment
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
    enterDeferScope();
    memoryManager.enterScope();

    for (const auto& stmt : node.statements) {
        generate(*stmt);
    }

    memoryManager.exitScope();
    exitDeferScope();
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

    // Function parameters need to be in the same scope as the function body
    // Let BlockStatement handle all scope management, but declare parameters first
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

    // Generate the function body - BlockStatement handles its own scope AND defer
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
    if (!deferStack.empty()) {
        deferStack.back().push_back(node.expression.get());
    } else {
        ErrorReporter::compilationError("Defer statement used outside of any scope");
    }
}

void StatementCodeGenerator::enterDeferScope() {
    deferStack.emplace_back();
}

void StatementCodeGenerator::exitDeferScope() {
    if (deferStack.empty()) {
        ErrorReporter::compilationError("Trying to exit defer scope but stack is empty!");
        return;
    }

    emitDeferredCalls();
    deferStack.pop_back();
}

void StatementCodeGenerator::emitDeferredCalls() {
    if (deferStack.empty()) return;

    auto& currScope = deferStack.back();

    for (auto it = currScope.rbegin(); it != currScope.rend(); ++it) {
        expressionCodeGen.generate(**it);
    }
}

void StatementCodeGenerator::handleSmartPointerAssignment(const std::string& varName, llvm::Value* varPtr,
                                                         const ParsedType& lhsType, const Expression* rvalue) {
    // Handle smart pointer assignments: sp1 = sp2, sp = literal, etc.

    // First, validate what type of assignment this is
    if (const auto* newExpr = dynamic_cast<const NewExpression*>(rvalue)) {
        // sp = new 42 -> Invalid for existing smart pointers
        ErrorReporter::compilationError("Cannot assign 'new' expression to existing smart pointer. Use variable declaration instead.");
        return;

    } else if (const auto* literalExpr = dynamic_cast<const LiteralExpression*>(rvalue)) {
        // sp = 42 -> Invalid
        ErrorReporter::compilationError("Cannot assign literal to smart pointer. Smart pointers must point to heap-allocated objects.");
        return;

    } else if (const auto* identExpr = dynamic_cast<const IdentifierExpression*>(rvalue)) {
        // sp1 = sp2 -> Potentially valid, check types
        std::string rhsVarName = identExpr->name;
        auto rhsType = scopeManager.lookupType(rhsVarName);

        if (!rhsType) {
            ErrorReporter::undefinedVariable(rhsVarName);
            return;
        }

        // Check smart pointer type compatibility
        if (rhsType->smartPointerType == SmartPointerType::None) {
            ErrorReporter::compilationError("Cannot assign regular variable to smart pointer.");
            return;
        }

        if (lhsType.smartPointerType != rhsType->smartPointerType) {
            ErrorReporter::compilationError("Cannot assign between different smart pointer types (unique/shared/weak).");
            return;
        }

        // Check if unique pointers (should not be assignable)
        if (lhsType.smartPointerType == SmartPointerType::Unique) {
            ErrorReporter::compilationError("Unique pointers cannot be assigned. Use move semantics instead.");
            return;
        }

        // Handle shared pointer assignment
        if (lhsType.smartPointerType == SmartPointerType::Shared) {
            handleSharedPointerAssignment(varName, varPtr, rhsVarName);
            return;
        }

        // Handle weak pointer assignment (future)
        if (lhsType.smartPointerType == SmartPointerType::Weak) {
            ErrorReporter::compilationError("Weak pointer assignment not yet implemented.");
            return;
        }

    } else {
        // Other expression types (function calls, etc.)
        ErrorReporter::compilationError("Invalid right-hand side for smart pointer assignment.");
        return;
    }
}

void StatementCodeGenerator::handleSharedPointerAssignment(const std::string& lhsVarName, llvm::Value* lhsVarPtr,
                                                          const std::string& rhsVarName) {
    // Handle: sp1 = sp2 (shared pointer assignment)

    // 1. Release the old value in sp1
    llvm::Function* releaseFunc = rfRegistry.getFunction(LLVMLabels::SHARED_PTR_RELEASE);
    builder.CreateCall(releaseFunc, {lhsVarPtr});

    // 2. Get the source shared pointer
    auto rhsVarPtr = scopeManager.lookup(rhsVarName);
    if (!rhsVarPtr) {
        ErrorReporter::undefinedVariable(rhsVarName);
        return;
    }

    // 3. Copy fields from source to destination (same as copySharedPointerVariable)
    // Get the smart pointer type from the LHS variable type
    auto lhsType = scopeManager.lookupType(lhsVarName);
    auto smartPtrType = LLVMTypeSystem::getLLVMType(context, *lhsType);

    // Get ref_count from source (field 0)
    llvm::Value* sourceRefCountPtr = builder.CreateGEP(smartPtrType, rhsVarPtr, {builder.getInt32(0), builder.getInt32(0)});
    llvm::Value* sourceRefCount = builder.CreateLoad(llvm::Type::getInt32Ty(context), sourceRefCountPtr);

    // Get data pointer from source (field 1)
    llvm::Value* sourceDataPtr = builder.CreateGEP(smartPtrType, rhsVarPtr, {builder.getInt32(0), builder.getInt32(1)});
    llvm::Value* sourceData = builder.CreateLoad(llvm::PointerType::getUnqual(context), sourceDataPtr);

    // Set ref_count in destination (field 0)
    llvm::Value* destRefCountPtr = builder.CreateGEP(smartPtrType, lhsVarPtr, {builder.getInt32(0), builder.getInt32(0)});
    builder.CreateStore(sourceRefCount, destRefCountPtr);

    // Set data pointer in destination (field 1)
    llvm::Value* destDataPtr = builder.CreateGEP(smartPtrType, lhsVarPtr, {builder.getInt32(0), builder.getInt32(1)});
    builder.CreateStore(sourceData, destDataPtr);

    // 4. Retain the new value
    llvm::Function* retainFunc = rfRegistry.getFunction(LLVMLabels::SHARED_PTR_RETAIN);
    builder.CreateCall(retainFunc, {lhsVarPtr});
}

