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
    class ClassType: public Type {
        private: 
            std::string name_;
            // Future: Add fields, methods, generics, etc.

        public:
            explicit ClassType(std::string name);

            const std::string& getName() const { return name_; }

            Kind getKind() const override { return Kind::Class; }
            std::string toString() const override;
            size_t getSizeBytes() const override;
            llvm::Type* toLLVMType(llvm::LLVMContext& ctx) const override;
            bool isAssignableFrom(const Type& other) const override;
            bool canImplicitlyConvertTo(const Type& other) const override;
            std::optional<std::unique_ptr<Type>> promoteWith(const Type& other) const override;
            bool requiresCleanup() const override { return true; }
            bool isCopyable() const override { return false; } // Classes are not copyable by default
            bool isMovable() const override { return true; }  // Classes are movable
            std::unique_ptr<Type> clone() const override;
            ~ClassType() override = default;
    };
}