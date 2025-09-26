#include "../../include/llvm/ExpressionCodeGenerator.hpp"
#include <iostream>

ExpressionCodeGenerator::ExpressionCodeGenerator(llvm::LLVMContext& context,
                                                llvm::IRBuilder<>& builder,
                                                ScopeManager& scopeManager,
                                                RuntimeFunctionRegistry& rfRegistry,
                                                llvm::Module& _module)
    : context(context), builder(builder), scopeManager(scopeManager), rfRegistry(rfRegistry), _module(_module) {
}


llvm::Value* ExpressionCodeGenerator::generate(const Expression& node) {
    if (const auto* literal = dynamic_cast<const LiteralExpression*>(&node)) {
        return generateLiteral(*literal);
    } else if (const auto* bnry = dynamic_cast<const BinaryExpression*>(&node)) {
        return generateBinary(*bnry);
    } else if (const auto* unry = dynamic_cast<const UnaryExpression*>(&node)) {
        return generateUnary(*unry);
    } else if (const auto* idfr = dynamic_cast<const IdentifierExpression*>(&node)) {
        return generateIdentifier(*idfr);
    } else if (const auto* fncl = dynamic_cast<const FunctionCall*>(&node)) {
        return generateFunctionCall(*fncl);
    } else if (const auto* arlt = dynamic_cast<const ArrayLiteralExpression*>(&node)) {
        return generateArrayLiteral(*arlt);
    } else if (const auto* inac = dynamic_cast<const IndexAccessExpression*>(&node)) {
        return generateIndexAccess(*inac);
    } else if (const auto* meac = dynamic_cast<const MemberAccessExpression*>(&node)) {
        return generateMemberAccess(*meac);
    } else if (const auto* newEx = dynamic_cast<const NewExpression*>(&node)) {
        return generateNew(*newEx);
    } else if (const auto* moveEx = dynamic_cast<const MoveExpression*>(&node)) {
        return generateMove(*moveEx);
    }

    return ErrorReporter::compilationError(std::string(typeid(node).name()));
}

llvm::Value* ExpressionCodeGenerator::generateLiteral(const LiteralExpression& node) {
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
        case TokenType::NULL_:
            return llvm::ConstantPointerNull::get(llvm::Type::getInt8PtrTy(context));
        default:
            return nullptr; 
    }
}

llvm::Value* ExpressionCodeGenerator::generateBinary(const BinaryExpression& node) {
    llvm::Value* lValue = generate(*node.left);
    llvm::Value* rValue = generate(*node.right);

    if (!lValue || !rValue) {
        return ErrorReporter::compilationError("Failed to evaluate binary expression operands");
    }

    TokenType op = node.operator_.getType();

    llvm::Type* resultType = LLVMTypeSystem::inferExpressionType(context, node);

    // Type promotions
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

    // Handle pointer arithmetic specially to pass element type information
    if (lValue->getType()->isPointerTy() && rValue->getType()->isIntegerTy()) {
        // Determine element type for pointer arithmetic
        llvm::Type* elementType = llvm::Type::getInt32Ty(context); // Default to int for now
        // TODO: Improve this to use actual declared pointer type from AST
        return BinaryOperationHandler::handlePointerArithmetic(builder, op, lValue, rValue, elementType);
    } else if (lValue->getType()->isIntegerTy() && rValue->getType()->isPointerTy()) {
        // Handle integer + pointer (commutative for addition)
        if (op == TokenType::PLUS) {
            llvm::Type* elementType = llvm::Type::getInt32Ty(context); // Default to int for now
            return BinaryOperationHandler::handlePointerArithmetic(builder, op, rValue, lValue, elementType);
        }
    }

    return BinaryOperationHandler::handleOperation(builder, op, lValue, rValue);
}

llvm::Value* ExpressionCodeGenerator::generateUnary(const UnaryExpression& node) {

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
    llvm::Value* operand = generate(*node.operand);

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
        // TODO: Move all this into the memorymanager ops.
        // Dereference operator: *ptr (handles both raw pointers and smart pointers)

        // First check if operand is a smart pointer identifier
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

        // For all other cases (including nested dereferences), evaluate the operand first
        // Note: operand was already evaluated at line 114 above
        if (operand && operand->getType()->isPointerTy()) {
            // Regular raw pointer dereference
            llvm::Type* elementType = LLVMTypeSystem::inferPointerElementType(context, node, scopeManager);
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

llvm::Value* ExpressionCodeGenerator::generateIdentifier(const IdentifierExpression& node) {
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

llvm::Value* ExpressionCodeGenerator::generateFunctionCall(const FunctionCall& node) {
    std::string funcName = node.functionName;
    llvm::Function* calledFunction = _module.getFunction(funcName);
    if (!calledFunction) {
        return ErrorReporter::unimplementedFunction(funcName);
    }

    std::vector<llvm::Value*> args;
    for (const auto& arg : node.arguments) {
        llvm::Value* argValue = generate(*arg);  
        if (!argValue) {
            return ErrorReporter::compilationError("Failed to evaluate function argument");
        }
        args.push_back(argValue);
    }

    llvm::Value* result = builder.CreateCall(calledFunction, args);
    return result;
}

llvm::Value* ExpressionCodeGenerator::generateArrayLiteral(const ArrayLiteralExpression& node) {
    return nullptr;
}

llvm::Value* ExpressionCodeGenerator::generateIndexAccess(const IndexAccessExpression& node) {
    return nullptr;
}

llvm::Value* ExpressionCodeGenerator::generateMemberAccess(const MemberAccessExpression& node) {
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

llvm::Value* ExpressionCodeGenerator::generateNew(const NewExpression& node) {
    // Generate LLVM code for "new value" expressions
    // This creates a heap-allocated value for smart pointers
    // Is this it? is there some more code that need to be called here???
    return generate(*node.valueExpression);
}

llvm::Value* ExpressionCodeGenerator::generateMove(const MoveExpression& node) {
    if (const auto* identExpr = dynamic_cast<const IdentifierExpression*>(node.operand.get())) {
        std::string varName = identExpr->name;

        if (scopeManager.isVariableMoved(varName)) {
            return ErrorReporter::compilationError("Tried to move already moved variable");
        }

        scopeManager.markVariableAsMoved(varName);
    }

    llvm::Value* operandValue = generate(*node.operand);
    return operandValue;
}