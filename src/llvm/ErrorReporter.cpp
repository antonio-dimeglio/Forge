#include "../../include/llvm/ErrorReporter.hpp"
#include <iostream>

// Static member definitions
size_t ErrorReporter::errorCount = 0;
size_t ErrorReporter::warningCount = 0;

llvm::Value* ErrorReporter::undefinedVariable(const std::string& name) {
    std::cerr << "Error: undefined variable '" << name << "'" << "\n";
    errorCount++;
    return nullptr;
}

void ErrorReporter::variableRedeclaration(const std::string& name) {
    std::cerr << "Error: redefined variable '" << name << "'" << "\n";
    errorCount++;
}

llvm::Value* ErrorReporter::typeMismatch(const std::string& expected, const std::string& actual) {
    std::cerr << "Error: type mismatch between " << expected << " and " << actual << "\n";
    errorCount++;
    return nullptr;
}

void ErrorReporter::unsupportedOperation(const std::string& op, const std::string& type) {
    std::cerr << "Error: unsupported operation " << op << " for type " << type << "\n";
    errorCount++;
}

llvm::Value* ErrorReporter::compilationError(const std::string& message) {
    std::cerr << "Error: " << message << "\n";
    errorCount++;
    return nullptr;
}

void ErrorReporter::warning(const std::string& message) {
    std::cerr << "Warning: " << message << "\n";
    warningCount++;
}

// Statistics methods
size_t ErrorReporter::getErrorCount() {
    return errorCount;
}

size_t ErrorReporter::getWarningCount() {
    return warningCount;
}

bool ErrorReporter::hasErrors() {
    return errorCount > 0;
}