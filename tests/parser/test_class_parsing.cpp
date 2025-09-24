#include <gtest/gtest.h>
#include "../../include/parser/Parser.hpp"
#include "../../include/lexer/Lexer.hpp"

class ClassParsingTest : public ::testing::Test {
protected:
    std::unique_ptr<Statement> parseInput(const std::string& input) {
        Lexer lexer(input);
        auto tokens = lexer.tokenize();
        Parser parser(std::move(tokens));
        return parser.parseStatement();
    }

    std::unique_ptr<Expression> parseExpression(const std::string& input) {
        Lexer lexer(input);
        auto tokens = lexer.tokenize();
        Parser parser(std::move(tokens));
        return parser.parseExpression();
    }
};

TEST_F(ClassParsingTest, BasicClassDefinition) {
    std::string input = R"(
class Person:
    name: string
    age: int

    def __init__(name: string, age: int):
        self.name = name
        self.age = age

    def greet() -> string:
        return "Hello"
)";

    auto stmt = parseInput(input);
    ASSERT_NE(stmt, nullptr);

    // Should be ClassDefinition
    auto classDefPtr = dynamic_cast<const ClassDefinition*>(stmt.get());
    ASSERT_NE(classDefPtr, nullptr);

    // Check class name
    EXPECT_EQ(classDefPtr->className.getValue(), "Person");

    // Check no generic parameters
    EXPECT_TRUE(classDefPtr->genericParameters.empty());

    // Check fields
    EXPECT_EQ(classDefPtr->fields.size(), 2);
    EXPECT_EQ(classDefPtr->fields[0].name.getValue(), "name");
    EXPECT_EQ(classDefPtr->fields[0].type.getType(), TokenType::IDENTIFIER);
    EXPECT_EQ(classDefPtr->fields[1].name.getValue(), "age");

    // Check methods
    EXPECT_EQ(classDefPtr->methods.size(), 2);
    EXPECT_EQ(classDefPtr->methods[0].methodName.getValue(), "__init__");
    EXPECT_EQ(classDefPtr->methods[1].methodName.getValue(), "greet");

    // Check constructor parameters
    EXPECT_EQ(classDefPtr->methods[0].parameters.size(), 2);
    EXPECT_EQ(classDefPtr->methods[0].parameters[0].name.getValue(), "name");
    EXPECT_EQ(classDefPtr->methods[0].parameters[1].name.getValue(), "age");

    // Check greet return type
    EXPECT_EQ(classDefPtr->methods[1].returnType.getType(), TokenType::IDENTIFIER);
}

TEST_F(ClassParsingTest, GenericClassDefinition) {
    std::string input = R"(
class Array[T]:
    data: T*
    size: int

    def __init__(capacity: int):
        self.size = 0

    def get(index: int) -> T:
        return self.data[index]
)";

    auto stmt = parseInput(input);
    ASSERT_NE(stmt, nullptr);

    auto classDefPtr = dynamic_cast<const ClassDefinition*>(stmt.get());
    ASSERT_NE(classDefPtr, nullptr);

    // Check class name
    EXPECT_EQ(classDefPtr->className.getValue(), "Array");

    // Check generic parameters
    EXPECT_EQ(classDefPtr->genericParameters.size(), 1);
    EXPECT_EQ(classDefPtr->genericParameters[0].getValue(), "T");

    // Check fields with generic types
    EXPECT_EQ(classDefPtr->fields.size(), 2);
    EXPECT_EQ(classDefPtr->fields[0].name.getValue(), "data");
    // TODO: Check pointer type T*

    // Check methods
    EXPECT_EQ(classDefPtr->methods.size(), 2);
    EXPECT_EQ(classDefPtr->methods[0].methodName.getValue(), "__init__");
    EXPECT_EQ(classDefPtr->methods[1].methodName.getValue(), "get");

    // Check get method return type is generic T
    // TODO: Check return type is T
}

TEST_F(ClassParsingTest, MultipleGenerics) {
    std::string input = R"(
class Map[K, V]:
    keys: Array[K]
    values: Array[V]

    def put(key: K, value: V):
        pass
)";

    auto stmt = parseInput(input);
    ASSERT_NE(stmt, nullptr);

    auto classDefPtr = dynamic_cast<const ClassDefinition*>(stmt.get());
    ASSERT_NE(classDefPtr, nullptr);

    // Check generic parameters
    EXPECT_EQ(classDefPtr->genericParameters.size(), 2);
    EXPECT_EQ(classDefPtr->genericParameters[0].getValue(), "K");
    EXPECT_EQ(classDefPtr->genericParameters[1].getValue(), "V");

    // Check method with generic parameters
    EXPECT_EQ(classDefPtr->methods[0].methodName.getValue(), "put");
    EXPECT_EQ(classDefPtr->methods[0].parameters.size(), 2);
    // TODO: Check parameter types are K and V
}

TEST_F(ClassParsingTest, ObjectInstantiation) {
    std::string input = "Person(\"Alice\", 25)";

    auto expr = parseExpression(input);
    ASSERT_NE(expr, nullptr);

    // Should be ObjectInstantiation (or FunctionCall with class name)
    auto objInstPtr = dynamic_cast<const ObjectInstantiation*>(expr.get());
    if (!objInstPtr) {
        // Fallback: might be parsed as function call initially
        auto funcCallPtr = dynamic_cast<const FunctionCall*>(expr.get());
        ASSERT_NE(funcCallPtr, nullptr);
        EXPECT_EQ(funcCallPtr->functionName.getValue(), "Person");
        EXPECT_EQ(funcCallPtr->arguments.size(), 2);
        return;
    }

    EXPECT_EQ(objInstPtr->className.getValue(), "Person");
    EXPECT_EQ(objInstPtr->arguments.size(), 2);
}

TEST_F(ClassParsingTest, GenericInstantiation) {
    std::string input = "Array[int](10)";

    auto expr = parseExpression(input);
    ASSERT_NE(expr, nullptr);

    // Should be GenericInstantiation
    auto genInstPtr = dynamic_cast<const GenericInstantiation*>(expr.get());
    ASSERT_NE(genInstPtr, nullptr);

    EXPECT_EQ(genInstPtr->className.getValue(), "Array");
    EXPECT_EQ(genInstPtr->typeArguments.size(), 1);
    EXPECT_EQ(genInstPtr->arguments.size(), 1);
}

TEST_F(ClassParsingTest, FieldAccess) {
    std::string input = "person.name";

    auto expr = parseExpression(input);
    ASSERT_NE(expr, nullptr);

    auto fieldAccessPtr = dynamic_cast<const FieldAccess*>(expr.get());
    ASSERT_NE(fieldAccessPtr, nullptr);

    // Check object and field name
    auto objPtr = dynamic_cast<const Identifier*>(fieldAccessPtr->object.get());
    ASSERT_NE(objPtr, nullptr);
    EXPECT_EQ(objPtr->getValue(), "person");
    EXPECT_EQ(fieldAccessPtr->fieldName.getValue(), "name");
}

TEST_F(ClassParsingTest, MethodCall) {
    std::string input = "person.greet()";

    auto expr = parseExpression(input);
    ASSERT_NE(expr, nullptr);

    auto methodCallPtr = dynamic_cast<const MethodCall*>(expr.get());
    ASSERT_NE(methodCallPtr, nullptr);

    // Check object, method name, and arguments
    auto objPtr = dynamic_cast<const Identifier*>(methodCallPtr->object.get());
    ASSERT_NE(objPtr, nullptr);
    EXPECT_EQ(objPtr->getValue(), "person");
    EXPECT_EQ(methodCallPtr->methodName.getValue(), "greet");
    EXPECT_EQ(methodCallPtr->arguments.size(), 0);
}

TEST_F(ClassParsingTest, ChainedCalls) {
    std::string input = "arr.get(0).toString()";

    auto expr = parseExpression(input);
    ASSERT_NE(expr, nullptr);

    // Should be MethodCall where object is another MethodCall
    auto outerCallPtr = dynamic_cast<const MethodCall*>(expr.get());
    ASSERT_NE(outerCallPtr, nullptr);
    EXPECT_EQ(outerCallPtr->methodName.getValue(), "toString");

    // Inner should be method call on arr
    auto innerCallPtr = dynamic_cast<const MethodCall*>(outerCallPtr->object.get());
    ASSERT_NE(innerCallPtr, nullptr);
    EXPECT_EQ(innerCallPtr->methodName.getValue(), "get");

    // Base should be identifier arr
    auto basePtr = dynamic_cast<const Identifier*>(innerCallPtr->object.get());
    ASSERT_NE(basePtr, nullptr);
    EXPECT_EQ(basePtr->getValue(), "arr");
}