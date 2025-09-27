#pragma once
#include "../errors/ErrorReporter.hpp"
#include "../errors/Result.hpp"
#include "../errors/ErrorTypes.hpp"
#include "../../ast/Expression.hpp"
#include "../../ast/ParsedType.hpp"
#include "../codegen/SymbolTable.hpp"
#include "Type.hpp"

using forge::errors::SourceLocation; 
using forge::errors::Result;
using forge::codegen::SymbolTable;

namespace forge::types {
    class TypeChecker {
        public:
            explicit TypeChecker(errors::ErrorReporter& errorReporter);

            // Type Analysis
            Result<std::unique_ptr<Type>, errors::TypeError>
            inferExpressionType(const ast::Expression& expr, const SymbolTable& symbols);

            Result<std::unique_ptr<Type>, errors::TypeError>
            analyzeType(const ast::ParsedType& parsedType);

            // Compatibility Checking
            Result<bool, errors::TypeError>
            areTypesCompatible(const Type& declared, const Type& actual);

            Result<std::unique_ptr<Type>, errors::TypeError>
            findCommonType(const Type& left, const Type& right);

            // Assignment Validation
            Result<void, errors::TypeError>
            validateAssignment(const Type& target, const Type& source, SourceLocation location);

            Result<void, errors::TypeError>
            validateFunctionCall(const Type& function, const std::vector<Type*>& args, SourceLocation location);

        private:
            errors::ErrorReporter& errorReporter_;

            // Internal helpers
            Result<std::unique_ptr<Type>, errors::TypeError>
            inferBinaryExpressionType(const ast::BinaryExpression& expr, const SymbolTable& symbols);

            Result<std::unique_ptr<Type>, errors::TypeError>
            inferUnaryExpressionType(const ast::UnaryExpression& expr, const SymbolTable& symbols);

            bool isImplicitConversionAllowed(const Type& from, const Type& to);
            std::unique_ptr<Type> createPromotedType(const Type& left, const Type& right);
    };
}