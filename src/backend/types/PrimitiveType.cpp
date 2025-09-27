#include "../../../include/backend/types/PrimitiveType.hpp"
#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>

namespace forge::types {
    PrimitiveType::PrimitiveType(TokenType kind) {
        this->primitiveKind_ = kind;
    }

    std::string PrimitiveType::toString() const {
        switch (this->primitiveKind_) {
            case TokenType::INT: return "int";
            case TokenType::FLOAT: return "float";
            case TokenType::DOUBLE: return "double";
            case TokenType::BOOL: return "bool";
            case TokenType::STRING: return "char";
            case TokenType::VOID: return "void";
            default: return "unknown";
        }
    }

    size_t PrimitiveType::getSizeBytes() const {
        switch (this->primitiveKind_) {
            case TokenType::INT: return 4; // Assuming 32-bit int
            case TokenType::FLOAT: return 4; // 32-bit float
            case TokenType::DOUBLE: return 8; // 64-bit double
            case TokenType::BOOL: return 1; // 1 byte for bool
            case TokenType::STRING: return sizeof(char*); // Pointer size
            case TokenType::VOID: return 0; // Void has no size
            default: throw std::runtime_error("Unknown primitive type size");
        }
    }

    /*
        * Converts this type to an LLVM type.
            * ctx: The LLVM context to use for type creation.
            * Returns: A pointer to the corresponding LLVM type.
    */
    llvm::Type* PrimitiveType::toLLVMType(llvm::LLVMContext& ctx) const {
        switch (primitiveKind_) {
            case TokenType::INT:
                return llvm::Type::getInt32Ty(ctx);
            case TokenType::FLOAT:
                return llvm::Type::getFloatTy(ctx);
            case TokenType::DOUBLE:
                return llvm::Type::getDoubleTy(ctx);
            case TokenType::BOOL:
                return llvm::Type::getInt1Ty(ctx);
            case TokenType::STRING:
                // LLVM 15+ opaque pointer style
                return llvm::PointerType::getUnqual(ctx);
            case TokenType::VOID:
                return llvm::Type::getVoidTy(ctx);
            default:
                throw std::runtime_error("Unsupported primitive type for LLVM conversion");
        }
    }

    bool PrimitiveType::isAssignableFrom(const Type& other) const {
        if (other.getKind() != Kind::Primitive) return false;
        const PrimitiveType& otherPrim = static_cast<const PrimitiveType&>(other);
        // Either they are the same type, or the other type can convert to this type
        return this->primitiveKind_ == otherPrim.primitiveKind_ || otherPrim.canImplicitlyConvertTo(*this);
    }

    bool PrimitiveType::canImplicitlyConvertTo(const Type& other) const {
        if (other.getKind() != Kind::Primitive) return false;
        const PrimitiveType& otherPrim = static_cast<const PrimitiveType&>(other);

        // Define implicit conversion rules
        if (this->primitiveKind_ == otherPrim.primitiveKind_) return true;
        if (this->primitiveKind_ == TokenType::INT && otherPrim.primitiveKind_ == TokenType::FLOAT) return true;
        if (this->primitiveKind_ == TokenType::INT && otherPrim.primitiveKind_ == TokenType::DOUBLE) return true;
        if (this->primitiveKind_ == TokenType::FLOAT && otherPrim.primitiveKind_ == TokenType::DOUBLE) return true;
        
        return false;
    }


    std::optional<std::unique_ptr<Type>> PrimitiveType::promoteWith(const Type& other) const {
        if (other.getKind() != Kind::Primitive) return std::nullopt;
        const PrimitiveType& otherPrim = static_cast<const PrimitiveType&>(other);

        // If they are the same type, return a copy of this type
        if (this->primitiveKind_ == otherPrim.primitiveKind_) {
            return std::make_optional(std::make_unique<PrimitiveType>(this->primitiveKind_));
        }

        // Define promotion rules
        if ((this->primitiveKind_ == TokenType::INT && otherPrim.primitiveKind_ == TokenType::FLOAT) ||
            (this->primitiveKind_ == TokenType::FLOAT && otherPrim.primitiveKind_ == TokenType::INT)) {
            return std::make_optional(std::make_unique<PrimitiveType>(TokenType::FLOAT));
        }

        if ((this->primitiveKind_ == TokenType::INT && otherPrim.primitiveKind_ == TokenType::DOUBLE) ||
            (this->primitiveKind_ == TokenType::DOUBLE && otherPrim.primitiveKind_ == TokenType::INT)) {
            return std::make_optional(std::make_unique<PrimitiveType>(TokenType::DOUBLE));
        }

        if ((this->primitiveKind_ == TokenType::FLOAT && otherPrim.primitiveKind_ == TokenType::DOUBLE) ||
            (this->primitiveKind_ == TokenType::DOUBLE && otherPrim.primitiveKind_ == TokenType::FLOAT)) {
            return std::make_optional(std::make_unique<PrimitiveType>(TokenType::DOUBLE));
        }

        // No valid promotion found
        return std::nullopt;
    }

    bool PrimitiveType::requiresCleanup() const {
        // Primitive types don't require cleanup (no heap allocation, no destructors)
        return false;
    }

    bool PrimitiveType::isCopyable() const {
        // All primitive types are copyable
        return true;
    }

    bool PrimitiveType::isMovable() const {
        // All primitive types are movable
        return true;
    }
}