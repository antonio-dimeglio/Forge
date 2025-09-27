#pragma once
#include "Kind.hpp"
#include <string>
#include <optional>
#include <memory>
#include <llvm/IR/Type.h>

namespace forge::types {
    class Type {
        public:
            /*
            * Returns the kind of the type.
            */
            virtual Kind getKind() const = 0;

            /*
            * Returns a string representation of the type.
            */
            virtual std::string toString() const = 0;

            /*
            * Returns the size of the type in bytes.
            */
            virtual size_t getSizeBytes() const = 0;

            /*
            * Converts this type to an LLVM type.
            * ctx: The LLVM context to use for type creation.
            * Returns: A pointer to the corresponding LLVM type.
            */
            virtual llvm::Type* toLLVMType(llvm::LLVMContext& ctx) const = 0;


            /*
                Checks if this type can be assigned from another type.
                other: The other type to check against.
                Returns: true if this type can be assigned from the other type, false otherwise.
            */
            virtual bool isAssignableFrom(const Type& other) const = 0;

            /*
                Checks if this type can be implicitly converted to another type.
                other: The other type to check against.
                Returns: true if this type can be implicitly converted to the other type, false otherwise.
            */
            virtual bool canImplicitlyConvertTo(const Type& other) const = 0;

            /*
                Promotes this type with another type to a common type.
                other: The other type to promote with.
                Returns: A unique pointer to the promoted type if promotion is possible, std::nullopt otherwise.
            */
            virtual std::optional<std::unique_ptr<Type>> promoteWith(const Type& other) const = 0;

            /*
                Determines if this type requires cleanup (e.g., destructors).
                Returns: true if the type requires cleanup, false otherwise.
            */
            virtual bool requiresCleanup() const = 0;
            
            /*
                Determines if this type is copyable.
                Returns: true if the type is copyable, false otherwise.
            */
            virtual bool isCopyable() const = 0;

            /*
                Determines if this type is movable.
                Returns: true if the type is movable, false otherwise.
            */
            virtual bool isMovable() const = 0;
            /* 
                Virtual destructor for proper cleanup of derived classes.
            */

            /*
            * Creates a deep copy of this type.
            * Returns: A unique pointer to a copy of this type.
            */
            virtual std::unique_ptr<Type> clone() const = 0;
            virtual ~Type() = default;
    };
};