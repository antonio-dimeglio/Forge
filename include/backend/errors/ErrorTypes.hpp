#pragma once

#include <string>

namespace forge::errors {
    struct SourceLocation {
        std::string filename;
        size_t line;
        size_t column;
        size_t length;

        SourceLocation() : filename(""), line(0), column(0), length(0) {}
        SourceLocation(const std::string& filename, size_t line, size_t column, size_t length = 1)
            : filename(filename), line(line), column(column), length(length) {}

        std::string toString() const;
    };

    class CompilerError {
        public:
            enum class Level {
                Error,
                Warning,
                Note
            };

        protected:
            Level level_;
            std::string message_;
            SourceLocation location_;

        public:
            CompilerError(Level level, std::string message, SourceLocation location);

            Level getLevel() const { return level_; }
            const std::string& getMessage() const { return message_; }
            const SourceLocation& getLocation() const { return location_; }
            virtual std::string formatError() const;
    };


    class TypeError : public CompilerError {
        public:
            TypeError(std::string message, SourceLocation location);
            std::string formatError() const override;
    };

    class BorrowError : public CompilerError {
        public:
            enum class Kind {
                UseAfterMove,
                MutableBorrowWhileImmutableBorrows,
                MultipleMutableBorrows,
                LifetimeTooShort,
                InvalidBorrow
            };

            BorrowError(Kind kind, std::string message, SourceLocation location);

            Kind getKind() const { return kind_; }
            std::string formatError() const override;

        private:
            Kind kind_;
    };

    class CodegenError : public CompilerError {
        public:
            CodegenError(std::string message, SourceLocation location);

            std::string formatError() const override;
    };

}