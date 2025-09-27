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
    class GenericType: public Type {
        private: 
            std::string name_;

        public:
            explicit GenericType(std::string name);

            const std::string& getName() const { return name_; }

            Kind getKind() const override { return Kind::Generic; }
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
            ~GenericType() override = default;
    };
}