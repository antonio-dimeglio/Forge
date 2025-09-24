#include "../../include/llvm/LLVMTypeSystem.hpp"

llvm::Type* LLVMTypeSystem::getLLVMType(llvm::LLVMContext& context, TokenType tokenType) {
    switch (tokenType) {
        case TokenType::INT: return llvm::Type::getInt32Ty(context);
        case TokenType::FLOAT: return llvm::Type::getFloatTy(context);
        case TokenType::DOUBLE: return llvm::Type::getDoubleTy(context);
        case TokenType::BOOL: return llvm::Type::getInt1Ty(context);
        default: return nullptr;
    }
}

llvm::Value* LLVMTypeSystem::evaluateConstantNumerical(llvm::LLVMContext& context, std::string value) {
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

llvm::Type* LLVMTypeSystem::inferExpressionType(llvm::LLVMContext& context, const Expression& expr) {
    if (const auto* literal = dynamic_cast<const LiteralExpression*>(&expr)) {
        // Check what type the literal would create
        const Token& token = literal->value;
        if (token.getType() == TokenType::NUMBER) {
            std::string value = token.getValue();
            if (value[value.size() - 1] == 'f') {
                return llvm::Type::getFloatTy(context);
            } else if (value.find('e') != std::string::npos || value.find('.') != std::string::npos) {
                return llvm::Type::getDoubleTy(context);
            }
            return llvm::Type::getInt32Ty(context);
        }
    } else if (const auto* binary = dynamic_cast<const BinaryExpression*>(&expr)) {
        // Binary expressions return the "wider" type of the operands
        llvm::Type* leftType = LLVMTypeSystem::inferExpressionType(context, *binary->left);
        llvm::Type* rightType = LLVMTypeSystem::inferExpressionType(context, *binary->right);

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

bool LLVMTypeSystem::canPromoteType(llvm::Type* from, llvm::Type* to) {
    // Can always promote to same type
    if (from == to) return true;

    // Can promote int to float or double
    if (from->isIntegerTy() && (to->isFloatTy() || to->isDoubleTy())) {
        return true;
    }

    // Can promote float to double
    if (from->isFloatTy() && to->isDoubleTy()) {
        return true;
    }

    return false;
}

llvm::Type* LLVMTypeSystem::getPromotedType(llvm::Type* left, llvm::Type* right) {
    // Type promotion hierarchy: double > float > int > bool
    llvm::LLVMContext& context = left->getContext();

    if (left->isDoubleTy() || right->isDoubleTy()) {
        return llvm::Type::getDoubleTy(context);
    }

    if (left->isFloatTy() || right->isFloatTy()) {
        return llvm::Type::getFloatTy(context);
    }

    if (left->isIntegerTy() || right->isIntegerTy()) {
        // For integer operations, use i32 as default
        return llvm::Type::getInt32Ty(context);
    }

    // Default fallback
    return llvm::Type::getInt32Ty(context);
}

void LLVMTypeSystem::setFunctionReturnType(llvm::IRBuilder<>& builder, llvm::Function* func) {
    llvm::Type* returnType = func->getReturnType();
    if (returnType->isVoidTy()) {
        builder.CreateRetVoid();
    } else if (returnType->isIntegerTy()) {
        builder.CreateRet(llvm::ConstantInt::get(returnType, 0));
    } else if (returnType->isFloatingPointTy()) {
        builder.CreateRet(llvm::ConstantFP::get(returnType, 0.0));
    } else if (returnType->isPointerTy()) {
            builder.CreateRet(llvm::Constant::getNullValue(returnType));
    }
}
