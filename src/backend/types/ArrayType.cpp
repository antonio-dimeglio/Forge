#include "../../../include/backend/types/ArrayType.hpp"

namespace forge::types {
    ArrayType::ArrayType(std::unique_ptr<Type> elementType, std::optional<size_t> size)
        : elementType_(std::move(elementType)), size_(size) {}

    std::string ArrayType::toString() const {
        if (size_.has_value()) {
            return "Array[" + elementType_->toString() + "; " + std::to_string(size_.value()) + "]";
        } else {
            return "Array[" + elementType_->toString() + "]";
        }
    }

    size_t ArrayType::getSizeBytes() const {
        if (size_.has_value()) {
            return elementType_->getSizeBytes() * size_.value();
        } else {
            // Dynamic arrays have a pointer and a size field
            return sizeof(void*) + sizeof(size_t);
        }
    }

    llvm::Type* ArrayType::toLLVMType(llvm::LLVMContext& ctx) const {
        if (size_.has_value()) {
            return llvm::ArrayType::get(elementType_->toLLVMType(ctx), size_.value());
        } else {
            // Dynamic arrays represented as struct { elementType*, size_t }
            std::vector<llvm::Type*> elements = {
                elementType_->toLLVMType(ctx)->getPointerTo(),
                llvm::Type::getInt64Ty(ctx)
            };
            return llvm::StructType::get(ctx, elements);
        }
    }

    bool ArrayType::isAssignableFrom(const Type& other) const {
        if (other.getKind() != Kind::Array) return false;
        const ArrayType& otherArray = static_cast<const ArrayType&>(other);
        if (size_.has_value() != otherArray.size_.has_value()) return false;
        if (size_.has_value() && size_.value() != otherArray.size_.value()) return false;
        return elementType_->isAssignableFrom(otherArray.getElementType());
    }

    bool ArrayType::canImplicitlyConvertTo(const Type& other) const {
        if (other.getKind() != Kind::Array) return false;
        const ArrayType& otherArray = static_cast<const ArrayType&>(other);
        if (size_.has_value() != otherArray.size_.has_value()) return false;
        if (size_.has_value() && size_.value() != otherArray.size_.value()) return false;
        return elementType_->canImplicitlyConvertTo(otherArray.getElementType());
    }

    std::optional<std::unique_ptr<Type>> ArrayType::promoteWith(const Type& other) const {
        if (other.getKind() != Kind::Array) return std::nullopt;
        const ArrayType& otherArray = static_cast<const ArrayType&>(other);
        if (size_.has_value() != otherArray.size_.has_value()) return std::nullopt;
        if (size_.has_value() && size_.value() != otherArray.size_.value()) return std::nullopt;
        auto promotedElementTypeOpt = elementType_->promoteWith(otherArray.getElementType());
        if (!promotedElementTypeOpt.has_value()) return std::nullopt;
        return std::make_optional(std::make_unique<ArrayType>(std::move(promotedElementTypeOpt.value()), size_));
    }

    std::unique_ptr<Type> ArrayType::clone() const {
        return std::make_unique<ArrayType>(elementType_->clone(), size_);
    }
}