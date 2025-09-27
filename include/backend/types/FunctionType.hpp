#pragma once

#include "Type.hpp"
#include "Kind.hpp"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include "llvm/IR/DerivedTypes.h"
#include <optional>
#include <memory>
#include <string>

namespace forge::types {
    class FunctionType: public Type {
        private: 
            std::unique_ptr<Type> returnType_;
            std::vector<std::unique_ptr<Type>> parameterTypes_;
            bool isVariadic_;

        public:
            FunctionType(std::unique_ptr<Type> returnType, std::vector<std::unique_ptr<Type>> parameterTypes, bool isVariadic);

            const Type& getReturnType() const { return *returnType_; }
            const std::vector<std::unique_ptr<Type>>& getParameterTypes() const { return parameterTypes_; }
            bool isVariadic() const { return isVariadic_; }

            Kind getKind() const override { return Kind::Function; }
            std::string toString() const override;
            size_t getSizeBytes() const override;
            llvm::Type* toLLVMType(llvm::LLVMContext& ctx) const override;
            bool isAssignableFrom(const Type& other) const override;
            bool canImplicitlyConvertTo(const Type& other) const override;
            std::optional<std::unique_ptr<Type>> promoteWith(const Type& other) const override;
            bool requiresCleanup() const override { return false; }
            bool isCopyable() const override { return true; }
            bool isMovable() const override { return true; }
            std::unique_ptr<Type> clone() const override;
            ~FunctionType() override = default;
    };
}