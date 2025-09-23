#include "../../include/parser/Expression.hpp"
#include "../../include/backends/vm/BytecodeCompiler.hpp"
#include "../../include/lexer/TokenType.hpp"
#include <sstream>

void LiteralExpression::accept(BytecodeCompiler& compiler) const {
    compiler.compileLiteral(*this);
}

void IdentifierExpression::accept(BytecodeCompiler& compiler) const {
    compiler.compileIdentifier(*this);
}

void BinaryExpression::accept(BytecodeCompiler& compiler) const {
    compiler.compileBinary(*this);
}

void UnaryExpression::accept(BytecodeCompiler& compiler) const {
    compiler.compileUnary(*this);
}

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

void FunctionCall::accept(BytecodeCompiler& compiler) const {
    compiler.compileFunctionCall(*this);
}

std::string FunctionCall::toString(int indent) const {
    std::string indentStr = std::string(indent * 2, ' ');
    std::stringstream ss;
    ss << indentStr << "FunctionCall: " << functionName << "()";
    // Later you can add argument printing
    return ss.str();
}

// ============= NEW ARRAY EXPRESSION IMPLEMENTATIONS =============

void ArrayLiteralExpression::accept(BytecodeCompiler& compiler) const {
    compiler.compileArrayLiteral(*this);
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

void IndexAccessExpression::accept(BytecodeCompiler& compiler) const {
    compiler.compileIndexAccess(*this);
}

std::string IndexAccessExpression::toString(int indent) const {
    std::stringstream ss;
    ss << makeIndent(indent) << "IndexAccess:\n";
    ss << makeIndent(indent + 1) << "Array:\n" << array->toString(indent + 2) << "\n";
    ss << makeIndent(indent + 1) << "Index:\n" << index->toString(indent + 2);
    return ss.str();
}

void MemberAccessExpression::accept(BytecodeCompiler& compiler) const {
    compiler.compileMemberAccess(*this);
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
