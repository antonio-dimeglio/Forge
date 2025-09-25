#include "../../include/parser/ParsedType.hpp"

std::string ParsedType::toString() const {
    std::string result = "";

    // Add smart pointer prefix
    switch (smartPointerType) {
        case SmartPointerType::Unique:
            result += "unique ";
            break;
        case SmartPointerType::Shared:
            result += "shared ";
            break;
        case SmartPointerType::Weak:
            result += "weak ";
            break;
        case SmartPointerType::None:
            break;
    }

    // Add pointer/reference prefixes based on nesting level
    if (isPointer) {
        for (int i = 0; i < nestingLevel; i++) {
            result += "*";
        }
    }
    if (isReference) {
        result += "&";
    }
    if (isMutReference) {
        result += "mut ";
    }

    // Add the primary type
    result += primaryType.getValue();

    // Add type parameters if any (for generics like Array[int])
    if (!typeParameters.empty()) {
        result += "[";
        for (size_t i = 0; i < typeParameters.size(); ++i) {
            result += typeParameters[i].getValue();
            if (i < typeParameters.size() - 1) {
                result += ", ";
            }
        }
        result += "]";
    }

    return result;
}
