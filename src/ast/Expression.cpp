#include "../../include/ast/Expression.hpp"
#include "../../include/lexer/TokenType.hpp"
#include <sstream>

namespace forge::ast {

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

std::string FunctionCall::toString(int indent) const {
    std::stringstream ss;
    ss << makeIndent(indent) << "FunctionCall: " << functionName;

    if (!typeArguments.empty()) {
        ss << "<";
        for (size_t i = 0; i < typeArguments.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << typeArguments[i].getValue();
        }
        ss << ">";
    }

    ss << "(";
    for (size_t i = 0; i < arguments.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << "\n" << arguments[i]->toString(indent + 2);
    }
    if (!arguments.empty()) ss << "\n" << makeIndent(indent);
    ss << ")";

    return ss.str();
}

std::string ArrayLiteralExpression::toString(int indent) const {
    std::stringstream ss;
    ss << makeIndent(indent) << "ArrayLiteral: [";

    for (size_t i = 0; i < arrayValues.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << "\n" << arrayValues[i]->toString(indent + 2);
    }

    if (!arrayValues.empty()) ss << "\n" << makeIndent(indent);
    ss << "]";

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
    if (isMethodCall) ss << " (method call)";
    ss << "\n";
    ss << makeIndent(indent + 1) << "Object:\n" << object->toString(indent + 2);

    if (!arguments.empty()) {
        ss << "\n" << makeIndent(indent + 1) << "Arguments:";
        for (const auto& arg : arguments) {
            ss << "\n" << arg->toString(indent + 2);
        }
    }

    return ss.str();
}

std::string ObjectInstantiation::toString(int indent) const {
    std::stringstream ss;
    ss << makeIndent(indent) << "ObjectInstantiation: " << className.getValue() << "(";

    for (size_t i = 0; i < arguments.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << "\n" << arguments[i]->toString(indent + 2);
    }

    if (!arguments.empty()) ss << "\n" << makeIndent(indent);
    ss << ")";

    return ss.str();
}

std::string GenericInstantiation::toString(int indent) const {
    std::stringstream ss;
    ss << makeIndent(indent) << "GenericInstantiation: " << className.getValue() << "<";

    for (size_t i = 0; i < typeArguments.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << typeArguments[i].getValue();
    }

    ss << ">(";

    for (size_t i = 0; i < arguments.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << "\n" << arguments[i]->toString(indent + 2);
    }

    if (!arguments.empty()) ss << "\n" << makeIndent(indent);
    ss << ")";

    return ss.str();
}

std::string MoveExpression::toString(int indent) const {
    std::stringstream ss;
    ss << makeIndent(indent) << "MoveExpression:\n";
    ss << makeIndent(indent + 1) << "Operand:\n" << operand->toString(indent + 2);
    return ss.str();
}

std::string NewExpression::toString(int indent) const {
    std::stringstream ss;
    ss << makeIndent(indent) << "NewExpression:\n";
    ss << makeIndent(indent + 1) << "Value:\n" << valueExpression->toString(indent + 2);
    return ss.str();
}

std::string OptionalExpression::toString(int indent) const {
    std::stringstream ss;
    ss << makeIndent(indent) << "OptionalExpression: " << type.getValue() << "\n";
    ss << makeIndent(indent + 1) << "Value:\n" << value->toString(indent + 2);
    return ss.str();
}

} // namespace forge::ast