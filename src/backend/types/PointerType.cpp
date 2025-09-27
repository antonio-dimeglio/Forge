#include "../../../include/backend/types/PointerType.hpp"

namespace forge::types {
    PointerType::PointerType(std::unique_ptr<Type> pointeeType)
        : pointeeType_(std::move(pointeeType)) {}

    std::string PointerType::toString() const {
        return "*" + pointeeType_->toString();
    }

    size_t PointerType::getSizeBytes() const {
        // Assuming 64-bit pointers
        return 8;
    }

    llvm::Type* PointerType::toLLVMType(llvm::LLVMContext& ctx) const {
        return llvm::PointerType::getUnqual(pointeeType_->toLLVMType(ctx));
    }

    bool PointerType::isAssignableFrom(const Type& other) const {
        if (other.getKind() != Kind::Pointer) return false;
        const PointerType& otherPtr = static_cast<const PointerType&>(other);
        return this->pointeeType_->isAssignableFrom(otherPtr.getPointeeType());
    }

    bool PointerType::canImplicitlyConvertTo(const Type& other) const {
        if (other.getKind() != Kind::Pointer) return false;
        const PointerType& otherPtr = static_cast<const PointerType&>(other);
        return this->pointeeType_->canImplicitlyConvertTo(otherPtr.getPointeeType());
    }

    std::optional<std::unique_ptr<Type>> PointerType::promoteWith(const Type& other) const {
        if (other.getKind() != Kind::Pointer) return std::nullopt;
        const PointerType& otherPtr = static_cast<const PointerType&>(other);

        auto promotedPointeeOpt = this->pointeeType_->promoteWith(otherPtr.getPointeeType());
        if (!promotedPointeeOpt.has_value()) {
            return std::nullopt;
        }

        return std::make_optional(std::make_unique<PointerType>(std::move(promotedPointeeOpt.value())));
    }

    std::unique_ptr<Type> PointerType::clone() const {
        return std::make_unique<PointerType>(this->pointeeType_->clone());
    }
}