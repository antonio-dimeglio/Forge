#pragma once 

#include "Kind.hpp"
#include "Type.hpp"
#include "../../lexer/TokenType.hpp"
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Type.h>
#include "llvm/IR/Constant.h"



namespace forge::types {
    class PrimitiveType : public Type {
        private:
            TokenType primitiveKind_;


        public:
            explicit PrimitiveType(TokenType kind);


            // Type interface implementations
            Kind getKind() const override { return Kind::Primitive;}
            std::string toString() const override;
            size_t getSizeBytes() const override;
            llvm::Type* toLLVMType(llvm::LLVMContext& ctx) const override;
            bool isAssignableFrom(const Type& other) const override;
            bool canImplicitlyConvertTo(const Type& other) const override;
            std::optional<std::unique_ptr<Type>> promoteWith(const Type& other) const override;
            bool requiresCleanup() const override;
            bool isCopyable() const override;
            bool isMovable() const override;

    };
}