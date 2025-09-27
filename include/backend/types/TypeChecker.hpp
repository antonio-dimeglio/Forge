#pragma once
#include "../errors/ErrorReporter.hpp"
#include "../errors/Result.hpp"
#include "../errors/ErrorTypes.hpp"
#include "../../ast/Expression.hpp"
#include "../../ast/ParsedType.hpp"
#include "../codegen/SymbolTable.hpp"
#include "../../lexer/TokenType.hpp"
#include "PrimitiveType.hpp"
#include "ReferenceType.hpp"
#include "SmartPointerType.hpp"

using forge::errors::SourceLocation; 
using forge::errors::Result;
using forge::codegen::SymbolTable;

namespace forge::types {
    /**
     * TypeChecker: The core type analysis engine for the Forge compiler.
     *
     * This class is responsible for:
     * - Inferring types of expressions (figuring out what type each expression evaluates to)
     * - Validating type compatibility (ensuring operations are performed on compatible types)
     * - Enforcing type safety rules (preventing unsafe operations at compile time)
     * - Providing detailed error messages when type errors occur
     *
     * The TypeChecker works in close cooperation with the SymbolTable to resolve
     * variable and function types, and with the ErrorReporter to provide meaningful
     * error messages with precise source locations.
     */
    class TypeChecker {
        public:
            /**
             * Constructs a TypeChecker with the given error reporter.
             * @param errorReporter Reference to error reporter for logging type errors
             */
            explicit TypeChecker(errors::ErrorReporter& errorReporter);

            // ==================== PUBLIC TYPE ANALYSIS API ====================

            /**
             * MAIN ENTRY POINT: Infers the type of any expression.
             *
             * This is the primary function called by the compiler to determine what
             * type an expression evaluates to. It dispatches to specialized helper
             * functions based on the actual expression type.
             *
             * Examples:
             * - "hello" -> StringType
             * - 42 + 10 -> IntType (if both operands are int)
             * - arr[0] -> ElementType of arr
             * - obj.field -> Type of field
             *
             * @param expr The expression to analyze
             * @param symbols Symbol table containing variable/function declarations
             * @return Result containing the inferred type, or TypeError on failure
             */
            Result<std::unique_ptr<Type>, errors::TypeError>
            inferExpressionType(const ast::Expression& expr, const SymbolTable& symbols);

            /**
             * Converts a parsed type annotation into a concrete Type object.
             *
             * This function takes type annotations from the source code (like "int",
             * "Array[string]", "*int", "&mut string") and converts them into
             * concrete Type objects that the type system can work with.
             *
             * Examples:
             * - "int" -> PrimitiveType(INT)
             * - "*int" -> PointerType(PrimitiveType(INT))
             * - "Array[string]" -> GenericType("Array", [StringType])
             *
             * @param parsedType The parsed type from source code
             * @return Result containing the concrete Type, or TypeError if invalid
             */
            Result<std::unique_ptr<Type>, errors::TypeError>
            analyzeType(const ast::ParsedType& parsedType);

            // ==================== TYPE COMPATIBILITY CHECKING ====================

            /**
             * Checks if two types are compatible for assignment/comparison.
             *
             * This determines whether a value of 'actual' type can be used where
             * 'declared' type is expected. Considers:
             * - Exact type matches (int == int)
             * - Implicit conversions (int -> float)
             * - Reference compatibility (&int vs &mut int)
             * - Inheritance relationships (Child -> Parent)
             *
             * Examples:
             * - areTypesCompatible(int, int) -> true
             * - areTypesCompatible(int, float) -> false (requires explicit cast)
             * - areTypesCompatible(&int, &mut int) -> false (immutable != mutable)
             *
             * @param declared The expected/target type
             * @param actual The actual/source type being provided
             * @return Result<bool> indicating compatibility, or TypeError
             */
            Result<bool, errors::TypeError>
            areTypesCompatible(const Type& declared, const Type& actual);

            /**
             * Finds the most specific common type between two types.
             *
             * Used for operations like conditional expressions (a ? b : c) or
             * binary operations where both operands need to be promoted to a
             * common type.
             *
             * Examples:
             * - findCommonType(int, int) -> int
             * - findCommonType(int, float) -> float (numeric promotion)
             * - findCommonType(Child, Parent) -> Parent (inheritance)
             * - findCommonType(int, string) -> Error (no common type)
             *
             * @param left First type to consider
             * @param right Second type to consider
             * @return Result containing the common type, or TypeError if none exists
             */
            Result<std::unique_ptr<Type>, errors::TypeError>
            findCommonType(const Type& left, const Type& right);

            // ==================== ASSIGNMENT & CALL VALIDATION ====================

            /**
             * Validates that an assignment operation is type-safe.
             *
             * Checks both type compatibility and memory safety rules:
             * - Type compatibility (can source be assigned to target?)
             * - Borrowing rules (is source borrowed? moved?)
             * - Mutability constraints (assigning to immutable reference?)
             *
             * Examples:
             * - int x = 5; -> OK
             * - int x = "hello"; -> Error (string not assignable to int)
             * - &int x = &mut y; -> Error (mut reference to immutable)
             *
             * @param target The type of the variable being assigned to
             * @param source The type of the value being assigned
             * @param location Source location for error reporting
             * @return Result<void> indicating success, or TypeError on failure
             */
            Result<void, errors::TypeError>
            validateAssignment(const Type& target, const Type& source, SourceLocation location);

            /**
             * Validates that a function call is type-safe.
             *
             * Checks:
             * - Correct number of arguments
             * - Each argument type matches corresponding parameter type
             * - Reference borrowing rules are satisfied
             * - Generic type constraints are met
             *
             * Examples:
             * - printf("hello", 42) with printf(string, ...) -> OK
             * - strlen(42) with strlen(string) -> Error (int not string)
             * - func(&mut x) when x is already borrowed -> Error (borrow conflict)
             *
             * @param function The function type being called
             * @param args Vector of argument types being passed
             * @param location Source location for error reporting
             * @return Result<void> indicating success, or TypeError on failure
             */
            Result<void, errors::TypeError>
            validateFunctionCall(const Type& function, const std::vector<Type*>& args, SourceLocation location);

        private:
            errors::ErrorReporter& errorReporter_;

            // ==================== INTERNAL TYPE INFERENCE HELPERS ====================
            // These functions handle type inference for specific expression types.
            // They are called by inferExpressionType() based on the actual expression type.

            /**
             * Infers types for literal values (numbers, strings, booleans).
             *
             * This is the simplest case - literals have obvious, fixed types:
             * - 42 -> IntType
             * - 3.14 -> FloatType
             * - "hello" -> StringType
             * - true/false -> BoolType
             *
             * @param expr The literal expression to analyze
             * @return Result containing the literal's type (never fails for valid literals)
             */
            Result<std::unique_ptr<Type>, errors::TypeError>
            inferLiteralExpressionType(const ast::LiteralExpression& expr);

            /**
             * Infers types for array literals like [1, 2, 3] or ["a", "b"].
             *
             * Process:
             * 1. Infer type of first element
             * 2. Check all other elements have compatible types
             * 3. Find common element type
             * 4. Return Array[CommonType]
             *
             * Examples:
             * - [1, 2, 3] -> Array[int]
             * - ["a", "b"] -> Array[string]
             * - [1, "a"] -> Error (incompatible element types)
             * - [] -> Array[unknown] (requires context or explicit type)
             *
             * @param expr The array literal expression
             * @param symbols Symbol table for resolving element expressions
             * @return Result containing Array[ElementType] or TypeError
             */
            Result<std::unique_ptr<Type>, errors::TypeError>
            inferArrayLiteralExpressionType(const ast::ArrayLiteralExpression& expr, const SymbolTable& symbols);

            /**
             * Infers types for array/container indexing like arr[i] or map["key"].
             *
             * Process:
             * 1. Infer type of container (arr)
             * 2. Check container supports indexing (Array, Map, etc.)
             * 3. Infer type of index (i)
             * 4. Check index type is valid for container
             * 5. Return element type of container
             *
             * Examples:
             * - int[] arr; arr[0] -> int
             * - Map[string, int] map; map["key"] -> int
             * - string s; s[0] -> char
             * - int x; x[0] -> Error (int not indexable)
             *
             * @param expr The index access expression
             * @param symbols Symbol table for resolving container and index
             * @return Result containing element type or TypeError
             */
            Result<std::unique_ptr<Type>, errors::TypeError>
            inferIndexAccessExpressionType(const ast::IndexAccessExpression& expr, const SymbolTable& symbols);

            /**
             * Infers types for member access like obj.field or obj.method().
             *
             * Process:
             * 1. Infer type of object (obj)
             * 2. Look up member in object's type definition
             * 3. Check access permissions (public/private)
             * 4. If method call, validate argument types
             * 5. Return field type or method return type
             *
             * Examples:
             * - Person p; p.name -> string (if name is string field)
             * - Person p; p.getName() -> string (if getName returns string)
             * - int x; x.field -> Error (int has no fields)
             *
             * @param expr The member access expression
             * @param symbols Symbol table for resolving object and member types
             * @return Result containing member type or TypeError
             */
            Result<std::unique_ptr<Type>, errors::TypeError>
            inferMemberAccessExpressionType(const ast::MemberAccessExpression& expr, const SymbolTable& symbols);

            /**
             * Infers types for variable identifiers by looking them up in symbol table.
             *
             * Process:
             * 1. Look up identifier name in symbol table
             * 2. Check if variable is declared
             * 3. Check if variable is in scope
             * 4. Return the variable's declared type
             *
             * Examples:
             * - int x = 5; -> returns IntType for 'x'
             * - undeclaredVar -> Error (undefined variable)
             * - out-of-scope var -> Error (variable not accessible)
             *
             * @param expr The identifier expression
             * @param symbols Symbol table containing variable declarations
             * @return Result containing variable's type or TypeError
             */
            Result<std::unique_ptr<Type>, errors::TypeError>
            inferIdentifierExpressionType(const ast::IdentifierExpression& expr, const SymbolTable& symbols);

            /**
             * Infers types for binary operations like a + b, x == y, etc.
             *
             * Process:
             * 1. Infer types of left and right operands
             * 2. Check if operator is valid for those types
             * 3. Apply type promotion rules if needed
             * 4. Return result type of the operation
             *
             * Examples:
             * - 5 + 3 -> int (int + int = int)
             * - 5.0 + 3 -> float (float + int = float, with promotion)
             * - "hello" + "world" -> string (string concatenation)
             * - 5 + "hello" -> Error (can't add int and string)
             * - x == y -> bool (comparison always returns bool)
             *
             * @param expr The binary expression
             * @param symbols Symbol table for resolving operand types
             * @return Result containing result type or TypeError
             */
            Result<std::unique_ptr<Type>, errors::TypeError>
            inferBinaryExpressionType(const ast::BinaryExpression& expr, const SymbolTable& symbols);

            /**
             * Infers types for unary operations like -x, !y, &z, *ptr.
             *
             * Process:
             * 1. Infer type of operand
             * 2. Check if unary operator is valid for that type
             * 3. Return appropriate result type
             *
             * Examples:
             * - -5 -> int (negation of int is int)
             * - !true -> bool (logical not of bool is bool)
             * - &x -> &TypeOfX (address-of creates reference)
             * - *ptr -> ElementType (dereference of *T is T)
             * - !5 -> Error (can't apply logical not to int)
             *
             * @param expr The unary expression
             * @param symbols Symbol table for resolving operand type
             * @return Result containing result type or TypeError
             */
            Result<std::unique_ptr<Type>, errors::TypeError>
            inferUnaryExpressionType(const ast::UnaryExpression& expr, const SymbolTable& symbols);

            /**
             * Infers return types for function calls like func(arg1, arg2).
             *
             * Process:
             * 1. Look up function in symbol table
             * 2. Check argument count matches parameter count
             * 3. Infer types of all arguments
             * 4. Check each argument type matches corresponding parameter
             * 5. Return function's return type
             *
             * Examples:
             * - strlen("hello") -> int (if strlen: string -> int)
             * - printf("%d", 42) -> int (variadic function)
             * - unknownFunc() -> Error (function not declared)
             * - func(wrongType) -> Error (argument type mismatch)
             *
             * @param expr The function call expression
             * @param symbols Symbol table for resolving function and argument types
             * @return Result containing function's return type or TypeError
             */
            Result<std::unique_ptr<Type>, errors::TypeError>
            inferFunctionCallType(const ast::FunctionCall& expr, const SymbolTable& symbols);

            /**
             * Infers types for object instantiation like new MyClass(args).
             *
             * Process:
             * 1. Look up class/type in symbol table
             * 2. Find appropriate constructor
             * 3. Check constructor argument types
             * 4. Return instance type of the class
             *
             * Examples:
             * - new Person("John", 25) -> Person
             * - new Array[int](10) -> Array[int]
             * - new UnknownClass() -> Error (class not declared)
             * - new Person(wrongArgs) -> Error (constructor mismatch)
             *
             * @param expr The object instantiation expression
             * @param symbols Symbol table for resolving class and constructor
             * @return Result containing instance type or TypeError
             */
            Result<std::unique_ptr<Type>, errors::TypeError>
            inferObjectInstantiationType(const ast::GenericInstantiation& expr, const SymbolTable& symbols);

            /**
             * Infers types for generic instantiation like Array[int](10).
             *
             * Process:
             * 1. Look up generic type template
             * 2. Validate generic type arguments
             * 3. Check constructor arguments for instantiated type
             * 4. Return fully specialized generic type
             *
             * Examples:
             * - Array[int](10) -> Array[int]
             * - Map[string, int]() -> Map[string, int]
             * - Array[UnknownType]() -> Error (invalid type argument)
             *
             * @param expr The generic instantiation expression
             * @param symbols Symbol table for resolving generic type and arguments
             * @return Result containing specialized type or TypeError
             */
            Result<std::unique_ptr<Type>, errors::TypeError>
            inferGenericInstantiationType(const ast::GenericInstantiation& expr, const SymbolTable& symbols);

            /**
             * Infers types for move expressions like move x.
             *
             * Process:
             * 1. Infer type of operand being moved
             * 2. Check operand is movable (not a reference/borrowed)
             * 3. Mark original variable as moved (for borrow checker)
             * 4. Return same type as operand
             *
             * Examples:
             * - move x -> TypeOfX (transfers ownership)
             * - move borrowed_ref -> Error (can't move borrowed value)
             * - move literal -> Error (can't move temporary)
             *
             * @param expr The move expression
             * @param symbols Symbol table for resolving operand type
             * @return Result containing moved type or TypeError
             */
            Result<std::unique_ptr<Type>, errors::TypeError>
            inferMoveExpressionType(const ast::MoveExpression& expr, const SymbolTable& symbols);

            /**
             * Infers types for new expressions (heap allocation).
             *
             * Process:
             * 1. Infer type of value being allocated
             * 2. Return appropriate smart pointer type
             * 3. Handle different allocation strategies
             *
             * Examples:
             * - new 42 -> unique_ptr[int] (or similar)
             * - new MyClass() -> unique_ptr[MyClass]
             *
             * @param expr The new expression
             * @param symbols Symbol table for resolving value type
             * @return Result containing pointer type or TypeError
             */
            Result<std::unique_ptr<Type>, errors::TypeError>
            inferNewExpressionType(const ast::NewExpression& expr, const SymbolTable& symbols);

            /**
             * Infers types for optional type annotations.
             *
             * Process:
             * 1. Parse the optional type syntax
             * 2. Validate the wrapped type exists
             * 3. Return Option[WrappedType]
             *
             * Examples:
             * - Option[int] -> Optional type wrapping int
             * - Option[UnknownType] -> Error (unknown wrapped type)
             *
             * @param parsedType The parsed optional type
             * @return Result containing optional type or TypeError
             */
            Result<std::unique_ptr<Type>, errors::TypeError>
            inferOptionalType(const ast::ParsedType& parsedType);

            // ==================== TYPE CONVERSION & PROMOTION HELPERS ====================

            /**
             * Checks if an implicit conversion from one type to another is allowed.
             *
             * Implicit conversions are automatically applied by the compiler without
             * explicit casting. Common examples:
             * - int -> float (widening numeric conversion)
             * - Child -> Parent (inheritance/subtyping)
             * - &T -> const &T (adding const qualifier)
             *
             * Examples:
             * - isImplicitConversionAllowed(int, float) -> true
             * - isImplicitConversionAllowed(float, int) -> false (narrowing)
             * - isImplicitConversionAllowed(Child, Parent) -> true
             * - isImplicitConversionAllowed(Parent, Child) -> false (downcasting)
             *
             * @param from Source type
             * @param to Target type
             * @return true if implicit conversion is allowed, false otherwise
             */
            bool isImplicitConversionAllowed(const Type& from, const Type& to);

            /**
             * Creates a promoted type that can hold values of both input types.
             *
             * Used in binary operations where operands have different types.
             * Follows standard type promotion rules:
             * - Numeric promotion (int + float -> float)
             * - Common base type (Child1 + Child2 -> Parent if both inherit from Parent)
             *
             * Examples:
             * - createPromotedType(int, float) -> float
             * - createPromotedType(int8, int32) -> int32
             * - createPromotedType(Child1, Child2) -> Parent (if common base)
             * - createPromotedType(int, string) -> nullptr (no promotion possible)
             *
             * @param left First type to promote
             * @param right Second type to promote
             * @return Promoted type that can hold both, or nullptr if impossible
             */
            std::optional<std::unique_ptr<Type>> createPromotedType(const Type& left, const Type& right);

            /*
                * Infers the numeric type (int, float, double) from a literal string value.
                *
                * Used to determine the appropriate numeric type for literals based on
                * their format:
                * - Integer literals (e.g. "42") -> int
                * - Float literals (e.g. "3.14f") -> float
                * - Double literals (e.g. "2.71828") -> double
                *
                * Examples:
                * - inferNumericType("42") -> INT
                * - inferNumericType("3.14f") -> FLOAT
                * - inferNumericType("2.71828") -> DOUBLE
                *
                * @param value The literal string value
                * @return TokenType representing the inferred numeric type
            */
            TokenType inferNumericType(const std::string& value);
    };
}