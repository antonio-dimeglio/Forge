#include "../../include/parser/Expression.hpp"
#include "../../include/lexer/TokenType.hpp"
#include <sstream>

// Helper function to create indentation
std::string makeIndent(int indent) {
    return std::string(indent * 2, ' ');
}

std::string LiteralExpression::toString(int indent) const {
    std::stringstream ss;
    ss << makeIndent(indent) << "Literal: " << value.getValue()
       << " (" << tokenTypeToString(value.getType()) << ")";
    return ss.str();
}

std::string IdentifierExpression::toString(int indent) const {
    std::stringstream ss;
    ss << makeIndent(indent) << "Identifier: " << name;
    return ss.str();
}

std::string BinaryExpression::toString(int indent) const {
    std::stringstream ss;
    ss << makeIndent(indent) << "BinaryExpression: " << operator_.getValue() << "\n";
    ss << makeIndent(indent + 1) << "Left:\n" << left->toString(indent + 2) << "\n";
    ss << makeIndent(indent + 1) << "Right:\n" << right->toString(indent + 2);
    return ss.str();
}

std::string UnaryExpression::toString(int indent) const {
    std::stringstream ss;
    ss << makeIndent(indent) << "UnaryExpression: " << operator_.getValue() << "\n";
    ss << makeIndent(indent + 1) << "Operand:\n" << operand->toString(indent + 2);
    return ss.str();
}

std::string MoveExpression::toString(int indent) const {
    std::stringstream ss;
    ss << makeIndent(indent) << "MoveExpression: " << moveToken.getValue() << "\n";
    ss << makeIndent(indent + 1) << "Operand:\n" << operand->toString(indent + 2);
    return ss.str();
}

std::string FunctionCall::toString(int indent) const {
    std::string indentStr = std::string(indent * 2, ' ');
    std::stringstream ss;
    ss << indentStr << "FunctionCall: " << functionName;

    if (!typeArguments.empty()) {
        ss << "<";
        for (size_t i = 0; i < typeArguments.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << typeArguments[i].getValue();
        }
        ss << ">";
    }
    ss << "()";

    return ss.str();
}

std::string ArrayLiteralExpression::toString(int indent) const {
    std::stringstream ss;
    ss << makeIndent(indent) << "ArrayLiteral: [";
    if (!arrayValues.empty()) {
        ss << "\n";
        for (size_t i = 0; i < arrayValues.size(); ++i) {
            ss << arrayValues[i]->toString(indent + 1);
            if (i < arrayValues.size() - 1) {
                ss << ",\n";
            } else {
                ss << "\n";
            }
        }
        ss << makeIndent(indent) << "]";
    } else {
        ss << "]";
    }
    return ss.str();
}


std::string IndexAccessExpression::toString(int indent) const {
    std::stringstream ss;
    ss << makeIndent(indent) << "IndexAccess:\n";
    ss << makeIndent(indent + 1) << "Array:\n" << array->toString(indent + 2) << "\n";
    ss << makeIndent(indent + 1) << "Index:\n" << index->toString(indent + 2);
    return ss.str();
}


std::string MemberAccessExpression::toString(int indent) const {
    std::stringstream ss;
    ss << makeIndent(indent) << "MemberAccess: " << memberName;
    if (isMethodCall) {
        ss << "()";
    }
    ss << "\n";
    ss << makeIndent(indent + 1) << "Object:\n" << object->toString(indent + 2);

    if (isMethodCall && !arguments.empty()) {
        ss << "\n" << makeIndent(indent + 1) << "Arguments:";
        for (size_t i = 0; i < arguments.size(); ++i) {
            ss << "\n" << arguments[i]->toString(indent + 2);
        }
    }

    return ss.str();
}

std::string ObjectInstantiation::toString(int indent) const {
    std::stringstream ss;
    ss << makeIndent(indent) << "ObjectInstantiation: " << className.getValue();

    if (!arguments.empty()) {
        ss << "\n" << makeIndent(indent + 1) << "Arguments:";
        for (size_t i = 0; i < arguments.size(); ++i) {
            ss << "\n" << arguments[i]->toString(indent + 2);
        }
    }

    return ss.str();
}

std::string GenericInstantiation::toString(int indent) const {
    std::stringstream ss;
    ss << makeIndent(indent) << "GenericInstantiation: " << className.getValue();

    if (!typeArguments.empty()) {
        ss << "[";
        for (size_t i = 0; i < typeArguments.size(); ++i) {
            ss << typeArguments[i].getValue();
            if (i < typeArguments.size() - 1) ss << ", ";
        }
        ss << "]";
    }

    if (!arguments.empty()) {
        ss << "\n" << makeIndent(indent + 1) << "Arguments:";
        for (size_t i = 0; i < arguments.size(); ++i) {
            ss << "\n" << arguments[i]->toString(indent + 2);
        }
    }

    return ss.str();
}

std::string NewExpression::toString(int indent) const {
    std::stringstream ss;
    ss << makeIndent(indent) << "NewExpression";
    if (valueExpression) {
        ss << "\n" << makeIndent(indent + 1) << "Value:";
        ss << "\n" << valueExpression->toString(indent + 2);
    }
    return ss.str();
}

std::string OptionalExpression::toString(int indent) const {
    std::stringstream ss;

    // if type is TokenType::SOME print the value, otherwise print just None
    ss << type.getValue();
    if (value != nullptr) {
        ss << "(" << value->toString() << ")";
    }
    return ss.str();
}