#include "../../include/llvm/LLVMCompiler.hpp"
#include "../../include/llvm/BinaryOperationHandler.hpp"
#include "../../include/llvm/LLVMTypeSystem.hpp"
#include "../../include/llvm/ErrorReporter.hpp"
#include "../../include/llvm/LLVMLabels.hpp"
#include <iostream>
#include <string>
#include <optional>

// LLVM target generation includes
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/MC/TargetRegistry.h>
/*
    TODO: 
        Implements classes
        Implement arrays (but first fix indexing by using a single square-multiple comma indexing style)
        Implement string support for operations such as +
*/

LLVMCompiler::LLVMCompiler() : context(), builder(context) {
    _module = std::make_unique<llvm::Module>(LLVMLabels::MAIN_LABEL, context);
}

void LLVMCompiler::compile(const Statement& ast) {
    declareRuntimeFunctions();
    ast.accept(*this);
}

void LLVMCompiler::visit(const Program& node) {
    // Pre-scan to determine the return type
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
        _module.get()
    );

    llvm::BasicBlock* entry = llvm::BasicBlock::Create(context, "entry", mainFunc);
    builder.SetInsertPoint(entry);

    // Process all statements
    llvm::Value* lastValue = nullptr;
    for (const auto& stmt : node.statements) {
        if (const auto* exprStmt = dynamic_cast<const ExpressionStatement*>(stmt.get())) {
            lastValue = visit(*exprStmt->expression);
        } else {
            visit(*stmt);
        }
    }

    // Return appropriate value
    // emitDeferredCalls();  // Execute deferred calls before main returns - COMMENTED OUT FOR DEBUGGING
    generateSmartPointerCleanup();  // Clean up smart pointers before main returns
    if (lastValue) {
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

// Dispatch to specific visit methods based on actual type
void LLVMCompiler::visit(const Statement& node) {
    if (const auto* program = dynamic_cast<const Program*>(&node)) {
        visit(*program);
    } else if (const auto* exprStmt = dynamic_cast<const ExpressionStatement*>(&node)) {
        visit(*exprStmt);
    } else if (const auto* declStmt = dynamic_cast<const VariableDeclaration*>(&node)) {
        visit(*declStmt);
    } else if (const auto* asigStmt = dynamic_cast<const Assignment*>(&node)) {
        visit(*asigStmt);
    } else if (const auto* blckStmt = dynamic_cast<const BlockStatement*>(&node)) {
        visit(*blckStmt);
    } else if (const auto* ifelStmt = dynamic_cast<const IfStatement*>(&node)) {
        visit(*ifelStmt);
    } else if (const auto* whleStmt = dynamic_cast<const WhileStatement*>(&node)) {
        visit(*whleStmt);
    } else if (const auto* fndfStmt = dynamic_cast<const FunctionDefinition*>(&node)) {
        visit(*fndfStmt);
    } else if (const auto* rtrnStmt = dynamic_cast<const ReturnStatement*>(&node)) {
        visit(*rtrnStmt);
    } else if (const auto* extrnStmt = dynamic_cast<const ExternStatement*>(&node)) {
        visit(*extrnStmt);
    } else if (const auto* deferStmt = dynamic_cast<const DeferStatement*>(&node)) {
        visit(*deferStmt);
    }
    else { 
        ErrorReporter::compilationError("Unimplemented statement type: " + std::string(typeid(node).name()));
    }

    return;
}

llvm::Value* LLVMCompiler::visit(const Expression& node) {
    if (const auto* literal = dynamic_cast<const LiteralExpression*>(&node)) {
        return visit(*literal);
    } else if (const auto* bnry = dynamic_cast<const BinaryExpression*>(&node)) {
        return visit(*bnry);
    } else if (const auto* unry = dynamic_cast<const UnaryExpression*>(&node)) {
        return visit(*unry);
    } else if (const auto* idfr = dynamic_cast<const IdentifierExpression*>(&node)) {
        return visit(*idfr);
    } else if (const auto* fncl = dynamic_cast<const FunctionCall*>(&node)) {
        return visit(*fncl);
    } else if (const auto* arlt = dynamic_cast<const ArrayLiteralExpression*>(&node)) {
        return visit(*arlt);
    } else if (const auto* inac = dynamic_cast<const IndexAccessExpression*>(&node)) {
        return visit(*inac);
    } else if (const auto* meac = dynamic_cast<const MemberAccessExpression*>(&node)) {
        return visit(*meac);
    } else if (const auto* newEx = dynamic_cast<const NewExpression*>(&node)) {
        return visit(*newEx);
    } else if (const auto* moveEx = dynamic_cast<const MoveExpression*>(&node)) {
        return visit(*moveEx);
    }

    return ErrorReporter::compilationError(std::string(typeid(node).name()));
}

// Statements 
void LLVMCompiler::visit(const ExpressionStatement& node) {
    visit(*node.expression);
}

void LLVMCompiler::visit(const VariableDeclaration& node) {
    // For now the getLLVMType does not consider arrays
    auto varName = node.variable.getValue();

    if (scopeManager.isDeclaredInCurrentScope(varName)) {
        ErrorReporter::variableRedeclaration(varName);
    }

    if (node.type.smartPointerType != SmartPointerType::None) {
        createSmartPointerVariable(node);
        return;
    }
    
    auto varType = LLVMTypeSystem::getLLVMType(context, node.type);
    auto varValue = visit(*node.expr);
    llvm::AllocaInst* ptr = builder.CreateAlloca(varType, nullptr, varName);
    builder.CreateStore(varValue, ptr);
    scopeManager.declare(varName, ptr, node.type);
}

void LLVMCompiler::createSmartPointerVariable(const VariableDeclaration& node) {
    auto varName = node.variable.getValue();
    auto smartPtrType = LLVMTypeSystem::getLLVMType(context, node.type);

    // 1. Allocate the smart pointer struct on the stack
    llvm::AllocaInst* smartPtr = builder.CreateAlloca(smartPtrType, nullptr, varName);

    // 2. Get the value to store (e.g., 42)
    auto value = visit(*node.expr);

    // 3. Allocate memory on heap for the actual value
    llvm::Function* mallocFunc = _module->getFunction("smart_ptr_malloc");
    llvm::Value* heapPtr = builder.CreateCall(mallocFunc, {builder.getInt32(4)}); // sizeof(int)

    // 4. Store the value in heap memory
    builder.CreateStore(value, heapPtr);

    // 5. Store heap pointer in smart pointer struct
    if (node.type.smartPointerType == SmartPointerType::Unique) {
        // For unique_ptr: data is field 0
        llvm::Value* dataFieldPtr = builder.CreateGEP(smartPtrType, smartPtr, {builder.getInt32(0), builder.getInt32(0)});
        builder.CreateStore(heapPtr, dataFieldPtr);

    } else if (node.type.smartPointerType == SmartPointerType::Shared) {
        // For shared_ptr: refcount is field 0, data is field 1
        llvm::Value* refcountPtr = builder.CreateGEP(smartPtrType, smartPtr, {builder.getInt32(0), builder.getInt32(0)});
        builder.CreateStore(builder.getInt32(1), refcountPtr);

        llvm::Value* dataFieldPtr = builder.CreateGEP(smartPtrType, smartPtr, {builder.getInt32(0), builder.getInt32(1)});
        builder.CreateStore(heapPtr, dataFieldPtr);
    }

    // 6. Register the variable in scope
    scopeManager.declare(varName, smartPtr, node.type);
    smartPointersInScope.push_back({smartPtr, node.type.smartPointerType});
}

void LLVMCompiler::visit(const Assignment& node) {
    // TODO: Full implementation needed to handle all LHS expression types:
    // - *ptr = value (dereference assignment)
    // - arr[i] = value (array index assignment)
    // - obj.field = value (member access assignment)

    // For now, handle simple identifier assignment
    auto identifierExpr = dynamic_cast<const IdentifierExpression*>(node.lvalue.get());
    if (identifierExpr) {
        auto varName = identifierExpr->name;
        auto varPtr = scopeManager.lookup(varName);

        if (!varPtr) {
            ErrorReporter::undefinedVariable(varName);
            return;
        }

        auto newValue = visit(*node.rvalue);
        builder.CreateStore(newValue, varPtr);
    } else {
        // Complex LHS assignment not yet implemented
        throw std::runtime_error("Complex assignment LHS not yet implemented in LLVM compiler");
    }
}

void LLVMCompiler::visit(const BlockStatement& node) {
    scopeManager.enterScope();

    for (const auto& stmt : node.statements) {
        visit(*stmt);
    }

    generateSmartPointerCleanup();
    scopeManager.exitScope();
}

void LLVMCompiler::visit(const IfStatement& node) {
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
    llvm::Value* condition = visit(*node.condition);
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
    visit(*node.thenBlock);
    bool thenReturns = thenBlock->getTerminator() != nullptr;
    if (!thenReturns) {
        builder.CreateBr(mergeBlock);
    }

     // ---- ELSE ----
    bool elseReturns = false;
    if (elseBlock) {
        builder.SetInsertPoint(elseBlock);
        visit(*node.elseBlock);
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

void LLVMCompiler::visit(const WhileStatement& node) {
    llvm::Function* currentFunction = builder.GetInsertBlock()->getParent();
    llvm::BasicBlock* conditionBlock = llvm::BasicBlock::Create(context, LLVMLabels::WHILE_CONDITION, currentFunction);
    llvm::BasicBlock* bodyBlock = llvm::BasicBlock::Create(context, LLVMLabels::WHILE_BODY, currentFunction);
    llvm::BasicBlock* exitBlock = llvm::BasicBlock::Create(context, LLVMLabels::WHILE_EXIT, currentFunction);

    builder.CreateBr(conditionBlock);

    builder.SetInsertPoint(conditionBlock);
    llvm::Value* condition = visit(*node.condition);
    if (!condition) return;
    builder.CreateCondBr(condition, bodyBlock, exitBlock);

    builder.SetInsertPoint(bodyBlock);
    visit(*node.body);
    builder.CreateBr(conditionBlock);

    builder.SetInsertPoint(exitBlock);
}

void LLVMCompiler::visit(const FunctionDefinition& node) {
    std::vector<llvm::Type*> paramTypes;
    for (const auto& param : node.parameters) {
        llvm::Type* paramType = LLVMTypeSystem::getLLVMType(context, param.type.getType());
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
        _module.get()
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
            LLVMTypeSystem::getLLVMType(context, param.type.getType()),
            nullptr, 
            param.name.getValue());

        builder.CreateStore(arg, ptr);
        scopeManager.declare(param.name.getValue(), ptr);
    }
    
    visit(*node.body);

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

void LLVMCompiler::visit(const ReturnStatement& node) {
    llvm::Function* currentFunction = nullptr;
    
    if (builder.GetInsertBlock())
        currentFunction = builder.GetInsertBlock()->getParent();

    if (node.returnValue) {
        llvm::Value* returnValue = visit(*node.returnValue);
        // emitDeferredCalls();  // Execute deferred calls before return - COMMENTED OUT FOR DEBUGGING
        generateSmartPointerCleanup();
        builder.CreateRet(returnValue);
    } else {
        // No explicit return value, produce a return matching the function's type
        if (!currentFunction) {
            // should not happen, but guard
            // emitDeferredCalls();  // Execute deferred calls before return - COMMENTED OUT FOR DEBUGGING
            generateSmartPointerCleanup();
            builder.CreateRetVoid();
            return;
        }
        // emitDeferredCalls();  // Execute deferred calls before return - COMMENTED OUT FOR DEBUGGING
        generateSmartPointerCleanup();
        LLVMTypeSystem::setFunctionReturnType(builder, currentFunction);
    }

}

void LLVMCompiler::visit(const ExternStatement& node) {
    // External function declaration: extern def malloc(size: int) -> *void

    // Convert parameter types
    std::vector<llvm::Type*> paramTypes;
    for (const auto& param : node.parameters) {
        llvm::Type* paramType = LLVMTypeSystem::getLLVMType(context, param.type.getType());
        if (!paramType) {
            ErrorReporter::compilationError("Unknown parameter type in extern function: " + param.name.getValue());
            return;
        }
        paramTypes.push_back(paramType);
    }

    // Convert return type
    llvm::Type* returnType = LLVMTypeSystem::getLLVMType(context, node.returnType.getType());
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
        _module.get()
    );

    // Set parameter names (helpful for debugging)
    auto argIt = externFunc->arg_begin();
    for (const auto& param : node.parameters) {
        argIt->setName(param.name.getValue());
        ++argIt;
    }
}

void LLVMCompiler::visit(const DeferStatement& node) {
    // Store the defer statement for later execution at scope end
    // For now, we assume the expression is a function call

    // This is a simplified implementation - in a full implementation,
    // we'd need to evaluate the expression and extract the function call
    ErrorReporter::compilationError("Defer statements not fully implemented yet - placeholder added");
}

void LLVMCompiler::addDeferredCall(llvm::Function* func, std::vector<llvm::Value*> args) {
    deferredCalls.push_back({func, std::move(args)});
}

void LLVMCompiler::emitDeferredCalls() {
    // Execute deferred calls in reverse order (LIFO - Last In, First Out)
    for (auto it = deferredCalls.rbegin(); it != deferredCalls.rend(); ++it) {
        builder.CreateCall(it->function, it->args);
    }
    deferredCalls.clear();
}

llvm::Value* LLVMCompiler::visit(const BinaryExpression& node) {
    llvm::Value* lValue = visit(*node.left);
    llvm::Value* rValue = visit(*node.right);

    if (!lValue || !rValue) {
        return ErrorReporter::compilationError("Failed to evaluate binary expression operands");
    }

    TokenType op = node.operator_.getType();

    // Promote operands to common type if needed
    llvm::Type* resultType = LLVMTypeSystem::inferExpressionType(context, node);

    if (lValue->getType() != resultType) {
        if (resultType->isDoubleTy() && lValue->getType()->isFloatTy()) {
            lValue = builder.CreateFPExt(lValue, resultType);
        } else if (resultType->isFloatTy() && lValue->getType()->isIntegerTy()) {
            lValue = builder.CreateSIToFP(lValue, resultType);
        } else if (resultType->isDoubleTy() && lValue->getType()->isIntegerTy()) {
            lValue = builder.CreateSIToFP(lValue, resultType);
        }
    }

    if (rValue->getType() != resultType) {
        if (resultType->isDoubleTy() && rValue->getType()->isFloatTy()) {
            rValue = builder.CreateFPExt(rValue, resultType);
        } else if (resultType->isFloatTy() && rValue->getType()->isIntegerTy()) {
            rValue = builder.CreateSIToFP(rValue, resultType);
        } else if (resultType->isDoubleTy() && rValue->getType()->isIntegerTy()) {
            rValue = builder.CreateSIToFP(rValue, resultType);
        }
    }

    return BinaryOperationHandler::handleOperation(builder, op, lValue, rValue);
}

llvm::Value* LLVMCompiler::visit(const LiteralExpression& node) {
    const Token& token = node.value;
    switch (token.getType()) {
        case TokenType::NUMBER:
            return LLVMTypeSystem::evaluateConstantNumerical(context, token.getValue());
        case TokenType::STRING:
            return builder.CreateGlobalStringPtr(token.getValue());
        case TokenType::TRUE:
            return llvm::ConstantInt::get(llvm::Type::getInt1Ty(context), 1);
        case TokenType::FALSE:
            return llvm::ConstantInt::get(llvm::Type::getInt1Ty(context), 0);
        default:
            return nullptr;
    }
}

llvm::Value* LLVMCompiler::visit(const UnaryExpression& node) {
    TokenType op = node.operator_.getType();

    // Special handling for address-of operator - need variable address, not value
    if (op == TokenType::BITWISE_AND) {
        if (const auto* identExpr = dynamic_cast<const IdentifierExpression*>(node.operand.get())) {
            if (scopeManager.isVariableMoved(identExpr->name)) {
                ErrorReporter::compilationError("Tried to access address-of-variable after move");
                return nullptr;
            }
            auto ptr = scopeManager.lookup(identExpr->name);
            if (!ptr) {
                ErrorReporter::undefinedVariable(identExpr->name);
                return nullptr;
            }
            return ptr;  // Return the AllocaInst (address), not the loaded value
        } else {
            ErrorReporter::compilationError("Address-of operator can only be applied to variables");
            return nullptr;
        }
    }

    // For all other operators, get the value normally
    llvm::Value* operand = visit(*node.operand);

    switch (op) {
        case TokenType::MINUS:
            if (operand->getType()->isIntegerTy())
                return builder.CreateNeg(operand);
            if (operand->getType()->isFloatingPointTy())
                return builder.CreateFNeg(operand);
            break;

        case TokenType::NOT:
            if (operand->getType()->isIntegerTy(1))
                return builder.CreateNot(operand);
            break;
        case TokenType::MULT:
            // Dereference operator: *ptr (handles both raw pointers and smart pointers)

            // For smart pointer dereferencing, we need to check if the operand is an identifier
            // that refers to a smart pointer variable
            if (const auto* identExpr = dynamic_cast<const IdentifierExpression*>(node.operand.get())) {
                auto varPtr = scopeManager.lookup(identExpr->name);
                if (!varPtr) {
                    ErrorReporter::undefinedVariable(identExpr->name);
                    return nullptr;
                }
                if (scopeManager.isVariableMoved(identExpr->name)) {
                    ErrorReporter::compilationError("Tried to access variable after move");
                    return nullptr;
                }

                // Check if this is a smart pointer variable by examining its allocated type
                if (auto allocaInst = llvm::dyn_cast<llvm::AllocaInst>(varPtr)) {
                    llvm::Type* allocatedType = allocaInst->getAllocatedType();
                    if (auto structType = llvm::dyn_cast<llvm::StructType>(allocatedType)) {
                        std::string typeName = structType->getName().str();
                        if (typeName.find("unique_ptr") != std::string::npos ||
                            typeName.find("shared_ptr") != std::string::npos ||
                            typeName.find("weak_ptr") != std::string::npos) {

                            // Smart pointer dereference: extract data pointer and load the value
                            llvm::Value* dataFieldPtr = builder.CreateGEP(
                                structType, varPtr,
                                {builder.getInt32(0), builder.getInt32(0)},
                                "smart_ptr_data_field"
                            );

                            // Load the data pointer (opaque pointer compatible)
                            llvm::Value* actualDataPtr = builder.CreateLoad(
                                llvm::PointerType::getUnqual(context),
                                dataFieldPtr,
                                "smart_ptr_data"
                            );

                            // Load the actual value from the data pointer
                            // Use proper type inference based on the smart pointer's element type
                            llvm::Type* elementType = llvm::Type::getInt32Ty(context); // Default fallback

                            // Try to infer the element type from the struct type name
                            if (typeName.find("INT") != std::string::npos) {
                                elementType = llvm::Type::getInt32Ty(context);
                            } else if (typeName.find("FLOAT") != std::string::npos) {
                                elementType = llvm::Type::getFloatTy(context);
                            } else if (typeName.find("DOUBLE") != std::string::npos) {
                                elementType = llvm::Type::getDoubleTy(context);
                            } else if (typeName.find("BOOL") != std::string::npos) {
                                elementType = llvm::Type::getInt1Ty(context);
                            }

                            return builder.CreateLoad(elementType, actualDataPtr, "smart_ptr_value");
                        }
                    }
                }
            }

            // For regular pointer dereferencing, evaluate the operand first
            if (operand->getType()->isPointerTy()) {
                // Regular raw pointer dereference
                llvm::Type* elementType = LLVMTypeSystem::inferPointerElementType(context, node);
                if (!elementType) {
                    // Default to int for now, but this should be improved
                    elementType = llvm::Type::getInt32Ty(context);
                }
                return builder.CreateLoad(elementType, operand, "deref");
            } else {
                ErrorReporter::compilationError("Cannot dereference non-pointer type");
                return nullptr;
            }
            break;
    }
    return nullptr;
}

llvm::Value* LLVMCompiler::visit(const IdentifierExpression& node) {
    auto ptr = scopeManager.lookup(node.name);
    if (!ptr) {
        return ErrorReporter::undefinedVariable(node.name);
    }

    llvm::AllocaInst* alloca = llvm::cast<llvm::AllocaInst>(ptr);
    llvm::Type* allocatedType = alloca->getAllocatedType();

    return builder.CreateLoad(
                allocatedType,
                ptr,
                node.name + "_load");
}

llvm::Value* LLVMCompiler::visit(const MemberAccessExpression& node) {
    // Get the object being accessed
    if (const auto* identExpr = dynamic_cast<const IdentifierExpression*>(node.object.get())) {
        // Look up the variable to get its allocation information
        auto varPtr = scopeManager.lookup(identExpr->name);
        if (!varPtr) {
            ErrorReporter::undefinedVariable(identExpr->name);
            return nullptr;
        }

        // Check if this is a smart pointer variable by examining its allocated type
        if (auto allocaInst = llvm::dyn_cast<llvm::AllocaInst>(varPtr)) {
            llvm::Type* allocatedType = allocaInst->getAllocatedType();
            if (auto structType = llvm::dyn_cast<llvm::StructType>(allocatedType)) {
                std::string typeName = structType->getName().str();

                // Check if this is a smart pointer type (unique_ptr, shared_ptr, etc.)
                if (typeName.find("unique_ptr") != std::string::npos ||
                    typeName.find("shared_ptr") != std::string::npos ||
                    typeName.find("weak_ptr") != std::string::npos) {

                    // Smart pointer struct layout: { data_ptr, destructor_ptr, ... }
                    // Get the data pointer (first field - index 0)
                    llvm::Value* dataFieldPtr = builder.CreateGEP(
                        structType, varPtr,
                        {builder.getInt32(0), builder.getInt32(0)},
                        "smart_ptr_data_field"
                    );

                    // Load the actual data pointer (opaque pointer compatible)
                    llvm::Value* actualDataPtr = builder.CreateLoad(
                        llvm::PointerType::getUnqual(context),
                        dataFieldPtr,
                        "smart_ptr_data"
                    );

                    // TODO: For now, just return the data pointer
                    // In a full implementation, we'd need to:
                    // 1. Determine the field offset of memberName in the target type
                    // 2. Generate GEP to access that specific field
                    // 3. Handle method calls vs field access differently

                    return actualDataPtr;
                }
            }
        }
    }

    // For regular (non-smart-pointer) types, this would be standard struct field access
    // TODO: Implement regular struct member access
    ErrorReporter::compilationError("Member access on non-smart-pointer types not yet implemented");
    return nullptr;
}

llvm::Value* LLVMCompiler::visit(const NewExpression& node) {
    // Generate LLVM code for "new value" expressions
    // This creates a heap-allocated value for smart pointers
    return visit(*node.valueExpression);
}

llvm::Value* LLVMCompiler::visit(const MoveExpression& node) {
    if (const auto* identExpr = dynamic_cast<const IdentifierExpression*>(node.operand.get())) {
        std::string varName = identExpr->name;

        if (scopeManager.isVariableMoved(varName)) {
            return ErrorReporter::compilationError("Tried to move already moved variable");
        }

        scopeManager.markVariableAsMoved(varName);
    }

    llvm::Value* operandValue = visit(*node.operand);
    return operandValue;
}

llvm::Value* LLVMCompiler::visit(const IndexAccessExpression& node) {
    return nullptr;
}

llvm::Value* LLVMCompiler::visit(const ArrayLiteralExpression& node)  {
    return nullptr;
}

llvm::Value* LLVMCompiler::visit(const FunctionCall& node) {
    std::string funcName = node.functionName;
    llvm::Function* calledFunction = _module->getFunction(funcName);
    if (!calledFunction) {
        return ErrorReporter::unimplementedFunction(funcName);
    }

    std::vector<llvm::Value*> args;
    for (const auto& arg : node.arguments) {
        llvm::Value* argValue = visit(*arg);  
        if (!argValue) {
            return ErrorReporter::compilationError("Failed to evaluate function argument");
        }
        args.push_back(argValue);
    }

    llvm::Value* result = builder.CreateCall(calledFunction, args);
    return result;
}

void LLVMCompiler::declareRuntimeFunctions() {
    llvm::Type* voidType = llvm::Type::getVoidTy(context);
    llvm::Type* i32Type = llvm::Type::getInt32Ty(context);
    llvm::Type* ptrType = llvm::Type::getInt8PtrTy(context);

    llvm::FunctionType* retainType = llvm::FunctionType::get(
        voidType,           // return type
        {ptrType},         // parameters: void* smart_ptr
        false              // not variadic
    );

    llvm::FunctionType* releaseType = llvm::FunctionType::get(
        voidType,          
        {ptrType},         
        false              
    );

    llvm::FunctionType* useCount = llvm::FunctionType::get(
        i32Type,          
        {ptrType},         
        false              
    );

    llvm::FunctionType* malloc_ = llvm::FunctionType::get(
        ptrType,
        {i32Type},
        false
    );

    // Type-specific function declarations
    // Unique pointer functions
    llvm::Function::Create(releaseType, llvm::Function::ExternalLinkage,
                            "unique_ptr_release", _module.get());

    // Shared pointer functions
    llvm::Function::Create(retainType, llvm::Function::ExternalLinkage,
                            "shared_ptr_retain", _module.get());
    llvm::Function::Create(releaseType, llvm::Function::ExternalLinkage,
                            "shared_ptr_release", _module.get());
    llvm::Function::Create(useCount, llvm::Function::ExternalLinkage,
                            "shared_ptr_use_count", _module.get());

    // Weak pointer functions
    llvm::Function::Create(releaseType, llvm::Function::ExternalLinkage,
                            "weak_ptr_release", _module.get());

    // Common malloc function
    llvm::Function::Create(malloc_, llvm::Function::ExternalLinkage,
                            LLVMLabels::SMART_PTR_MALLOC, _module.get());
}

void LLVMCompiler::generateSmartPointerCleanup() {
    for (auto& [smartPtr, type] : smartPointersInScope) {
        if (type == SmartPointerType::Unique) {
            // Call unique_ptr_release for unique_ptr
            llvm::Function* releaseFunc = _module->getFunction("unique_ptr_release");
            builder.CreateCall(releaseFunc, {smartPtr});
        } else if (type == SmartPointerType::Shared) {
            // Call shared_ptr_release for shared_ptr (handles refcount)
            llvm::Function* releaseFunc = _module->getFunction("shared_ptr_release");
            builder.CreateCall(releaseFunc, {smartPtr});
        } else if (type == SmartPointerType::Weak) {
            // Call weak_ptr_release for weak_ptr
            llvm::Function* releaseFunc = _module->getFunction("weak_ptr_release");
            builder.CreateCall(releaseFunc, {smartPtr});
        }
    }
    smartPointersInScope.clear();
}

llvm::Module* LLVMCompiler::getModule() const {
    return _module.get();
}

void LLVMCompiler::printModule() const {
    _module->print(llvm::outs(), nullptr);
}

void LLVMCompiler::generateObjectFile(const std::string& filename) {
    // Initialize all target info
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    // Get target triple for current machine
    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    _module->setTargetTriple(TargetTriple);

    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);
    if (!Target) {
        throw std::runtime_error("Failed to lookup target: " + Error);
    }

    auto CPU = "generic";
    auto Features = "";
    llvm::TargetOptions opt;
    auto RM = llvm::Optional<llvm::Reloc::Model>();
    auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

    _module->setDataLayout(TargetMachine->createDataLayout());

    // Open output file
    std::error_code EC;
    llvm::raw_fd_ostream dest(filename, EC, llvm::sys::fs::OF_None);
    if (EC) {
        throw std::runtime_error("Could not open file: " + EC.message());
    }

    // Create pass to emit object file
    llvm::legacy::PassManager pass;
    auto FileType = llvm::CGFT_ObjectFile;
    if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        throw std::runtime_error("TargetMachine can't emit a file of this type");
    }

    // Run the pass
    pass.run(*_module);
    dest.flush();
}