#include "../../../include/backend/errors/ErrorTypes.hpp"

namespace forge::errors {
    // SourceLocation Implementation
    std::string SourceLocation::toString() const {
        return filename + ":" + std::to_string(line) + ":" + std::to_string(column) + "-" + std::to_string(column + length);
    }

    // CompilerError Implementation
    CompilerError::CompilerError(Level level, std::string message, SourceLocation location) {
        level_ = level;
        message_ = std::move(message);
        location_ = std::move(location);
    }

    std::string CompilerError::formatError() const {
        std::string levelStr;
        switch (level_) {
            case Level::Error: levelStr = "error"; break;
            case Level::Warning: levelStr = "warning"; break;
            case Level::Note: levelStr = "note"; break;
        }
        return location_.toString() + ": " + levelStr + ": " + message_;
    }

    // TypeError Implementation
    TypeError::TypeError(std::string message, SourceLocation location) 
        : CompilerError(Level::Error, std::move(message), std::move(location)) {}

    std::string TypeError::formatError() const {
        return CompilerError::formatError();
    }

    // BorrowError Implementation
    BorrowError::BorrowError(Kind kind, std::string message, SourceLocation location)
        : CompilerError(Level::Error, std::move(message), std::move(location)), kind_(kind) {}


    std::string BorrowError::formatError() const {
        std::string kindStr;
        switch (kind_) {
            case Kind::UseAfterMove: kindStr = "Use After Move"; break;
            case Kind::MutableBorrowWhileImmutableBorrows: kindStr = "Mutable Borrow While Immutable Borrows"; break;
            case Kind::MultipleMutableBorrows: kindStr = "Multiple Mutable Borrows"; break;
            case Kind::LifetimeTooShort: kindStr = "Lifetime Too Short"; break;
            case Kind::InvalidBorrow: kindStr = "Invalid Borrow"; break;
        }
        return CompilerError::formatError() + " [" + kindStr + "]";
    }

    // CodegenError Implementation

    CodegenError::CodegenError(std::string message, SourceLocation location)
        : CompilerError(Level::Error, std::move(message), std::move(location)) {}

    std::string CodegenError::formatError() const {
        return CompilerError::formatError();
    }
}
