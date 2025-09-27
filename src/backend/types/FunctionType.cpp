#include "../../../include/backend/types/FunctionType.hpp"

namespace forge::types {
    FunctionType::FunctionType(std::unique_ptr<Type> returnType, std::vector<std::unique_ptr<Type>> parameterTypes, bool isVariadic)
        : returnType_(std::move(returnType)), parameterTypes_(std::move(parameterTypes)), isVariadic_(isVariadic) {}

    std::string FunctionType::toString() const {
        std::string paramStr;
        for (size_t i = 0; i < parameterTypes_.size(); ++i) {
            paramStr += parameterTypes_[i]->toString();
            if (i < parameterTypes_.size() - 1) paramStr += ", ";
        }
        if (isVariadic_) {
            if (!parameterTypes_.empty()) paramStr += ", ";
            paramStr += "...";
        }
        return "fn(" + paramStr + ") -> " + returnType_->toString();
    }

    size_t FunctionType::getSizeBytes() const {
        // Function types themselves don't occupy space like variables
        return 0;
    }

    llvm::Type* FunctionType::toLLVMType(llvm::LLVMContext& ctx) const {
        std::vector<llvm::Type*> llvmParamTypes;
        for (const auto& paramType : parameterTypes_) {
            llvmParamTypes.push_back(paramType->toLLVMType(ctx));
        }
        llvm::Type* llvmReturnType = returnType_->toLLVMType(ctx);
        return llvm::FunctionType::get(llvmReturnType, llvmParamTypes, isVariadic_);
    }

    bool FunctionType::isAssignableFrom(const Type& other) const {
        if (other.getKind() != Kind::Function) return false;
        const FunctionType& otherFunc = static_cast<const FunctionType&>(other);
        if (!returnType_->isAssignableFrom(*otherFunc.returnType_)) return false;
        if (parameterTypes_.size() != otherFunc.parameterTypes_.size()) return false;
        for (size_t i = 0; i < parameterTypes_.size(); ++i) {
            if (!parameterTypes_[i]->isAssignableFrom(*otherFunc.parameterTypes_[i])) return false;
        }
        return isVariadic_ == otherFunc.isVariadic_;
    }

    bool FunctionType::canImplicitlyConvertTo(const Type& other) const {
        // Functions are not implicitly convertible to other types
        return isAssignableFrom(other);
    }

    std::optional<std::unique_ptr<Type>> FunctionType::promoteWith(const Type& other) const {
        if (isAssignableFrom(other)) {
            return clone();
        }
        return std::nullopt;
    }

    std::unique_ptr<Type> FunctionType::clone() const {
        std::vector<std::unique_ptr<Type>> paramClones;
        for (const auto& paramType : parameterTypes_) {
            paramClones.push_back(paramType->clone());
        }
        return std::make_unique<FunctionType>(returnType_->clone(), std::move(paramClones), isVariadic_);
    }

} 
