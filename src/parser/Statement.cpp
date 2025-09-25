#include "../../include/parser/Statement.hpp"
#include "../../include/lexer/TokenType.hpp"
#include <sstream>

std::string VariableDeclaration::toString(int indent) const {
    std::string indentStr = std::string(indent * 2, ' ');
    std::stringstream ss;
    ss << indentStr << "VariableDeclaration: " << variable.getValue()
       << ":" << type.toString() << " = \n"
       << expr->toString(indent + 1);
    return ss.str();
}

std::string Assignment::toString(int indent) const {
    std::string indentStr = std::string(indent * 2, ' ');
    std::stringstream ss;
    ss << indentStr << "Assignment:\n"
       << indentStr << "  LValue:\n" << lvalue->toString(indent + 2) << "\n"
       << indentStr << "  RValue:\n" << rvalue->toString(indent + 2);
    return ss.str();
}

std::string IndexAssignment::toString(int indent) const {
    std::string indentStr = std::string(indent * 2, ' ');
    std::stringstream ss;
    ss << indentStr << "IndexAssignment:\n"
       << indentStr << "  LValue:\n" << lvalue->toString(indent + 2) << "\n"
       << indentStr << "  RValue:\n" << rvalue->toString(indent + 2);
    return ss.str();
}

std::string BlockStatement::toString(int indent) const {
    std::string indentStr = std::string(indent * 2, ' ');
    std::stringstream ss;
    ss << indentStr << "BlockStatement:\n";
    ss << indentStr << "{\n";
    for (const auto& stmt : statements) {
        ss << stmt->toString(indent + 2) << "\n";
    }
    ss << indentStr << "}";
    return ss.str();
}

std::string IfStatement::toString(int indent) const {
    std::string indentStr = std::string(indent * 2, ' ');
    std::stringstream ss;
    ss << indentStr << "IfStatement:\n";
    ss << indentStr << "  Condition:\n" << condition->toString(indent + 2) << "\n";
    ss << indentStr << "  ThenBlock:\n" << thenBlock->toString(indent + 2) << "\n";
    if (elseBlock != nullptr) {
        ss << indentStr << "  ElseBlock:\n" << elseBlock->toString(indent + 2) << "\n";
    }
    return ss.str();
}


std::string WhileStatement::toString(int indent) const {
    std::string indentStr = std::string(indent * 2, ' ');
    std::stringstream ss;
    ss << indentStr << "WhileStatement:\n";
    ss << indentStr << "  Condition:\n" << condition->toString(indent + 2) << "\n";
    ss << indentStr << "  Body:\n" << body->toString(indent + 2) << "\n";
    return ss.str();
}


std::string FunctionDefinition::toString(int indent) const {
    std::string indentStr = std::string(indent * 2, ' ');
    std::stringstream ss;
    ss << indentStr << "FunctionDefinition: " << functionName.getValue();

    if (!typeParameters.empty()) {
        ss << "<";
        for (size_t i = 0; i < typeParameters.size(); ++i) {
            if (i > 0) ss << ", ";
            ss << typeParameters[i].getValue();
        }
        ss << ">";
    }
    ss << "\n";

    ss << indentStr << "  ReturnType: " << functionReturnType.getValue() << "\n";
    ss << indentStr << "  Parameters:\n";
    for (const auto& param : parameters) {
        ss << indentStr << "    " << param.name.getValue() << ": " << param.type.getValue() << "\n";
    }
    ss << indentStr << "  Body:\n" << body->toString(indent + 2) << "\n";
    return ss.str();
}

std::string ReturnStatement::toString(int indent) const {
    std::string indentStr = std::string(indent * 2, ' ');
    std::stringstream ss;
    ss << indentStr << "ReturnStatement:\n";
    ss << indentStr << "  Value:" << returnValue->toString(indent + 2) << "\n";
    return ss.str();
}

std::string ExpressionStatement::toString(int indent) const {
    std::string indentStr = std::string(indent * 2, ' ');
    std::stringstream ss;
    ss << indentStr << "ExpressionStatement:\n"
       << expression->toString(indent + 1);
    return ss.str();
}


std::string Program::toString(int indent) const {
    std::string indentStr = std::string(indent * 2, ' ');
    std::stringstream ss;
    ss << indentStr << "Program:\n";
    for (const auto& stmt : statements) {
        ss << stmt->toString(indent + 1);
        if (&stmt != &statements.back()) {  // Add newline between statements
            ss << "\n";
        }
    }
    return ss.str();
}

std::string ClassDefinition::toString(int indent) const {
    std::string indentStr = std::string(indent * 2, ' ');
    std::stringstream ss;
    ss << indentStr << "ClassDefinition: " << className.getValue();

    if (!genericParameters.empty()) {
        ss << "[";
        for (size_t i = 0; i < genericParameters.size(); ++i) {
            ss << genericParameters[i].getValue();
            if (i < genericParameters.size() - 1) ss << ", ";
        }
        ss << "]";
    }
    ss << "\n";

    if (!fields.empty()) {
        ss << indentStr << "  Fields:\n";
        for (const auto& field : fields) {
            ss << indentStr << "    " << field.name.getValue()
               << ": " << field.type.getValue() << "\n";
        }
    }

    if (!methods.empty()) {
        ss << indentStr << "  Methods:\n";
        for (const auto& method : methods) {
            ss << indentStr << "    " << method.methodName.getValue()
               << "() -> " << method.returnType.getValue() << "\n";
        }
    }

    return ss.str();
}

std::string DeferStatement::toString(int indent) const {
    std::string indentStr = std::string(indent * 2, ' ');
    std::stringstream ss;
    ss << indentStr << "DeferStatement:\n"
       << expression->toString(indent + 1);
    return ss.str();
}

std::string ExternStatement::toString(int indent) const {
    std::string indentStr = std::string(indent * 2, ' ');
    std::stringstream ss;
    ss << indentStr << "ExternStatement: " << functionName.getValue() << "(";

    for (size_t i = 0; i < parameters.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << parameters[i].name.getValue() << ": " << parameters[i].type.getValue();
    }

    ss << ") -> " << returnType.getValue();
    return ss.str();
}