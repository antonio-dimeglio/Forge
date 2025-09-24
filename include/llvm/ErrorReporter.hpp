#pragma once
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <string>

class ErrorReporter {
public:
    // Variable-related errors
    static llvm::Value* undefinedVariable(const std::string& name);
    static void variableRedeclaration(const std::string& name);

    // Type-related errors
    static llvm::Value* typeMismatch(const std::string& expected, const std::string& actual);
    static void unsupportedOperation(const std::string& op, const std::string& type);

    // General errors
    static llvm::Value* compilationError(const std::string& message);
    static void warning(const std::string& message);

    // Statistics
    static size_t getErrorCount();
    static size_t getWarningCount();
    static bool hasErrors();

private:
    static size_t errorCount;
    static size_t warningCount;
};