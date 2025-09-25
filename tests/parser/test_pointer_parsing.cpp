#include <gtest/gtest.h>
#include "../../include/parser/Parser.hpp"
#include "../../include/parser/ParserException.hpp"
#include "../../include/parser/Expression.hpp"
#include "../../include/parser/Statement.hpp"
#include "../../include/lexer/Tokenizer.hpp"

class PointerParsingTest : public ::testing::Test {
protected:
    Parser parser{{}};

    void SetUp() override {
        // Called before each test
    }

    std::unique_ptr<Statement> parseStatement(const std::string& source) {
        Tokenizer tokenizer(source);
        auto tokens = tokenizer.tokenize();
        parser.reset(tokens);
        auto program = parser.parseProgram();
        auto programStmt = dynamic_cast<Program*>(program.get());
        if (!programStmt || programStmt->statements.empty()) return nullptr;
        return std::move(programStmt->statements[0]);
    }

    std::unique_ptr<Expression> parseExpression(const std::string& source) {
        auto stmt = parseStatement(source);
        auto exprStmt = dynamic_cast<ExpressionStatement*>(stmt.get());
        if (!exprStmt) return nullptr;
        return std::move(exprStmt->expression);
    }

    // For type parsing, we'll need to use function parameters or variable declarations
    std::unique_ptr<Statement> parseVariableWithType(const std::string& varDecl) {
        return parseStatement(varDecl);
    }
};

// ============= MOVE EXPRESSION TESTS =============

TEST_F(PointerParsingTest, MoveExpressionSimple) {
    auto expr = parseExpression("move player");

    ASSERT_NE(expr, nullptr);
    auto moveExpr = dynamic_cast<MoveExpression*>(expr.get());
    ASSERT_NE(moveExpr, nullptr);

    EXPECT_EQ(moveExpr->moveToken.getType(), TokenType::MOVE);
    EXPECT_EQ(moveExpr->moveToken.getValue(), "move");

    // The operand should be an identifier expression
    auto operand = dynamic_cast<IdentifierExpression*>(moveExpr->operand.get());
    ASSERT_NE(operand, nullptr);
    EXPECT_EQ(operand->name, "player");
}

TEST_F(PointerParsingTest, MoveExpressionComplexOperand) {
    auto expr = parseExpression("move arr[0]");

    ASSERT_NE(expr, nullptr);
    auto moveExpr = dynamic_cast<MoveExpression*>(expr.get());
    ASSERT_NE(moveExpr, nullptr);

    // The operand should be an index access expression
    auto operand = dynamic_cast<IndexAccessExpression*>(moveExpr->operand.get());
    ASSERT_NE(operand, nullptr);
}

TEST_F(PointerParsingTest, MoveExpressionInAssignment) {
    auto stmt = parseStatement("player2 = move player1");

    ASSERT_NE(stmt, nullptr);
    auto assignment = dynamic_cast<Assignment*>(stmt.get());
    ASSERT_NE(assignment, nullptr);

    // The right side should be a move expression
    auto moveExpr = dynamic_cast<MoveExpression*>(assignment->expr.get());
    ASSERT_NE(moveExpr, nullptr);
}

// ============= ADDRESS-OF AND DEREFERENCE TESTS =============

TEST_F(PointerParsingTest, AddressOfOperator) {
    auto expr = parseExpression("&variable");

    ASSERT_NE(expr, nullptr);
    auto unaryExpr = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_NE(unaryExpr, nullptr);

    EXPECT_EQ(unaryExpr->operator_.getType(), TokenType::BITWISE_AND);
    EXPECT_EQ(unaryExpr->operator_.getValue(), "&");

    auto operand = dynamic_cast<IdentifierExpression*>(unaryExpr->operand.get());
    ASSERT_NE(operand, nullptr);
    EXPECT_EQ(operand->name, "variable");
}

TEST_F(PointerParsingTest, DereferenceOperator) {
    auto expr = parseExpression("*pointer");

    ASSERT_NE(expr, nullptr);
    auto unaryExpr = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_NE(unaryExpr, nullptr);

    EXPECT_EQ(unaryExpr->operator_.getType(), TokenType::MULT);
    EXPECT_EQ(unaryExpr->operator_.getValue(), "*");

    auto operand = dynamic_cast<IdentifierExpression*>(unaryExpr->operand.get());
    ASSERT_NE(operand, nullptr);
    EXPECT_EQ(operand->name, "pointer");
}

TEST_F(PointerParsingTest, ComplexPointerExpression) {
    auto expr = parseExpression("*(&variable + 1)");

    ASSERT_NE(expr, nullptr);
    auto dereferenceExpr = dynamic_cast<UnaryExpression*>(expr.get());
    ASSERT_NE(dereferenceExpr, nullptr);
    EXPECT_EQ(dereferenceExpr->operator_.getType(), TokenType::MULT);

    // The operand should be a binary expression (address-of + 1)
    auto binaryExpr = dynamic_cast<BinaryExpression*>(dereferenceExpr->operand.get());
    ASSERT_NE(binaryExpr, nullptr);
    EXPECT_EQ(binaryExpr->operator_.getType(), TokenType::PLUS);

    // Left side should be address-of
    auto addressOfExpr = dynamic_cast<UnaryExpression*>(binaryExpr->left.get());
    ASSERT_NE(addressOfExpr, nullptr);
    EXPECT_EQ(addressOfExpr->operator_.getType(), TokenType::BITWISE_AND);
}

// ============= TYPE INFERENCE ASSIGNMENT TESTS =============

TEST_F(PointerParsingTest, TypeInferenceBasic) {
    auto stmt = parseStatement("number := 42");

    ASSERT_NE(stmt, nullptr);
    auto varDecl = dynamic_cast<VariableDeclaration*>(stmt.get());
    ASSERT_NE(varDecl, nullptr);

    EXPECT_EQ(varDecl->variable.getValue(), "number");
    EXPECT_EQ(varDecl->type.getValue(), "auto");  // Placeholder type

    auto literalExpr = dynamic_cast<LiteralExpression*>(varDecl->expr.get());
    ASSERT_NE(literalExpr, nullptr);
    EXPECT_EQ(literalExpr->value.getValue(), "42");
}

TEST_F(PointerParsingTest, TypeInferenceString) {
    auto stmt = parseStatement("name := \"Alice\"");

    ASSERT_NE(stmt, nullptr);
    auto varDecl = dynamic_cast<VariableDeclaration*>(stmt.get());
    ASSERT_NE(varDecl, nullptr);

    EXPECT_EQ(varDecl->variable.getValue(), "name");
    EXPECT_EQ(varDecl->type.getValue(), "auto");

    auto literalExpr = dynamic_cast<LiteralExpression*>(varDecl->expr.get());
    ASSERT_NE(literalExpr, nullptr);
    EXPECT_EQ(literalExpr->value.getValue(), "Alice");
}

TEST_F(PointerParsingTest, TypeInferenceWithMove) {
    auto stmt = parseStatement("player2 := move player1");

    ASSERT_NE(stmt, nullptr);
    auto varDecl = dynamic_cast<VariableDeclaration*>(stmt.get());
    ASSERT_NE(varDecl, nullptr);

    EXPECT_EQ(varDecl->variable.getValue(), "player2");
    EXPECT_EQ(varDecl->type.getValue(), "auto");

    auto moveExpr = dynamic_cast<MoveExpression*>(varDecl->expr.get());
    ASSERT_NE(moveExpr, nullptr);
}

TEST_F(PointerParsingTest, TypeInferenceComplexExpression) {
    auto stmt = parseStatement("result := player.getHealth() + 10");

    ASSERT_NE(stmt, nullptr);
    auto varDecl = dynamic_cast<VariableDeclaration*>(stmt.get());
    ASSERT_NE(varDecl, nullptr);

    EXPECT_EQ(varDecl->variable.getValue(), "result");
    EXPECT_EQ(varDecl->type.getValue(), "auto");

    // Expression should be a binary addition
    auto binaryExpr = dynamic_cast<BinaryExpression*>(varDecl->expr.get());
    ASSERT_NE(binaryExpr, nullptr);
    EXPECT_EQ(binaryExpr->operator_.getType(), TokenType::PLUS);
}

// ============= DEFER STATEMENT TESTS =============

TEST_F(PointerParsingTest, DeferSimpleCall) {
    auto stmt = parseStatement("defer free(ptr)");

    ASSERT_NE(stmt, nullptr);
    auto deferStmt = dynamic_cast<DeferStatement*>(stmt.get());
    ASSERT_NE(deferStmt, nullptr);

    // The expression should be a function call
    auto funcCall = dynamic_cast<FunctionCall*>(deferStmt->expression.get());
    ASSERT_NE(funcCall, nullptr);
    EXPECT_EQ(funcCall->functionName, "free");
    EXPECT_EQ(funcCall->arguments.size(), 1);
}

TEST_F(PointerParsingTest, DeferComplexExpression) {
    auto stmt = parseStatement("defer close(file.handle)");

    ASSERT_NE(stmt, nullptr);
    auto deferStmt = dynamic_cast<DeferStatement*>(stmt.get());
    ASSERT_NE(deferStmt, nullptr);

    // The expression should be a function call with member access
    auto funcCall = dynamic_cast<FunctionCall*>(deferStmt->expression.get());
    ASSERT_NE(funcCall, nullptr);
    EXPECT_EQ(funcCall->functionName, "close");
}

TEST_F(PointerParsingTest, DeferWithAssignment) {
    auto stmt = parseStatement("defer cleanup(ptr)");

    ASSERT_NE(stmt, nullptr);
    auto deferStmt = dynamic_cast<DeferStatement*>(stmt.get());
    ASSERT_NE(deferStmt, nullptr);

    // The expression should be a function call with argument
    auto funcCall = dynamic_cast<FunctionCall*>(deferStmt->expression.get());
    ASSERT_NE(funcCall, nullptr);
    EXPECT_EQ(funcCall->functionName, "cleanup");
    EXPECT_EQ(funcCall->arguments.size(), 1);
}

// ============= EXTERN DECLARATION TESTS =============

TEST_F(PointerParsingTest, ExternSimpleFunction) {
    auto stmt = parseStatement("extern def malloc(size: int) -> *void");

    ASSERT_NE(stmt, nullptr);
    auto externStmt = dynamic_cast<ExternStatement*>(stmt.get());
    ASSERT_NE(externStmt, nullptr);

    EXPECT_EQ(externStmt->functionName.getValue(), "malloc");
    EXPECT_EQ(externStmt->parameters.size(), 1);
    EXPECT_EQ(externStmt->parameters[0].name.getValue(), "size");
    EXPECT_EQ(externStmt->parameters[0].type.getValue(), "int");
    EXPECT_EQ(externStmt->returnType.getValue(), "void");  // Note: pointer info stored in ParsedType
}

TEST_F(PointerParsingTest, ExternMultipleParameters) {
    auto stmt = parseStatement("extern def memcpy(dest: *void, src: *void, n: int) -> *void");

    ASSERT_NE(stmt, nullptr);
    auto externStmt = dynamic_cast<ExternStatement*>(stmt.get());
    ASSERT_NE(externStmt, nullptr);

    EXPECT_EQ(externStmt->functionName.getValue(), "memcpy");
    EXPECT_EQ(externStmt->parameters.size(), 3);

    EXPECT_EQ(externStmt->parameters[0].name.getValue(), "dest");
    EXPECT_EQ(externStmt->parameters[0].type.getValue(), "void");

    EXPECT_EQ(externStmt->parameters[1].name.getValue(), "src");
    EXPECT_EQ(externStmt->parameters[1].type.getValue(), "void");

    EXPECT_EQ(externStmt->parameters[2].name.getValue(), "n");
    EXPECT_EQ(externStmt->parameters[2].type.getValue(), "int");
}

TEST_F(PointerParsingTest, ExternNoParameters) {
    auto stmt = parseStatement("extern def getpid() -> int");

    ASSERT_NE(stmt, nullptr);
    auto externStmt = dynamic_cast<ExternStatement*>(stmt.get());
    ASSERT_NE(externStmt, nullptr);

    EXPECT_EQ(externStmt->functionName.getValue(), "getpid");
    EXPECT_EQ(externStmt->parameters.size(), 0);
    EXPECT_EQ(externStmt->returnType.getValue(), "int");
}

// ============= POINTER TYPE PARSING TESTS =============

TEST_F(PointerParsingTest, RawPointerType) {
    auto stmt = parseStatement("ptr: *int = malloc(4)");

    ASSERT_NE(stmt, nullptr);
    auto varDecl = dynamic_cast<VariableDeclaration*>(stmt.get());
    ASSERT_NE(varDecl, nullptr);

    EXPECT_EQ(varDecl->variable.getValue(), "ptr");
    EXPECT_EQ(varDecl->type.getValue(), "int");  // Base type stored, pointer info in ParsedType
}

TEST_F(PointerParsingTest, ImmutableReferenceType) {
    auto stmt = parseStatement("ref: &str = &text");

    ASSERT_NE(stmt, nullptr);
    auto varDecl = dynamic_cast<VariableDeclaration*>(stmt.get());
    ASSERT_NE(varDecl, nullptr);

    EXPECT_EQ(varDecl->variable.getValue(), "ref");
    EXPECT_EQ(varDecl->type.getValue(), "str");
}

TEST_F(PointerParsingTest, MutableReferenceType) {
    auto stmt = parseStatement("ref: &mut Player = &mut player");

    ASSERT_NE(stmt, nullptr);
    auto varDecl = dynamic_cast<VariableDeclaration*>(stmt.get());
    ASSERT_NE(varDecl, nullptr);

    EXPECT_EQ(varDecl->variable.getValue(), "ref");
    EXPECT_EQ(varDecl->type.getValue(), "Player");
}

TEST_F(PointerParsingTest, PointerInVariableDeclaration) {
    auto stmt = parseStatement("ptr: *int = &value");

    ASSERT_NE(stmt, nullptr);
    auto varDecl = dynamic_cast<VariableDeclaration*>(stmt.get());
    ASSERT_NE(varDecl, nullptr);

    EXPECT_EQ(varDecl->variable.getValue(), "ptr");
    EXPECT_EQ(varDecl->type.getValue(), "int");  // Base type stored in token

    // Expression should be address-of
    auto addressOf = dynamic_cast<UnaryExpression*>(varDecl->expr.get());
    ASSERT_NE(addressOf, nullptr);
    EXPECT_EQ(addressOf->operator_.getType(), TokenType::BITWISE_AND);
}

TEST_F(PointerParsingTest, ReferenceInFunctionParameter) {
    auto stmt = parseStatement("def process(data: &mut Player) -> void { }");

    ASSERT_NE(stmt, nullptr);
    auto funcDef = dynamic_cast<FunctionDefinition*>(stmt.get());
    ASSERT_NE(funcDef, nullptr);

    EXPECT_EQ(funcDef->parameters.size(), 1);
    EXPECT_EQ(funcDef->parameters[0].name.getValue(), "data");
    EXPECT_EQ(funcDef->parameters[0].type.getValue(), "Player");
}

// ============= ERROR HANDLING TESTS =============

TEST_F(PointerParsingTest, InvalidMoveOperand) {
    EXPECT_THROW({
        parseExpression("move");  // Missing operand
    }, ParsingException);
}

TEST_F(PointerParsingTest, InvalidDeferStatement) {
    EXPECT_THROW({
        parseStatement("defer");  // Missing expression
    }, ParsingException);
}

TEST_F(PointerParsingTest, InvalidExternDeclaration) {
    EXPECT_THROW({
        parseStatement("extern def malloc(size) -> *void");  // Missing parameter type
    }, ParsingException);
}

TEST_F(PointerParsingTest, InvalidTypeInference) {
    EXPECT_THROW({
        parseStatement("number :=");  // Missing expression
    }, ParsingException);
}

// ============= INTEGRATION TESTS =============

TEST_F(PointerParsingTest, CompletePointerProgram) {
    std::string program = "extern def malloc(size: int) -> *void\ndef allocate_player() -> *Player {\nptr: *Player = malloc(4)\nreturn ptr\n}";

    Tokenizer tokenizer(program);
    auto tokens = tokenizer.tokenize();
    parser.reset(tokens);

    auto ast = parser.parseProgram();
    ASSERT_NE(ast, nullptr);

    auto programStmt = dynamic_cast<Program*>(ast.get());
    ASSERT_NE(programStmt, nullptr);
    EXPECT_GT(programStmt->statements.size(), 0);
}