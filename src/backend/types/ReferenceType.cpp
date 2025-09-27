#include "../../../include/backend/types/ReferenceType.hpp"

namespace forge::types {
    ReferenceType::ReferenceType(std::unique_ptr<Type> pointedType, bool isMutable) {
        pointedType_ = std::move(pointedType);
        isMutable_ = isMutable;
    }
    
    std::string ReferenceType::toString() const {
        return (isMutable_ ? "&mut " : "& ") + pointedType_->toString();
    }
    
    size_t ReferenceType::getSizeBytes() const {
        return sizeof(void*); // Size of a pointer
    }

    llvm::Type* ReferenceType::toLLVMType(llvm::LLVMContext& ctx) const {
        return llvm::PointerType::getUnqual(pointedType_->toLLVMType(ctx));
    }

    bool ReferenceType::isAssignableFrom(const Type& other) const {
        if (other.getKind() != Kind::Reference) return false;
        const ReferenceType& otherRef = static_cast<const ReferenceType&>(other);
        if (isMutable_ && !otherRef.isMutable_) return false; // Can't assign immutable to mutable
        return pointedType_->isAssignableFrom(otherRef.getPointedType());
    }

    bool ReferenceType::canImplicitlyConvertTo(const Type& other) const {
        if (other.getKind() != Kind::Reference) return false;
        const ReferenceType& otherRef = static_cast<const ReferenceType&>(other);
        if (isMutable_ && !otherRef.isMutable_) return false; 
        return pointedType_->canImplicitlyConvertTo(otherRef.getPointedType());
    }

    std::optional<std::unique_ptr<Type>> ReferenceType::promoteWith(const Type& other) const {
        if (other.getKind() != Kind::Reference) return std::nullopt;
        const ReferenceType& otherRef = static_cast<const ReferenceType&>(other);
        if (isMutable_ != otherRef.isMutable_) return std::nullopt; // Mutability must match
        auto promotedPointedTypeOpt = pointedType_->promoteWith(otherRef.getPointedType());
        if (!promotedPointedTypeOpt) return std::nullopt;
        return std::make_optional(std::make_unique<ReferenceType>(std::move(*promotedPointedTypeOpt), isMutable_));
    }
}