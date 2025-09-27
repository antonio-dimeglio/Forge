#include "../../../include/backend/types/TypeChecker.hpp"

using namespace forge::types;
using namespace forge::errors;
using namespace forge::ast;

TypeChecker::TypeChecker(ErrorReporter& errorReporter)
    : errorReporter_(errorReporter) {}

Result<std::unique_ptr<Type>, TypeError>
TypeChecker::inferExpressionType(const Expression& expr, const SymbolTable& symbols) {
    // Dispatch based on the actual expression type using dynamic_cast
    if (auto literalExpr = dynamic_cast<const LiteralExpression*>(&expr)) {
        return inferLiteralExpressionType(*literalExpr);
    } else if (auto arrayLiteralExpr = dynamic_cast<const ArrayLiteralExpression*>(&expr)) {
        return inferArrayLiteralExpressionType(*arrayLiteralExpr, symbols);
    } else if (auto identifierExpr = dynamic_cast<const IdentifierExpression*>(&expr)) {
        return inferIdentifierExpressionType(*identifierExpr, symbols);
    } else if (auto binaryExpr = dynamic_cast<const BinaryExpression*>(&expr)) {
        return inferBinaryExpressionType(*binaryExpr, symbols);
    } else if (auto unaryExpr = dynamic_cast<const UnaryExpression*>(&expr)) {
        return inferUnaryExpressionType(*unaryExpr, symbols);
    } else if (auto funcCallExpr = dynamic_cast<const FunctionCall*>(&expr)) {
        return inferFunctionCallType(*funcCallExpr, symbols);
    } else if (auto genericInstExpr = dynamic_cast<const GenericInstantiation*>(&expr)) {
        return inferGenericInstantiationType(*genericInstExpr, symbols);
    } else if (auto moveExpr = dynamic_cast<const MoveExpression*>(&expr)) {
        return inferMoveExpressionType(*moveExpr, symbols);
    } else if (auto newExpr = dynamic_cast<const NewExpression*>(&expr)) {
        return inferNewExpressionType(*newExpr, symbols);
    }

    return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
        TypeError("Unsupported expression type for type inference", expr.getLocation()));

}

Result<std::unique_ptr<Type>, TypeError>
TypeChecker::inferLiteralExpressionType(const ast::LiteralExpression& expr) {
    if (expr.value.getType() == TokenType::NUMBER) {
        TokenType numericType = inferNumericType(expr.value.getValue());
        auto primitivePtr = std::make_unique<PrimitiveType>(numericType);
        return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Ok(std::move(primitivePtr));
    } else if (expr.value.getType() == TokenType::STRING) {
        auto primitivePtr = std::make_unique<PrimitiveType>(TokenType::STRING);
        return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Ok(std::move(primitivePtr));
        
    } else if (expr.value.getType() == TokenType::TRUE || expr.value.getType() == TokenType::FALSE) {
        auto primitivePtr = std::make_unique<PrimitiveType>(TokenType::BOOL);
        return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Ok(std::move(primitivePtr));
    }

    return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
        TypeError("Unsupported literal type", expr.getLocation()));
}

Result<std::unique_ptr<Type>, TypeError>
TypeChecker::inferIdentifierExpressionType(const ast::IdentifierExpression& expr, const SymbolTable& symbols) {
    if (auto varInfoOpt = symbols.lookup(expr.name)) {
        const auto& symbol = *varInfoOpt.value();

        if (auto primitiveType = dynamic_cast<const PrimitiveType*>(symbol.type.get())) {
            auto typeCopy = std::make_unique<PrimitiveType>(*primitiveType);
            return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Ok(std::move(typeCopy));
        }
        // TODO: Handle other type cases (ReferenceType, SmartPointerType, etc.)
        return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
            TypeError("Unsupported type for variable: " + expr.name, expr.getLocation()));
    } else {
        return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
            TypeError("Undefined variable: " + expr.name, expr.getLocation()));
    }
}

bool TypeChecker::isImplicitConversionAllowed(const Type& left, const Type& right) {
    return right.canImplicitlyConvertTo(left);
}

std::optional<std::unique_ptr<Type>> TypeChecker::createPromotedType(const Type& left, const Type& right) {
    return left.promoteWith(right);
}

TokenType TypeChecker::inferNumericType(const std::string& value) { 
    if (!value.empty() && value.back() == 'f') {
        return TokenType::FLOAT;
    }
    // Decimal point or scientific notation
    if (value.find('.') != std::string::npos ||
        value.find('e') != std::string::npos ||
        value.find('E') != std::string::npos) {
        return TokenType::DOUBLE;
    }
    // Default to integer
    return TokenType::INT;
}