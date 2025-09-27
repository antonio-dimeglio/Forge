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
    class ArrayType: public Type {
        private: 
            std::unique_ptr<Type> elementType_;
            std::optional<size_t> size_; // nullopt for dynamic arrays

        public:
            ArrayType(std::unique_ptr<Type> elementType, std::optional<size_t> size);

            const Type& getElementType() const { return *elementType_; }
            std::optional<size_t> getSize() const { return size_; }

            Kind getKind() const override { return Kind::Array; }
            std::string toString() const override;
            size_t getSizeBytes() const override;
            llvm::Type* toLLVMType(llvm::LLVMContext& ctx) const override;
            bool isAssignableFrom(const Type& other) const override;
            bool canImplicitlyConvertTo(const Type& other) const override;
            std::optional<std::unique_ptr<Type>> promoteWith(const Type& other) const override;
            bool requiresCleanup() const override { return true; }
            bool isCopyable() const override { return true; }
            bool isMovable() const override { return true; }
            std::unique_ptr<Type> clone() const override;
            ~ArrayType() override = default;
    };
}