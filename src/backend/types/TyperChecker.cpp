#include "../../../include/backend/types/TypeChecker.hpp"

using namespace forge::types;
using namespace forge::errors;
using namespace forge::ast;

TypeChecker::TypeChecker(ErrorReporter& errorReporter)
    : errorReporter_(errorReporter) {}

Result<std::unique_ptr<Type>, TypeError>
TypeChecker::inferExpressionType(const Expression& expr, const SymbolTable& symbols) {
    // TODO: Implement expression type inference
    return Result<std::unique_ptr<Type>, TypeError>::Err(TypeError("Not implemented", expr.getLocation()));
}