#include "../../include/parser/Statement.hpp"
#include "../../include/backends/vm/BytecodeCompiler.hpp"
#include "../../include/backends/vm/RuntimeException.hpp"
#include "../../include/lexer/TokenType.hpp"
#include <sstream>

void VariableDeclaration::accept(BytecodeCompiler& compiler) const {
    compiler.compileVariableDeclaration(*this);
}

std::string VariableDeclaration::toString(int indent) const {
    std::string indentStr = std::string(indent * 2, ' ');
    std::stringstream ss;
    ss << indentStr << "VariableDeclaration: " << variable.getValue()
       << ":" << type.getValue() << " = \n"
       << expr->toString(indent + 1);
    return ss.str();
}

void Assignment::accept(BytecodeCompiler& compiler) const {
    compiler.compileAssignment(*this);
}

std::string Assignment::toString(int indent) const {
    std::string indentStr = std::string(indent * 2, ' ');
    std::stringstream ss;
    ss << indentStr << "Assignment: " << variable.getValue() << " = \n"
       << expr->toString(indent + 1);
    return ss.str();
}

void IfStatement::accept(BytecodeCompiler& compiler) const {
    throw RuntimeException("IfStatement compilation not implemented");
}

std::string IfStatement::toString(int indent) const {
    std::string indentStr = std::string(indent * 2, ' ');
    std::stringstream ss;
    ss << indentStr << "IfStatement:\n";
    ss << indentStr << "  Condition:\n" << condition->toString(indent + 2) << "\n";
    ss << indentStr << "  Body:\n";
    for (const auto& stmt : body) {
        ss << stmt->toString(indent + 2) << "\n";
    }
    if (!elseBody.empty()) {
        ss << indentStr << "  Else:\n";
        for (const auto& stmt : elseBody) {
            ss << stmt->toString(indent + 2) << "\n";
        }
    }
    return ss.str();
}


void ExpressionStatement::accept(BytecodeCompiler& compiler) const {
    expression->accept(compiler);
}

std::string ExpressionStatement::toString(int indent) const {
    std::string indentStr = std::string(indent * 2, ' ');
    std::stringstream ss;
    ss << indentStr << "ExpressionStatement:\n"
       << expression->toString(indent + 1);
    return ss.str();
}


void Program::accept(BytecodeCompiler& compiler) const {
    for (const auto& stmt : statements) {
        stmt->accept(compiler);
    }
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