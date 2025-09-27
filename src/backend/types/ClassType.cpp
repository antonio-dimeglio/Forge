#include "../../../include/backend/types/ClassType.hpp"

namespace forge::types {
    ClassType::ClassType(std::string name) : name_(std::move(name)) {}

    std::string ClassType::toString() const {
        return "Class<" + name_ + ">";
    }

    size_t ClassType::getSizeBytes() const {
        // For simplicity, assume all class instances are pointers (8 bytes on 64-bit)
        return 8;
    }

    llvm::Type* ClassType::toLLVMType(llvm::LLVMContext& ctx) const {
        // Represent class types as opaque structs in LLVM
        return llvm::StructType::create(ctx, name_);
    }

    bool ClassType::isAssignableFrom(const Type& other) const {
        // Classes are assignable if they are the same type
        if (other.getKind() != Kind::Class) return false;
        const auto& otherClass = static_cast<const ClassType&>(other);
        return name_ == otherClass.name_;
    }

    bool ClassType::canImplicitlyConvertTo(const Type& other) const {
        // No implicit conversions for classes
        return isAssignableFrom(other);
    }

    std::optional<std::unique_ptr<Type>> ClassType::promoteWith(const Type& other) const {
        // No promotions for classes
        if (isAssignableFrom(other)) {
            return std::make_optional(clone());
        }
        return std::nullopt;
    }

    std::unique_ptr<Type> ClassType::clone() const {
        return std::make_unique<ClassType>(name_);
    }
}