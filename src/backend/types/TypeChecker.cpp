#include "../../../include/backend/types/TypeChecker.hpp"
#include "../../../include/backend/types/FunctionType.hpp"

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
TypeChecker::analyzeType(const ParsedType& parsedType) {
    if (parsedType.isSimpleType()) {
        switch (parsedType.primaryType.getType()) {
            case TokenType::INT:
            case TokenType::FLOAT:
            case TokenType::BOOL:
            case TokenType::STRING: {
                auto primitivePtr = std::make_unique<PrimitiveType>(parsedType.primaryType.getType());
                return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Ok(std::move(primitivePtr));
            }
            // TODO: Set location for parstedtype
            default:
                return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
                    TypeError("Unknown primitive type: " + parsedType.primaryType.getValue(), SourceLocation()));
        }
    }

    if (parsedType.isPointer) {
        auto baseTypeResult = analyzeType(ParsedType{
            parsedType.primaryType,
            parsedType.typeParameters,
            parsedType.nestingLevel,
            false, // isPointer
            false, // isReference
            false, // isMutReference
            parsedType.isOptional,
            parsedType.smartPointerType
        });

        if (baseTypeResult.isErr()) {
            return baseTypeResult;
        }

        auto pointerTypePtr = std::make_unique<PointerType>(baseTypeResult.unwrap()->clone());
        return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Ok(std::move(pointerTypePtr));
    }

    if (parsedType.isReference || parsedType.isMutReference) {
        auto baseTypeResult = analyzeType(ParsedType{
            parsedType.primaryType,
            parsedType.typeParameters,
            parsedType.nestingLevel,
            false, // isPointer
            false, // isReference
            false, // isMutReference
            parsedType.isOptional,
            parsedType.smartPointerType
        });

        if (baseTypeResult.isErr()) {
            return baseTypeResult;
        }

        bool isMutable = parsedType.isMutReference;
        auto referenceTypePtr = std::make_unique<ReferenceType>(baseTypeResult.unwrap()->clone(), isMutable);
        return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Ok(std::move(referenceTypePtr));
    }

    if (parsedType.isOptional) {
        return inferOptionalType(parsedType);
    }

    if (parsedType.isSmartPointer()) {
        // TODO: implement smart pointer type analysis
        return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
            TypeError("Smart pointer type analysis not yet implemented", SourceLocation()));
    }

    if (!parsedType.typeParameters.empty()) {
        // TODO: Implement generic type analysis
        return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
            TypeError("Generic type analysis not yet implemented", SourceLocation()));
    }

    return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
        TypeError("Unsupported type annotation", SourceLocation()));
}

Result<bool, TypeError>
TypeChecker::areTypesCompatible(const Type& left, const Type& right) {
    if (left.isAssignableFrom(right)) {
        return forge::errors::Result<bool, TypeError>::Ok(true);
    }

    if (isImplicitConversionAllowed(left, right)) {
        return forge::errors::Result<bool, TypeError>::Ok(true);
    }

    return forge::errors::Result<bool, TypeError>::Ok(false);
}

Result<std::unique_ptr<Type>, TypeError>
TypeChecker::findCommonType(const Type& type1, const Type& type2) {
    if (auto promotedOpt = type1.promoteWith(type2)) {
        return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Ok(std::move(promotedOpt.value()));
    }

    // TODO: Handle more complex type hierarchies, generics, etc

    return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
        TypeError("No common type found between " + type1.toString() + " and " + type2.toString(), SourceLocation()));
}

Result<void, TypeError>
TypeChecker::validateAssignment(const Type& target, const Type& source, SourceLocation location) {
    auto compatibilityResult = areTypesCompatible(target, source);
    if (compatibilityResult.isErr()) {
        return forge::errors::Result<void, TypeError>::Err(compatibilityResult.error());
    }
    if (!compatibilityResult.unwrap()) {
        return forge::errors::Result<void, TypeError>::Err(
            TypeError("Type mismatch in assignment. Cannot assign " + source.toString() +
                        " to " + target.toString(), location));
    }
    return forge::errors::Result<void, TypeError>::Ok();
}

Result<bool, TypeError>
TypeChecker::validateFunctionCall(const Type& function, const std::vector<Type*>& args, SourceLocation location) {
    if (function.getKind() != Kind::Function) {
        return forge::errors::Result<bool, TypeError>::Err(
            TypeError("Attempted to call a non-function type: " + function.toString(), location));
    }

    const auto& funcType = static_cast<const FunctionType&>(function);
    const auto& paramTypes = funcType.getParameterTypes();

    // Check parameter count (handle variadic functions)
    if (funcType.isVariadic()) {
        // For variadic functions, we need at least the required parameters
        if (args.size() < paramTypes.size()) {
            return forge::errors::Result<bool, TypeError>::Err(
                TypeError("Argument count mismatch in function call. Expected at least " +
                            std::to_string(paramTypes.size()) +
                            ", got " + std::to_string(args.size()), location));
        }
    } else {
        // For non-variadic functions, parameter count must match exactly
        if (paramTypes.size() != args.size()) {
            return forge::errors::Result<bool, TypeError>::Err(
                TypeError("Argument count mismatch in function call. Expected " +
                            std::to_string(paramTypes.size()) +
                            ", got " + std::to_string(args.size()), location));
        }
    }

    // Validate only the required parameters (variadic args are not type-checked)
    size_t requiredParams = paramTypes.size();
    for (size_t i = 0; i < requiredParams; ++i) {
        auto compatibilityResult = areTypesCompatible(*paramTypes[i], *args[i]);
        if (compatibilityResult.isErr()) {
            return forge::errors::Result<bool, TypeError>::Err(compatibilityResult.error());
        }
        if (!compatibilityResult.unwrap()) {
            return forge::errors::Result<bool, TypeError>::Err(
                TypeError("Type mismatch for argument " + std::to_string(i+1) +
                            ". Expected " + paramTypes[i]->toString() +
                            ", got " + args[i]->toString(), location));
        }
    }

    // For variadic functions, additional arguments beyond required params are allowed
    // and not type-checked (they're handled by the runtime)

    return forge::errors::Result<bool, TypeError>::Ok(true);
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
TypeChecker::inferArrayLiteralExpressionType(const ast::ArrayLiteralExpression& expr, const SymbolTable& symbols) {
    return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
        TypeError("Array literal type inference not yet implemented", expr.getLocation()));
}

Result<std::unique_ptr<Type>, TypeError>
TypeChecker::inferIndexAccessExpressionType(const ast::IndexAccessExpression& expr, const SymbolTable& symbols) {
    return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
        TypeError("Index access type inference not yet implemented", expr.getLocation()));
}

Result<std::unique_ptr<Type>, TypeError>
TypeChecker::inferMemberAccessExpressionType(const ast::MemberAccessExpression& expr, const SymbolTable& symbols) {
    return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
        TypeError("Member access type inference not yet implemented", expr.getLocation()));
}

Result<std::unique_ptr<Type>, TypeError>
TypeChecker::inferIdentifierExpressionType(const ast::IdentifierExpression& expr, const SymbolTable& symbols) {
    if (auto varInfoOpt = symbols.lookup(expr.name)) {
        const auto& symbol = *varInfoOpt.value();
        return Result<std::unique_ptr<Type>, TypeError>::Ok(symbol.type->clone());
    } else {
        return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
            TypeError("Undefined variable: " + expr.name, expr.getLocation()));
    }
}

Result<std::unique_ptr<Type>, TypeError>
TypeChecker::inferBinaryExpressionType(const BinaryExpression& expr, const SymbolTable& symbols) {
    auto lhsTypeResult = inferExpressionType(*expr.left, symbols);
    if (lhsTypeResult.isErr()) {
        return lhsTypeResult;
    }

    auto rhsTypeResult = inferExpressionType(*expr.right, symbols);
    if (rhsTypeResult.isErr()) {
        return rhsTypeResult;
    }

    auto& lhsType = *lhsTypeResult.unwrap();
    auto& rhsType = *rhsTypeResult.unwrap();

    // Handle arithmetic operators
    if (expr.operator_.getType() == TokenType::PLUS ||
        expr.operator_.getType() == TokenType::MINUS ||
        expr.operator_.getType() == TokenType::MULT ||
        expr.operator_.getType() == TokenType::DIV) {

        auto promotedTypeOpt = createPromotedType(lhsType, rhsType);
        if (!promotedTypeOpt.has_value()) {
            return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
                TypeError("Incompatible types for arithmetic operation: " + lhsType.toString() +
                            " and " + rhsType.toString(), expr.getLocation()));
        }

        return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Ok(std::move(promotedTypeOpt.value()));
    }

    // Handle comparison operators
    if (expr.operator_.getType() == TokenType::EQUAL_EQUAL ||
        expr.operator_.getType() == TokenType::NOT_EQUAL ||
        expr.operator_.getType() == TokenType::LESS ||
        expr.operator_.getType() == TokenType::LEQ ||
        expr.operator_.getType() == TokenType::GREATER ||
        expr.operator_.getType() == TokenType::GEQ) {

        auto compatibilityResult = areTypesCompatible(lhsType, rhsType);
        if (compatibilityResult.isErr()) {
            return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(compatibilityResult.error());
        }
        if (!compatibilityResult.unwrap()) {
            return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
                TypeError("Incompatible types for comparison: " + lhsType.toString() +
                            " and " + rhsType.toString(), expr.getLocation()));
        }

        auto boolTypePtr = std::make_unique<PrimitiveType>(TokenType::BOOL);
        return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Ok(std::move(boolTypePtr));
    }

    return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
        TypeError("Unsupported binary operator: " + expr.operator_.getValue(), expr.getLocation()));
}

Result<std::unique_ptr<Type>, TypeError>
TypeChecker::inferUnaryExpressionType(const UnaryExpression& expr, const SymbolTable& symbols) {
    auto opTypeResult = inferExpressionType(*expr.operand, symbols);
    if (opTypeResult.isErr()) {
        return opTypeResult;
    }

    auto& operandType = *opTypeResult.unwrap();
    if (expr.operator_.getType() == TokenType::MINUS) {
        if (operandType.getKind() != Kind::Primitive) {
            return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
                TypeError("Unary '-' operator requires a numeric operand, got: " + operandType.toString(), expr.getLocation()));
        }
        auto primType = static_cast<PrimitiveType&>(operandType);
        if (primType.getPrimitiveKind() != TokenType::INT &&
            primType.getPrimitiveKind() != TokenType::FLOAT &&
            primType.getPrimitiveKind() != TokenType::DOUBLE) {
            return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
                TypeError("Unary '-' operator requires an integer or float operand, got: " + operandType.toString(), expr.getLocation()));
        }
        return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Ok(operandType.clone());
    }

    if (expr.operator_.getType() == TokenType::NOT) {
        if (operandType.getKind() != Kind::Primitive) {
            return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
                TypeError("Unary '!' operator requires a boolean operand, got: " + operandType.toString(), expr.getLocation()));
        }
        auto primType = static_cast<PrimitiveType&>(operandType);
        if (primType.getPrimitiveKind() != TokenType::BOOL) {
            return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
                TypeError("Unary '!' operator requires a boolean operand, got: " + operandType.toString(), expr.getLocation()));
        }
        return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Ok(operandType.clone());
    }

    if (expr.operator_.getType() == TokenType::BITWISE_AND) {
        if (operandType.getKind() != Kind::Primitive && operandType.getKind() != Kind::Pointer) {
            return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
                TypeError("Unary '&' operator requires a variable or pointer operand, got: " + operandType.toString(), expr.getLocation()));
        }
        auto refTypePtr = std::make_unique<ReferenceType>(operandType.clone(), false);
        return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Ok(std::move(refTypePtr));
    }

    if (expr.operator_.getType() == TokenType::MULT) {
        if (operandType.getKind() != Kind::Pointer && operandType.getKind() != Kind::Reference) {
            return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
                TypeError("Unary '*' operator requires a pointer or reference operand, got: " + operandType.toString(), expr.getLocation()));
        }
        if (operandType.getKind() == Kind::Pointer) {
            auto& ptrType = static_cast<PointerType&>(operandType);
            return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Ok(ptrType.getPointeeType().clone());
        } else { // Reference
            auto& refType = static_cast<ReferenceType&>(operandType);
            return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Ok(refType.getPointedType().clone());
        }
    }

    return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
        TypeError("Unsupported unary operator: " + expr.operator_.getValue(), expr.getLocation()));
}

Result<std::unique_ptr<Type>, TypeError>
TypeChecker::inferFunctionCallType(const FunctionCall& expr, const SymbolTable& symbols) {
    // Function lookup 
    auto lookupResult = symbols.lookup(expr.functionName);
    if (!lookupResult.has_value()) {
        return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
            TypeError("Undefined function: " + expr.functionName, expr.getLocation()));
    }

    // The symbol table currently only holds variables, not functions.
    // Therefore we cannot check if the function signatures mathces. 
    // Function handling would require a separate function symbol table.

    return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
        TypeError("Function calls are not yet supported in type inference", expr.getLocation()));   
}

Result<std::unique_ptr<Type>, TypeError>
TypeChecker::inferObjectInstantiationType(const GenericInstantiation& expr, const SymbolTable& symbols) {
    
    // We have no way of looking up classes yet, so we cannot validate the instantiation.
    return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
        TypeError("Object instantiation is not yet supported in type inference", expr.getLocation()));
}

Result<std::unique_ptr<Type>, TypeError>
TypeChecker::inferGenericInstantiationType(const GenericInstantiation& expr, const SymbolTable& symbols){

    // We have no way of looking up classes yet, so we cannot validate the instantiation.
    return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
        TypeError("Generic instantiation is not yet supported in type inference", expr.getLocation()));
}

Result<std::unique_ptr<Type>, TypeError>
TypeChecker::inferMoveExpressionType(const MoveExpression& expr, const SymbolTable& symbols) {
    // Unimplemented: Move semantics and borrow checking are not implemented yet.
    auto operandTypeResult = inferExpressionType(*expr.operand, symbols);
    if (operandTypeResult.isErr()) {
        return operandTypeResult;
    }
    auto& operandType = *operandTypeResult.unwrap();

    if (!operandType.isMovable()) {
        return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
            TypeError("Type is not movable: " + operandType.toString(), expr.getLocation()));
    }
    
    // We cannot yet mark the variable as moved in the symbol table, as we do not track variable states yet.
    // Hence we return an error for now.
    return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
        TypeError("Move semantics are not yet fully supported in type inference", expr.getLocation()));
}

Result<std::unique_ptr<Type>, TypeError>
TypeChecker::inferNewExpressionType(const NewExpression& expr, const SymbolTable& symbols) {

    // Here we do not check what kind of smart pointer we need to create.
    // Why? 
    
    auto valueTypeResult = inferExpressionType(*expr.valueExpression, symbols);
    if (valueTypeResult.isErr()) {
        return valueTypeResult;
    }

    auto& valueType = *valueTypeResult.unwrap();
    auto smartPtrTypePtr = std::make_unique<SmartPointerType>(valueType.clone(), PointerKind::Unique);
    return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Ok(std::move(smartPtrTypePtr));
}

Result<std::unique_ptr<Type>, TypeError>
TypeChecker::inferOptionalType(const ParsedType& parsedType) {
    return forge::errors::Result<std::unique_ptr<Type>, TypeError>::Err(
        TypeError("Optional types are not yet supported in type analysis", SourceLocation()));
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