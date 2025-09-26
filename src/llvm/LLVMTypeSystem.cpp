#include "../../include/llvm/LLVMTypeSystem.hpp"
#include "../../include/llvm/ErrorReporter.hpp"

llvm::Type* LLVMTypeSystem::getLLVMType(llvm::LLVMContext& context, TokenType tokenType) {
    switch (tokenType) {
        case TokenType::INT: return llvm::Type::getInt32Ty(context);
        case TokenType::FLOAT: return llvm::Type::getFloatTy(context);
        case TokenType::DOUBLE: return llvm::Type::getDoubleTy(context);
        case TokenType::BOOL: return llvm::Type::getInt1Ty(context);
        case TokenType::VOID: return llvm::Type::getVoidTy(context);
        default:
            ErrorReporter::compilationError("Runtime error, unsupported type: " + tokenTypeToString(tokenType));
            return nullptr;
    }
}

llvm::Type* LLVMTypeSystem::getLLVMType(llvm::LLVMContext& context, const ParsedType& parsedType) {
      // Handle basic types first - convert IDENTIFIER tokens with type names to TokenType
    TokenType actualType;
    if (parsedType.primaryType.getType() == TokenType::IDENTIFIER) {
        std::string typeName = parsedType.primaryType.getValue();
        if (typeName == "int") actualType = TokenType::INT;
        else if (typeName == "float") actualType = TokenType::FLOAT;
        else if (typeName == "double") actualType = TokenType::DOUBLE;
        else if (typeName == "bool") actualType = TokenType::BOOL;
        else {
            ErrorReporter::compilationError("Unknown type: " + typeName);
            return nullptr;
        }
    } else {
        actualType = parsedType.primaryType.getType();
    }

    // Get the base LLVM type
    llvm::Type* baseType = getLLVMType(context, actualType);

    // Handle pointer wrapping
    if (parsedType.isPointer) {
        // Start with the actual base type, then wrap it in pointer levels
        llvm::Type* result = baseType;
        for (int i = 0; i < parsedType.nestingLevel; i++) {
            result = llvm::PointerType::get(result, 0);
        }
        return result;
    }

    // Handle smart pointers
    switch (parsedType.smartPointerType) {
        case SmartPointerType::Unique:
            return getUniquePointerType(context, actualType);
        case SmartPointerType::Shared:
            return getSharedPointerType(context, actualType);
        case SmartPointerType::Weak:
            return getWeakPointerType(context, actualType);
        case SmartPointerType::None:
            break;
    }

    return baseType;
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

llvm::StructType* LLVMTypeSystem::getUniquePointerType(llvm::LLVMContext& context, TokenType elementType) {
    auto llvmType = getLLVMType(context, elementType);
    auto ptrToElement = llvm::PointerType::get(llvmType, 0);
    auto dstrPtr = llvm::Type::getInt8PtrTy(context); // Void pointer
    std::vector<llvm::Type*> fields = {ptrToElement, dstrPtr};
    return llvm::StructType::create(context, fields, "unique_ptr" + tokenTypeToString(elementType));
}

llvm::StructType* LLVMTypeSystem::getSharedPointerType(llvm::LLVMContext& context, TokenType elementType) {
    auto llvmType = getLLVMType(context, elementType);
    auto ptrToElement = llvm::PointerType::get(llvmType, 0);
    auto refCount = llvm::IntegerType::get(context, 32);
    auto dstrPtr = llvm::Type::getInt8PtrTy(context); // Void pointer
    std::vector<llvm::Type*> fields = {refCount, ptrToElement, dstrPtr};
    return llvm::StructType::create(context, fields, "shared_ptr" + tokenTypeToString(elementType));
}


llvm::StructType* LLVMTypeSystem::getWeakPointerType(llvm::LLVMContext& context, TokenType elementType) {
    llvm::Type* llvmType = getLLVMType(context, elementType);        // T
    llvm::Type* ptrToElement = llvm::PointerType::get(llvmType, 0); // T*
    llvm::Type* refcountPtr = llvm::Type::getInt32PtrTy(context);     // i32* (pointer to refcount)
    llvm::Type* validFlag = llvm::Type::getInt8Ty(context);          // i8 (boolean)

    std::vector<llvm::Type*> fields = {refcountPtr, ptrToElement, validFlag};
    return llvm::StructType::create(context, fields, "weak_ptr" + tokenTypeToString(elementType));
}

llvm::Type* LLVMTypeSystem::inferPointerElementType(llvm::LLVMContext& context, const UnaryExpression& node) {
    // This function will be enhanced with a scope manager parameter in the future
    // For now, we'll implement a basic version that defaults to int
    // TODO: This should analyze the operand's type more carefully
    // For example, if operand is an identifier, look up its declared type

    if (dynamic_cast<const IdentifierExpression*>(node.operand.get())) {
        // TODO: Look up the variable's declared type in symbol table
        // For now, assume int pointer -> int element
        return llvm::Type::getInt32Ty(context);
    }

    // Default fallback
    return llvm::Type::getInt32Ty(context);
}