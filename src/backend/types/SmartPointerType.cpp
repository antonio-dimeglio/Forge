#include "../../../include/backend/types/SmartPointerType.hpp"

namespace forge::types {
    SmartPointerType::SmartPointerType(std::unique_ptr<Type> elementType, PointerKind pointerKind) {
        elementType_ = std::move(elementType);
        pointerKind_ = pointerKind;
    }

    std::string SmartPointerType::toString() const {
        std::string kindStr;
        switch (pointerKind_) {
            case PointerKind::Unique: kindStr = "Unique"; break;
            case PointerKind::Shared: kindStr = "Shared"; break;
            case PointerKind::Weak:   kindStr = "Weak";   break;
        }
        return "SmartPointer<" + elementType_->toString() + ", " + kindStr + ">";
    }

    size_t SmartPointerType::getSizeBytes() const {
        return sizeof(void*); 
    }

    llvm::Type* SmartPointerType::toLLVMType(llvm::LLVMContext& ctx) const {
        // For simplicity, represent all smart pointers as opaque pointers
        return llvm::PointerType::getUnqual(ctx);
    }


    // - Unique<T> can only be assigned from another Unique<T>
    // - Shared<T> can be assigned from Shared<T> and Unique<T>
    // - Weak<T> can be assigned from Weak<T> and Shared<T>
    bool SmartPointerType::isAssignableFrom(const Type& other) const {
        if (other.getKind() != Kind::SmartPointer) return false;
        const SmartPointerType& otherSP = static_cast<const SmartPointerType&>(other);
        
        if (!elementType_->isAssignableFrom(otherSP.getElementType())) return false;

        switch (pointerKind_) {
            case PointerKind::Unique:
                return otherSP.getPointerKind() == PointerKind::Unique;
            case PointerKind::Shared:
                return otherSP.getPointerKind() == PointerKind::Shared ||
                       otherSP.getPointerKind() == PointerKind::Unique;
            case PointerKind::Weak:
                return otherSP.getPointerKind() == PointerKind::Weak ||
                       otherSP.getPointerKind() == PointerKind::Shared;
        }
        return false; // Should never reach here, unless new PointerKind is added
    }

    bool SmartPointerType::canImplicitlyConvertTo(const Type& other) const {
        if (other.getKind() != Kind::SmartPointer) return false;
        const SmartPointerType& otherSP = static_cast<const SmartPointerType&>(other);
        
        if (!elementType_->canImplicitlyConvertTo(otherSP.getElementType())) return false;

        // Implicit conversions: can this convert to other?
        // This means: can other accept this?
        return otherSP.isAssignableFrom(*this);
    }

    std::optional<std::unique_ptr<Type>> SmartPointerType::promoteWith(const Type& other) const {
        if (other.getKind() != Kind::SmartPointer) return std::nullopt;
        const SmartPointerType& otherSP = static_cast<const SmartPointerType&>(other);
        
        auto promotedElementTypeOpt = elementType_->promoteWith(otherSP.getElementType());
        if (!promotedElementTypeOpt) return std::nullopt;

        // Determine the resulting PointerKind based on promotion rules
        PointerKind resultKind;
        if (pointerKind_ == otherSP.getPointerKind()) {
            resultKind = pointerKind_;
        } else if ((pointerKind_ == PointerKind::Unique && otherSP.getPointerKind() == PointerKind::Shared) ||
                   (pointerKind_ == PointerKind::Shared && otherSP.getPointerKind() == PointerKind::Unique)) {
            resultKind = PointerKind::Shared;
        } else if ((pointerKind_ == PointerKind::Shared && otherSP.getPointerKind() == PointerKind::Weak) ||
                   (pointerKind_ == PointerKind::Weak && otherSP.getPointerKind() == PointerKind::Shared)) {
            resultKind = PointerKind::Weak;
        } else {
            return std::nullopt; // Cannot promote Unique and Weak directly
        }

        return std::make_optional(std::make_unique<SmartPointerType>(std::move(*promotedElementTypeOpt), resultKind));
    }

    bool SmartPointerType::isCopyable() const {
        return pointerKind_ == PointerKind::Shared || pointerKind_ == PointerKind::Weak;
    }

    std::unique_ptr<Type> SmartPointerType::clone() const {
        return std::make_unique<SmartPointerType>(elementType_->clone(), pointerKind_);
    }
}