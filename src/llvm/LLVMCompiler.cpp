#include "../../include/llvm/LLVMCompiler.hpp"
#include "../../include/llvm/BinaryOperationHandler.hpp"
#include "../../include/llvm/LLVMTypeSystem.hpp"
#include "../../include/llvm/ErrorReporter.hpp"
#include "../../include/llvm/LLVMLabels.hpp"
#include <iostream>
#include <string>
/*
    TODO: 
        Implement arrays
        Implements classes (?)
        Implement string support for operations such as +
*/

LLVMCompiler::LLVMCompiler() : context(), builder(context) {
    _module = std::make_unique<llvm::Module>(LLVMLabels::MAIN_LABEL, context);
}

std::unique_ptr<llvm::Module> LLVMCompiler::compile(const Statement& ast) {
    ast.accept(*this);
    return std::move(_module);
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
    
    auto varType = LLVMTypeSystem::getLLVMType(context, node.type.getType());
    auto varValue = visit(*node.expr);

    llvm::AllocaInst* ptr = builder.CreateAlloca(varType, nullptr, varName);
    builder.CreateStore(varValue, ptr);

    scopeManager.declare(varName, ptr);
}

void LLVMCompiler::visit(const Assignment& node) {
    auto varName = node.variable.getValue();
    auto varPtr = scopeManager.lookup(varName);

    if (!varPtr) {
        ErrorReporter::undefinedVariable(varName);
        return;
    }

    auto newValue = visit(*node.expr);

    builder.CreateStore(newValue, varPtr);
}

void LLVMCompiler::visit(const BlockStatement& node) {
    scopeManager.enterScope();

    for (const auto& stmt : node.statements) {
        visit(*stmt);
    }

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
        builder.CreateRet(returnValue);
    } else {
        // No explicit return value, produce a return matching the function's type
        if (!currentFunction) {
            // should not happen, but guard
            builder.CreateRetVoid();
            return;
        }
        LLVMTypeSystem::setFunctionReturnType(builder, currentFunction);
    }
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
    llvm::Value* operand = visit(*node.operand);
    TokenType op = node.operator_.getType();

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
    return nullptr;
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