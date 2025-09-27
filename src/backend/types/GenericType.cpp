#include "../../../include/backend/types/GenericType.hpp"

namespace forge::types {
    GenericType::GenericType(std::string name) : name_(std::move(name)) {}

    std::string GenericType::toString() const {
        return name_;
    }

    size_t GenericType::getSizeBytes() const {
        // Size is not defined for generic types until instantiated
        throw std::runtime_error("Size of generic type is undefined until instantiated");
    }

    llvm::Type* GenericType::toLLVMType(llvm::LLVMContext& ctx) const {
        // Generic types do not have a direct LLVM representation
        throw std::runtime_error("Generic type cannot be directly converted to LLVM type");
    }

    bool GenericType::isAssignableFrom(const Type& other) const {
        // Generic types are not directly assignable; must be specialized first
        return false;
    }

    bool GenericType::canImplicitlyConvertTo(const Type& other) const {
        // No implicit conversions for generic types
        return false;
    }

    std::optional<std::unique_ptr<Type>> GenericType::promoteWith(const Type& other) const {
        // No promotion rules for generic types
        return std::nullopt;
    }

    std::unique_ptr<Type> GenericType::clone() const {
        return std::make_unique<GenericType>(name_);
    }
}