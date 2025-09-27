#pragma once

#include <memory>
#include <optional>
#include <llvm/IR/Type.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/DerivedTypes.h>
#include "Type.hpp"
#include "Kind.hpp"

namespace forge::types {
    enum class PointerKind { Unique, Shared, Weak };
    
    class SmartPointerType : public Type {
        private:
            std::unique_ptr<Type> elementType_;
            PointerKind pointerKind_;

        public:
            SmartPointerType(std::unique_ptr<Type> elementType, PointerKind pointerKind);
            
            const Type& getElementType() const { return *elementType_; }
            PointerKind getPointerKind() const { return pointerKind_; }

            // Type interface implementations
            Kind getKind() const override { return Kind::SmartPointer; }
            std::string toString() const override;
            size_t getSizeBytes() const override;
            llvm::Type* toLLVMType(llvm::LLVMContext& ctx) const override;
            bool isAssignableFrom(const Type& other) const override;
            bool canImplicitlyConvertTo(const Type& other) const override;
            std::optional<std::unique_ptr<Type>> promoteWith(const Type& other) const override;
            bool requiresCleanup() const override { return true; }
            bool isCopyable() const override;
            bool isMovable() const override { return true; }
    };
}