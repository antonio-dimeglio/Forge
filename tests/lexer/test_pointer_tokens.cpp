#include <gtest/gtest.h>
#include "../../include/lexer/Tokenizer.hpp"
#include "../../include/lexer/LexerException.hpp"

class PointerTokenTest : public ::testing::Test {
protected:
    Tokenizer tokenizer{""}; // Empty string initially

    void SetUp() override {
        // Called before each test
    }

    std::vector<Token> tokenize(const std::string& source) {
        tokenizer.reset(source);
        return tokenizer.tokenize();
    }

    void expectToken(const Token& token, TokenType expectedType, const std::string& expectedValue = "") {
        EXPECT_EQ(token.getType(), expectedType);
        if (!expectedValue.empty()) {
            EXPECT_EQ(token.getValue(), expectedValue);
        }
    }
};

// ============= POINTER KEYWORD TESTS =============

TEST_F(PointerTokenTest, MoveKeyword) {
    auto tokens = tokenize("move");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::MOVE, "move");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(PointerTokenTest, NullKeyword) {
    auto tokens = tokenize("null");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::NULL_, "null");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(PointerTokenTest, DeferKeyword) {
    auto tokens = tokenize("defer");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::DEFER, "defer");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(PointerTokenTest, ExternKeyword) {
    auto tokens = tokenize("extern");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::EXTERN, "extern");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(PointerTokenTest, UniqueKeyword) {
    auto tokens = tokenize("unique");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::UNIQUE, "unique"); 
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(PointerTokenTest, SharedKeyword) {
    auto tokens = tokenize("shared");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::SHARED, "shared"); 
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(PointerTokenTest, WeakKeyword) {
    auto tokens = tokenize("weak");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::WEAK, "weak"); 
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

// ============= POINTER OPERATOR TESTS =============

TEST_F(PointerTokenTest, PointerTypeDeclaration) {
    auto tokens = tokenize("*int");
    ASSERT_EQ(tokens.size(), 3);
    expectToken(tokens[0], TokenType::MULT, "*");  // Parser will handle context
    expectToken(tokens[1], TokenType::INT, "int");
    expectToken(tokens[2], TokenType::END_OF_FILE);
}

TEST_F(PointerTokenTest, ReferenceOperator) {
    auto tokens = tokenize("&variable");
    ASSERT_EQ(tokens.size(), 3);
    expectToken(tokens[0], TokenType::BITWISE_AND, "&");  // Parser will handle context
    expectToken(tokens[1], TokenType::IDENTIFIER, "variable");
    expectToken(tokens[2], TokenType::END_OF_FILE);
}

TEST_F(PointerTokenTest, MutableReference) {
    auto tokens = tokenize("&mut variable");
    ASSERT_EQ(tokens.size(), 4);
    expectToken(tokens[0], TokenType::BITWISE_AND, "&");
    expectToken(tokens[1], TokenType::MUT, "mut");  // Now recognized as keyword
    expectToken(tokens[2], TokenType::IDENTIFIER, "variable");
    expectToken(tokens[3], TokenType::END_OF_FILE);
}

TEST_F(PointerTokenTest, PointerDereference) {
    auto tokens = tokenize("*ptr");
    ASSERT_EQ(tokens.size(), 3);
    expectToken(tokens[0], TokenType::MULT, "*");  // Parser will distinguish context
    expectToken(tokens[1], TokenType::IDENTIFIER, "ptr");
    expectToken(tokens[2], TokenType::END_OF_FILE);
}

// ============= SMART POINTER SYNTAX TESTS =============

TEST_F(PointerTokenTest, UniquePointerDeclaration) {
    std::string input = "player: unique Player";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 5);
    expectToken(tokens[0], TokenType::IDENTIFIER, "player");
    expectToken(tokens[1], TokenType::COLON, ":");
    expectToken(tokens[2], TokenType::UNIQUE, "unique");  // Will be keyword later
    expectToken(tokens[3], TokenType::IDENTIFIER, "Player");
    expectToken(tokens[4], TokenType::END_OF_FILE);
}

TEST_F(PointerTokenTest, SharedPointerDeclaration) {
    std::string input = "config: shared GameConfig";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 5);
    expectToken(tokens[0], TokenType::IDENTIFIER, "config");
    expectToken(tokens[1], TokenType::COLON, ":");
    expectToken(tokens[2], TokenType::SHARED, "shared");  
    expectToken(tokens[3], TokenType::IDENTIFIER, "GameConfig");
    expectToken(tokens[4], TokenType::END_OF_FILE);
}

TEST_F(PointerTokenTest, WeakPointerDeclaration) {
    std::string input = "weak_ref: weak Node";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 5);
    expectToken(tokens[0], TokenType::IDENTIFIER, "weak_ref");
    expectToken(tokens[1], TokenType::COLON, ":");
    expectToken(tokens[2], TokenType::WEAK, "weak");   
    expectToken(tokens[3], TokenType::IDENTIFIER, "Node");
    expectToken(tokens[4], TokenType::END_OF_FILE);
}

TEST_F(PointerTokenTest, SmartPointerCreation) {
    std::string input = "unique new Player(\"Alice\")";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 7);
    expectToken(tokens[0], TokenType::UNIQUE, "unique"); 
    expectToken(tokens[1], TokenType::NEW, "new");   
    expectToken(tokens[2], TokenType::IDENTIFIER, "Player");
    expectToken(tokens[3], TokenType::LPAREN, "(");
    expectToken(tokens[4], TokenType::STRING, "Alice");
    expectToken(tokens[5], TokenType::RPAREN, ")");
    expectToken(tokens[6], TokenType::END_OF_FILE);
}

// ============= RAW POINTER TESTS =============

TEST_F(PointerTokenTest, MallocCall) {
    std::string input = "malloc(sizeof(int))";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 8);
    expectToken(tokens[0], TokenType::IDENTIFIER, "malloc");
    expectToken(tokens[1], TokenType::LPAREN, "(");
    expectToken(tokens[2], TokenType::IDENTIFIER, "sizeof");
    expectToken(tokens[3], TokenType::LPAREN, "(");
    expectToken(tokens[4], TokenType::INT, "int");
    expectToken(tokens[5], TokenType::RPAREN, ")");
    expectToken(tokens[6], TokenType::RPAREN, ")");
    expectToken(tokens[7], TokenType::END_OF_FILE);
}

TEST_F(PointerTokenTest, PointerArithmetic) {
    std::string input = "ptr + 1";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 4);
    expectToken(tokens[0], TokenType::IDENTIFIER, "ptr");
    expectToken(tokens[1], TokenType::PLUS, "+");
    expectToken(tokens[2], TokenType::NUMBER, "1");
    expectToken(tokens[3], TokenType::END_OF_FILE);
}

TEST_F(PointerTokenTest, PointerArrayAccess) {
    std::string input = "ptr[0]";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 5);
    expectToken(tokens[0], TokenType::IDENTIFIER, "ptr");
    expectToken(tokens[1], TokenType::LSQUARE, "[");
    expectToken(tokens[2], TokenType::NUMBER, "0");
    expectToken(tokens[3], TokenType::RSQUARE, "]");
    expectToken(tokens[4], TokenType::END_OF_FILE);
}

TEST_F(PointerTokenTest, ExternFunctionDeclaration) {
    std::string input = "extern def malloc(size: int) -> *void";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 12);
    expectToken(tokens[0], TokenType::EXTERN, "extern");
    expectToken(tokens[1], TokenType::DEF, "def");
    expectToken(tokens[2], TokenType::IDENTIFIER, "malloc");
    expectToken(tokens[3], TokenType::LPAREN, "(");
    expectToken(tokens[4], TokenType::IDENTIFIER, "size");
    expectToken(tokens[5], TokenType::COLON, ":");
    expectToken(tokens[6], TokenType::INT, "int");
    expectToken(tokens[7], TokenType::RPAREN, ")");
    expectToken(tokens[8], TokenType::ARROW, "->");
    expectToken(tokens[9], TokenType::MULT, "*");       // Raw pointer
    expectToken(tokens[10], TokenType::VOID, "void");
    expectToken(tokens[11], TokenType::END_OF_FILE);
}

// ============= MOVE SEMANTICS TESTS =============

TEST_F(PointerTokenTest, MoveExpression) {
    std::string input = "move player";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 3);
    expectToken(tokens[0], TokenType::MOVE, "move");
    expectToken(tokens[1], TokenType::IDENTIFIER, "player");
    expectToken(tokens[2], TokenType::END_OF_FILE);
}

TEST_F(PointerTokenTest, MoveAssignment) {
    std::string input = "player2 := move player1";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 5);
    expectToken(tokens[0], TokenType::IDENTIFIER, "player2");
    expectToken(tokens[1], TokenType::INFER_ASSIGN, ":=");   // Now a single compound token!
    expectToken(tokens[2], TokenType::MOVE, "move");
    expectToken(tokens[3], TokenType::IDENTIFIER, "player1");
    expectToken(tokens[4], TokenType::END_OF_FILE);
}

// ============= TYPE INFERENCE ASSIGNMENT TESTS =============

TEST_F(PointerTokenTest, InferAssignOperator) {
    auto tokens = tokenize(":=");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::INFER_ASSIGN, ":=");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(PointerTokenTest, SmartPointerInference) {
    std::string input = "player := unique new Player(\"Alice\")";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 9);
    expectToken(tokens[0], TokenType::IDENTIFIER, "player");
    expectToken(tokens[1], TokenType::INFER_ASSIGN, ":=");
    expectToken(tokens[2], TokenType::UNIQUE, "unique");
    expectToken(tokens[3], TokenType::NEW, "new");
    expectToken(tokens[4], TokenType::IDENTIFIER, "Player");
    expectToken(tokens[5], TokenType::LPAREN, "(");
    expectToken(tokens[6], TokenType::STRING, "Alice");
    expectToken(tokens[7], TokenType::RPAREN, ")");
    expectToken(tokens[8], TokenType::END_OF_FILE);
}

TEST_F(PointerTokenTest, BasicTypeInference) {
    std::string input = "number := 42";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 4);
    expectToken(tokens[0], TokenType::IDENTIFIER, "number");
    expectToken(tokens[1], TokenType::INFER_ASSIGN, ":=");
    expectToken(tokens[2], TokenType::NUMBER, "42");
    expectToken(tokens[3], TokenType::END_OF_FILE);
}

TEST_F(PointerTokenTest, StringTypeInference) {
    std::string input = "name := \"Alice\"";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 4);
    expectToken(tokens[0], TokenType::IDENTIFIER, "name");
    expectToken(tokens[1], TokenType::INFER_ASSIGN, ":=");
    expectToken(tokens[2], TokenType::STRING, "Alice");
    expectToken(tokens[3], TokenType::END_OF_FILE);
}

TEST_F(PointerTokenTest, SharedPointerInference) {
    std::string input = "config := shared new GameConfig()";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 8);
    expectToken(tokens[0], TokenType::IDENTIFIER, "config");
    expectToken(tokens[1], TokenType::INFER_ASSIGN, ":=");
    expectToken(tokens[2], TokenType::SHARED, "shared");
    expectToken(tokens[3], TokenType::NEW, "new");
    expectToken(tokens[4], TokenType::IDENTIFIER, "GameConfig");
    expectToken(tokens[5], TokenType::LPAREN, "(");
    expectToken(tokens[6], TokenType::RPAREN, ")");
    expectToken(tokens[7], TokenType::END_OF_FILE);
}

TEST_F(PointerTokenTest, InferAssignVsColonAssign) {
    // Test that := is different from : =
    std::string input1 = "a := 5";      // Type inference
    std::string input2 = "a : int = 5"; // Explicit type

    auto tokens1 = tokenize(input1);
    ASSERT_EQ(tokens1.size(), 4);
    expectToken(tokens1[1], TokenType::INFER_ASSIGN, ":=");

    auto tokens2 = tokenize(input2);
    ASSERT_EQ(tokens2.size(), 6);
    expectToken(tokens2[1], TokenType::COLON, ":");
    expectToken(tokens2[3], TokenType::ASSIGN, "=");
}

// ============= DEFER STATEMENT TESTS =============

TEST_F(PointerTokenTest, DeferStatement) {
    std::string input = "defer free(ptr)";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 6);
    expectToken(tokens[0], TokenType::DEFER, "defer");
    expectToken(tokens[1], TokenType::IDENTIFIER, "free");
    expectToken(tokens[2], TokenType::LPAREN, "(");
    expectToken(tokens[3], TokenType::IDENTIFIER, "ptr");
    expectToken(tokens[4], TokenType::RPAREN, ")");
    expectToken(tokens[5], TokenType::END_OF_FILE);
}

// ============= NULL SAFETY TESTS =============

TEST_F(PointerTokenTest, NullComparison) {
    std::string input = "ptr != null";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 4);
    expectToken(tokens[0], TokenType::IDENTIFIER, "ptr");
    expectToken(tokens[1], TokenType::NOT_EQUAL, "!=");
    expectToken(tokens[2], TokenType::NULL_, "null");
    expectToken(tokens[3], TokenType::END_OF_FILE);
}

TEST_F(PointerTokenTest, NullAssignment) {
    std::string input = "ptr = null";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 4);
    expectToken(tokens[0], TokenType::IDENTIFIER, "ptr");
    expectToken(tokens[1], TokenType::ASSIGN, "=");
    expectToken(tokens[2], TokenType::NULL_, "null");
    expectToken(tokens[3], TokenType::END_OF_FILE);
}

// ============= COMPLEX POINTER EXPRESSIONS =============

TEST_F(PointerTokenTest, ComplexPointerExpression) {
    std::string input = "*(&variable + 1)";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 8);
    expectToken(tokens[0], TokenType::MULT, "*");         // Dereference
    expectToken(tokens[1], TokenType::LPAREN, "(");
    expectToken(tokens[2], TokenType::BITWISE_AND, "&"); // Address-of
    expectToken(tokens[3], TokenType::IDENTIFIER, "variable");
    expectToken(tokens[4], TokenType::PLUS, "+");
    expectToken(tokens[5], TokenType::NUMBER, "1");
    expectToken(tokens[6], TokenType::RPAREN, ")");
    expectToken(tokens[7], TokenType::END_OF_FILE);
}

TEST_F(PointerTokenTest, SmartPointerMethodCall) {
    std::string input = "player.reset()";
    auto tokens = tokenize(input);

    ASSERT_EQ(tokens.size(), 6);
    expectToken(tokens[0], TokenType::IDENTIFIER, "player");
    expectToken(tokens[1], TokenType::DOT, ".");
    expectToken(tokens[2], TokenType::IDENTIFIER, "reset");
    expectToken(tokens[3], TokenType::LPAREN, "(");
    expectToken(tokens[4], TokenType::RPAREN, ")");
    expectToken(tokens[5], TokenType::END_OF_FILE);
}