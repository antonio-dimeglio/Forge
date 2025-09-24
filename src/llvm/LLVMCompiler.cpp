#include "../../include/llvm/LLVMCompiler.hpp"
#include <iostream>
#include <string>

LLVMCompiler::LLVMCompiler() : context(), builder(context) {
    _module = std::make_unique<llvm::Module>("main", context);
}

std::unique_ptr<llvm::Module> LLVMCompiler::compile(const Statement& ast) {
    ast.accept(*this);
    return std::move(_module);
}

llvm::Type* LLVMCompiler::inferExpressionType(const Expression& expr) {
    if (const auto* literal = dynamic_cast<const LiteralExpression*>(&expr)) {
        // Check what type the literal would create
        const Token& token = literal->value;
        if (token.getType() == TokenType::NUMBER) {
            std::string value = token.getValue();
            if (value[value.size() - 1] == 'f') {
                return llvm::Type::getFloatTy(context);
            } else if (value.find('e') != std::string::npos || value.find('.') !=
std::string::npos) {
                return llvm::Type::getDoubleTy(context);
            }
            return llvm::Type::getInt32Ty(context);
        }
    } else if (const auto* binary = dynamic_cast<const BinaryExpression*>(&expr)) {
        // Binary expressions return the "wider" type of the operands
        llvm::Type* leftType = inferExpressionType(*binary->left);
        llvm::Type* rightType = inferExpressionType(*binary->right);

        // Simple type promotion: double > float > int
        if (leftType->isDoubleTy() || rightType->isDoubleTy()) {
            return llvm::Type::getDoubleTy(context);
        } else if (leftType->isFloatTy() || rightType->isFloatTy()) {
            return llvm::Type::getFloatTy(context);
        }
        return llvm::Type::getInt32Ty(context);
    }

    return llvm::Type::getInt32Ty(context);
}

// UTILITY FUNCTIONS
llvm::Value* LLVMCompiler::evaluateConstantNumerical(std::string value) {
    if (value[value.size() - 1] == 'f') {
        float floatValue = std::stof(value);
        return llvm::ConstantFP::get(llvm::Type::getFloatTy(context), floatValue);
    } else if (value.find('e') != std::string::npos || value.find('.') != std::string::npos) {
        double doubleValue = std::stod(value);
        return llvm::ConstantFP::get(llvm::Type::getDoubleTy(context), doubleValue);
    }
    int intValue = std::stoi(value);
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), intValue);
}

void LLVMCompiler::visit(const Program& node) {
    // Pre-scan to determine the return type
    llvm::Type* returnType = llvm::Type::getInt32Ty(context); // Default to int

    if (!node.statements.empty()) {
        const auto& lastStmt = node.statements.back();
        if (const auto* exprStmt = dynamic_cast<const ExpressionStatement*>(lastStmt.get())) {
            returnType = inferExpressionType(*exprStmt->expression);
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
        std::cerr << "Unimplemented statement type: " << typeid(node).name() << std::endl;
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
    // Add other expression types as needed
    
    std::cerr << "Unknown expression type: " << typeid(node).name() << std::endl;
    return nullptr;
}


// Statements 
void LLVMCompiler::visit(const ExpressionStatement& node) {
    visit(*node.expression);
}

void LLVMCompiler::visit(const VariableDeclaration& node) {
    
}
void LLVMCompiler::visit(const Assignment& node) {
    
}
void LLVMCompiler::visit(const BlockStatement& node) {
    
}
void LLVMCompiler::visit(const IfStatement& node) {
    
}
void LLVMCompiler::visit(const WhileStatement& node) {
    
}
void LLVMCompiler::visit(const FunctionDefinition& node) {
    
}
void LLVMCompiler::visit(const ReturnStatement& node) {
    
}

// Expressions 
llvm::Value* LLVMCompiler::visit(const LiteralExpression& node) {
    const Token& token = node.value;
    switch (token.getType()) {
        case TokenType::NUMBER: {
                return evaluateConstantNumerical(token.getValue());
            }
        case TokenType::STRING: {
                return builder.CreateGlobalStringPtr(token.getValue());
            }
        case TokenType::TRUE: {
                return llvm::ConstantInt::get(llvm::Type::getInt1Ty(context), 1);
            }
        case TokenType::FALSE: {
                return llvm::ConstantInt::get(llvm::Type::getInt1Ty(context), 0);
            }
        default:
            return nullptr; 
    }
}

llvm::Value* LLVMCompiler::visit(const BinaryExpression& node) {
    llvm::Value* lhr = visit(*node.left);
    llvm::Value* rhr = visit(*node.right);

    TokenType op = node.operator_.getType();

    if (lhr->getType()->isIntegerTy(32) && rhr->getType()->isIntegerTy(32)) {
        switch (op) {
            case TokenType::PLUS: return builder.CreateAdd(lhr, rhr); // all operators such as += are missing here
            case TokenType::MINUS: return builder.CreateSub(lhr, rhr);
            case TokenType::MULT: return builder.CreateMul(lhr, rhr);
            case TokenType::DIV: return builder.CreateSDiv(lhr, rhr);  
            case TokenType::BITWISE_AND: return builder.CreateAnd(lhr, rhr);
            case TokenType::BITWISE_OR: return builder.CreateOr(lhr, rhr);
            case TokenType::BITWISE_XOR: return builder.CreateXor(lhr, rhr);
            case TokenType::EQUAL_EQUAL: return builder.CreateICmpEQ(lhr, rhr);
            case TokenType::NOT_EQUAL: return builder.CreateICmpNE(lhr, rhr);
            case TokenType::GREATER: return builder.CreateICmpSGT(lhr, rhr);
            case TokenType::LESS: return builder.CreateICmpSLT(lhr, rhr);
            default:
                return nullptr; // Unsupported;
        }
    }
    if (lhr->getType()->isFloatTy() && rhr->getType()->isFloatTy()) {
        switch (op) {
            case TokenType::PLUS: return builder.CreateFAdd(lhr, rhr); // all operators such as += are missing here
            case TokenType::MINUS: return builder.CreateFSub(lhr, rhr);
            case TokenType::MULT: return builder.CreateFMul(lhr, rhr);
            case TokenType::DIV: return builder.CreateFDiv(lhr, rhr);  
            case TokenType::EQUAL_EQUAL: return builder.CreateFCmpOEQ(lhr, rhr);
            case TokenType::NOT_EQUAL: return builder.CreateFCmpONE(lhr, rhr);
            case TokenType::GREATER: return builder.CreateFCmpOGT(lhr, rhr);
            case TokenType::LESS: return builder.CreateFCmpOLE(lhr, rhr);
            default:
                return nullptr; // Unsupported;
        }
    } 
    // Booleans
    if (lhr->getType()->isIntegerTy(1) && rhr->getType()->isIntegerTy(1)) {
        switch (op) {
            case TokenType::BITWISE_AND: return builder.CreateAnd(lhr, rhr);
            case TokenType::BITWISE_OR: return builder.CreateOr(lhr, rhr);
            case TokenType::BITWISE_XOR: return builder.CreateXor(lhr, rhr);
            case TokenType::EQUAL_EQUAL: return builder.CreateICmpEQ(lhr, rhr);
            case TokenType::NOT_EQUAL: return builder.CreateICmpNE(lhr, rhr);
            case TokenType::GREATER: return builder.CreateICmpSGT(lhr, rhr);
            case TokenType::LESS: return builder.CreateICmpSLT(lhr, rhr);
            default:
                return nullptr; // Unsupported;
        }
    }
    return nullptr; 
}

llvm::Value* LLVMCompiler::visit(const UnaryExpression& node) {
    return nullptr;
}

llvm::Value* LLVMCompiler::visit(const IdentifierExpression& node) {
    return nullptr;
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
    return nullptr;
}