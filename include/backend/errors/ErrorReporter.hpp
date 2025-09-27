#pragma once

#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include "ErrorTypes.hpp"

namespace forge::errors {
    class ErrorReporter {
        public: 
            ErrorReporter() = default;
            void reportError(std::unique_ptr<CompilerError> error);
            void reportTypeError(std::string message, SourceLocation location);
            void reportBorrowError(BorrowError::Kind kind, std::string message, SourceLocation location);
            void reportCodegenError(std::string message, SourceLocation location);
            void reportWarning(std::string message, SourceLocation location);

            // Query Interface
            bool hasErrors() const;
            bool hasWarnings() const;
            size_t getErrorCount() const;
            size_t getWarningCount() const;

            const std::vector<std::unique_ptr<CompilerError>>& getErrors() const { return errors_; }

            // Output
            void printErrors(std::ostream& out) const;
            void printSummary(std::ostream& out) const;

            // Clear state
            void clear();

        private:
            std::vector<std::unique_ptr<CompilerError>> errors_;

            void sortErrorsByLocation();
    };
}