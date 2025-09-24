#include <gtest/gtest.h>
#include "../../include/lexer/Tokenizer.hpp"
#include "../../include/lexer/LexerException.hpp"

class TokenizerTest : public ::testing::Test {
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

// ============= BASIC TOKEN TESTS =============


TEST_F(TokenizerTest, EmptyInput) {
    auto tokens = tokenize("");
    ASSERT_EQ(tokens.size(), 1);
    expectToken(tokens[0], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, WhitespaceOnly) {
    auto tokens = tokenize("   \t  \r   ");
    ASSERT_EQ(tokens.size(), 1);
    expectToken(tokens[0], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, SingleNewline) {
    auto tokens = tokenize("\n");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::NEWLINE);
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, MultipleNewlines) {
    auto tokens = tokenize("\n\n\n");
    ASSERT_EQ(tokens.size(), 4);
    expectToken(tokens[0], TokenType::NEWLINE);
    expectToken(tokens[1], TokenType::NEWLINE);
    expectToken(tokens[2], TokenType::NEWLINE);
    expectToken(tokens[3], TokenType::END_OF_FILE);
}

// ============= NUMBER TESTS =============

TEST_F(TokenizerTest, SingleDigit) {
    auto tokens = tokenize("5");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::NUMBER, "5");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, MultipleDigits) {
    auto tokens = tokenize("12345");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::NUMBER, "12345");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, FloatingPointNumber) {
    auto tokens = tokenize("3.14159");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::NUMBER, "3.14159");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, NumberStartingWithDot) {
    auto tokens = tokenize(".5");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::NUMBER, ".5");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, NumberEndingWithDot) {
    auto tokens = tokenize("42.");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::NUMBER, "42.");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, ZeroNumber) {
    auto tokens = tokenize("0");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::NUMBER, "0");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, LargeNumber) {
    auto tokens = tokenize("999999999999999");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::NUMBER, "999999999999999");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, MultipleNumbers) {
    auto tokens = tokenize("1 2.5 42");
    ASSERT_EQ(tokens.size(), 4);
    expectToken(tokens[0], TokenType::NUMBER, "1");
    expectToken(tokens[1], TokenType::NUMBER, "2.5");
    expectToken(tokens[2], TokenType::NUMBER, "42");
    expectToken(tokens[3], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, InvalidNumberMultipleDots) {
    EXPECT_THROW(tokenize("3.14.15"), InvalidSyntaxException);
}

// ============= IDENTIFIER AND KEYWORD TESTS =============

TEST_F(TokenizerTest, SimpleIdentifier) {
    auto tokens = tokenize("variable");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::IDENTIFIER, "variable");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, IdentifierWithUnderscore) {
    auto tokens = tokenize("_variable");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::IDENTIFIER, "_variable");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, IdentifierWithNumbers) {
    auto tokens = tokenize("var123");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::IDENTIFIER, "var123");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, MixedCaseIdentifier) {
    auto tokens = tokenize("myVariable");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::IDENTIFIER, "myVariable");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, SingleLetterIdentifier) {
    auto tokens = tokenize("x");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::IDENTIFIER, "x");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

// Keyword tests
TEST_F(TokenizerTest, IfKeyword) {
    auto tokens = tokenize("if");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::IF, "if");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, ElseKeyword) {
    auto tokens = tokenize("else");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::ELSE, "else");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, WhileKeyword) {
    auto tokens = tokenize("while");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::WHILE, "while");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, ForKeyword) {
    auto tokens = tokenize("for");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::FOR, "for");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, DefKeyword) {
    auto tokens = tokenize("def");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::DEF, "def");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, ReturnKeyword) {
    auto tokens = tokenize("return");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::RETURN, "return");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, TypeKeywords) {
    auto tokens = tokenize("int str bool float double");
    ASSERT_EQ(tokens.size(), 6);
    expectToken(tokens[0], TokenType::INT, "int");
    expectToken(tokens[1], TokenType::STR, "str");
    expectToken(tokens[2], TokenType::BOOL, "bool");
    expectToken(tokens[3], TokenType::FLOAT, "float");
    expectToken(tokens[4], TokenType::DOUBLE, "double");
    expectToken(tokens[5], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, BooleanKeywords) {
    auto tokens = tokenize("true false");
    ASSERT_EQ(tokens.size(), 3);
    expectToken(tokens[0], TokenType::TRUE, "true");
    expectToken(tokens[1], TokenType::FALSE, "false");
    expectToken(tokens[2], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, KeywordVsIdentifier) {
    auto tokens = tokenize("if ifx iff xif");
    ASSERT_EQ(tokens.size(), 5);
    expectToken(tokens[0], TokenType::IF, "if");
    expectToken(tokens[1], TokenType::IDENTIFIER, "ifx");
    expectToken(tokens[2], TokenType::IDENTIFIER, "iff");
    expectToken(tokens[3], TokenType::IDENTIFIER, "xif");
    expectToken(tokens[4], TokenType::END_OF_FILE);
}

// ============= STRING TESTS =============

TEST_F(TokenizerTest, SimpleDoubleQuotedString) {
    auto tokens = tokenize("\"hello\"");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::STRING, "hello");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, SimpleSingleQuotedString) {
    auto tokens = tokenize("'hello'");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::STRING, "hello");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, EmptyString) {
    auto tokens = tokenize("\"\"");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::STRING, "");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, StringWithSpaces) {
    auto tokens = tokenize("\"hello world\"");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::STRING, "hello world");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, StringWithNumbers) {
    auto tokens = tokenize("\"test123\"");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::STRING, "test123");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, StringWithSpecialChars) {
    auto tokens = tokenize("\"hello!@#$%^&*()\"");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::STRING, "hello!@#$%^&*()");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, StringWithNewlines) {
    auto tokens = tokenize("\"hello\nworld\"");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::STRING, "hello\nworld");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, MultipleStrings) {
    auto tokens = tokenize("\"first\" \"second\"");
    ASSERT_EQ(tokens.size(), 3);
    expectToken(tokens[0], TokenType::STRING, "first");
    expectToken(tokens[1], TokenType::STRING, "second");
    expectToken(tokens[2], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, UnterminatedString) {
    EXPECT_THROW(tokenize("\"unterminated"), InvalidSyntaxException);
}

TEST_F(TokenizerTest, UnterminatedSingleQuoteString) {
    EXPECT_THROW(tokenize("'unterminated"), InvalidSyntaxException);
}

// ============= OPERATOR TESTS =============

TEST_F(TokenizerTest, ArithmeticOperators) {
    auto tokens = tokenize("+ - * /");
    ASSERT_EQ(tokens.size(), 5);
    expectToken(tokens[0], TokenType::PLUS, "+");
    expectToken(tokens[1], TokenType::MINUS, "-");
    expectToken(tokens[2], TokenType::MULT, "*");
    expectToken(tokens[3], TokenType::DIV, "/");
    expectToken(tokens[4], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, AssignmentOperators) {
    auto tokens = tokenize("= ==");
    ASSERT_EQ(tokens.size(), 3);
    expectToken(tokens[0], TokenType::ASSIGN, "=");
    expectToken(tokens[1], TokenType::EQUAL_EQUAL, "==");
    expectToken(tokens[2], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, CompoundAssignmentOperators) {
    auto tokens = tokenize("+= -= *= /=");
    ASSERT_EQ(tokens.size(), 5);
    expectToken(tokens[0], TokenType::PLUS_EQ, "+=");
    expectToken(tokens[1], TokenType::MINUS_EQ, "-=");
    expectToken(tokens[2], TokenType::MULT_EQ, "*=");
    expectToken(tokens[3], TokenType::DIV_EQ, "/=");
    expectToken(tokens[4], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, ComparisonOperators) {
    auto tokens = tokenize("== != < >");
    ASSERT_EQ(tokens.size(), 5);
    expectToken(tokens[0], TokenType::EQUAL_EQUAL, "==");
    expectToken(tokens[1], TokenType::NOT_EQUAL, "!=");
    expectToken(tokens[2], TokenType::LESS, "<");
    expectToken(tokens[3], TokenType::GREATER, ">");
    expectToken(tokens[4], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, LogicalOperators) {
    auto tokens = tokenize("! !=");
    ASSERT_EQ(tokens.size(), 3);
    expectToken(tokens[0], TokenType::NOT, "!");
    expectToken(tokens[1], TokenType::NOT_EQUAL, "!=");
    expectToken(tokens[2], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, PunctuationTokens) {
    auto tokens = tokenize("( ) { } [ ] , :");
    ASSERT_EQ(tokens.size(), 9);
    expectToken(tokens[0], TokenType::LPAREN, "(");
    expectToken(tokens[1], TokenType::RPAREN, ")");
    expectToken(tokens[2], TokenType::LBRACE, "{");
    expectToken(tokens[3], TokenType::RBRACE, "}");
    expectToken(tokens[4], TokenType::LSQUARE, "[");
    expectToken(tokens[5], TokenType::RSQUARE, "]");
    expectToken(tokens[6], TokenType::COMMA, ",");
    expectToken(tokens[7], TokenType::COLON, ":");
    expectToken(tokens[8], TokenType::END_OF_FILE);
}

// ============= COMPLEX EXPRESSION TESTS =============

TEST_F(TokenizerTest, SimpleAssignment) {
    auto tokens = tokenize("x = 42");
    ASSERT_EQ(tokens.size(), 4);
    expectToken(tokens[0], TokenType::IDENTIFIER, "x");
    expectToken(tokens[1], TokenType::ASSIGN, "=");
    expectToken(tokens[2], TokenType::NUMBER, "42");
    expectToken(tokens[3], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, ArithmeticExpression) {
    auto tokens = tokenize("x + y * 2");
    ASSERT_EQ(tokens.size(), 6);
    expectToken(tokens[0], TokenType::IDENTIFIER, "x");
    expectToken(tokens[1], TokenType::PLUS, "+");
    expectToken(tokens[2], TokenType::IDENTIFIER, "y");
    expectToken(tokens[3], TokenType::MULT, "*");
    expectToken(tokens[4], TokenType::NUMBER, "2");
    expectToken(tokens[5], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, BooleanExpression) {
    auto tokens = tokenize("x == 42 && y != 0");
    ASSERT_EQ(tokens.size(), 8);
    expectToken(tokens[0], TokenType::IDENTIFIER, "x");
    expectToken(tokens[1], TokenType::EQUAL_EQUAL, "==");
    expectToken(tokens[2], TokenType::NUMBER, "42");
    expectToken(tokens[3], TokenType::LOGIC_AND, "&&");
    expectToken(tokens[7], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, FunctionCall) {
    auto tokens = tokenize("print(\"Hello World!\")");

    ASSERT_EQ(tokens.size(), 5);
    expectToken(tokens[0], TokenType::IDENTIFIER, "print");
    expectToken(tokens[1], TokenType::LPAREN, "(");
    expectToken(tokens[2], TokenType::STRING, "Hello World!");
    expectToken(tokens[3], TokenType::RPAREN, ")");
    expectToken(tokens[4], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, IfStatement) {
    auto tokens = tokenize("if (x > 0)");
    ASSERT_EQ(tokens.size(), 7);
    expectToken(tokens[0], TokenType::IF, "if");
    expectToken(tokens[1], TokenType::LPAREN, "(");
    expectToken(tokens[2], TokenType::IDENTIFIER, "x");
    expectToken(tokens[3], TokenType::GREATER, ">");
    expectToken(tokens[4], TokenType::NUMBER, "0");
    expectToken(tokens[5], TokenType::RPAREN, ")");
    expectToken(tokens[6], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, FunctionDefinition) {
    auto tokens = tokenize("def add(a: int, b: int) -> int:");
    ASSERT_EQ(tokens.size(), 15);
    expectToken(tokens[0], TokenType::DEF, "def");
    expectToken(tokens[1], TokenType::IDENTIFIER, "add");
    expectToken(tokens[2], TokenType::LPAREN, "(");
    expectToken(tokens[3], TokenType::IDENTIFIER, "a");
    expectToken(tokens[4], TokenType::COLON, ":");
    expectToken(tokens[5], TokenType::INT, "int");
    expectToken(tokens[6], TokenType::COMMA, ",");
    expectToken(tokens[7], TokenType::IDENTIFIER, "b");
    expectToken(tokens[8], TokenType::COLON, ":");
    expectToken(tokens[9], TokenType::INT, "int");
    expectToken(tokens[10], TokenType::RPAREN, ")");
    expectToken(tokens[11], TokenType::ARROW, "->");
    expectToken(tokens[12], TokenType::INT, "int");
    expectToken(tokens[13], TokenType::COLON);
    expectToken(tokens[14], TokenType::END_OF_FILE);
}

// ============= EDGE CASES AND ERROR TESTS =============

TEST_F(TokenizerTest, NoSpacesBetweenTokens) {
    auto tokens = tokenize("x=42+y");
    ASSERT_EQ(tokens.size(), 6);
    expectToken(tokens[0], TokenType::IDENTIFIER, "x");
    expectToken(tokens[1], TokenType::ASSIGN, "=");
    expectToken(tokens[2], TokenType::NUMBER, "42");
    expectToken(tokens[3], TokenType::PLUS, "+");
    expectToken(tokens[4], TokenType::IDENTIFIER, "y");
    expectToken(tokens[5], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, MixedWhitespace) {
    auto tokens = tokenize("x \t= \r\n 42");
    ASSERT_EQ(tokens.size(), 5);
    expectToken(tokens[0], TokenType::IDENTIFIER, "x");
    expectToken(tokens[1], TokenType::ASSIGN, "=");
    expectToken(tokens[2], TokenType::NEWLINE);
    expectToken(tokens[3], TokenType::NUMBER, "42");
    expectToken(tokens[4], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, ConsecutiveOperators) {
    auto tokens = tokenize("++--");
    ASSERT_EQ(tokens.size(), 5);
    expectToken(tokens[0], TokenType::PLUS, "+");
    expectToken(tokens[1], TokenType::PLUS, "+");
    expectToken(tokens[2], TokenType::MINUS, "-");
    expectToken(tokens[3], TokenType::MINUS, "-");
    expectToken(tokens[4], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, InvalidCharacters) {
    EXPECT_THROW(tokenize("@"), InvalidSyntaxException);
    EXPECT_THROW(tokenize("#"), InvalidSyntaxException);
    EXPECT_THROW(tokenize("$"), InvalidSyntaxException);
    EXPECT_THROW(tokenize("%"), InvalidSyntaxException);
    EXPECT_THROW(tokenize("~"), InvalidSyntaxException);
    EXPECT_THROW(tokenize("`"), InvalidSyntaxException);
    EXPECT_THROW(tokenize("\\"), InvalidSyntaxException);
}

TEST_F(TokenizerTest, UnicodeCharacters) {
    EXPECT_THROW(tokenize("π"), InvalidSyntaxException);
    EXPECT_THROW(tokenize("café"), InvalidSyntaxException);
}

// ============= COMPLETE PROGRAM TESTS =============

TEST_F(TokenizerTest, SimpleProgram) {
    std::string program = R"(
x: int = 42
if (x > 0) {
    print("positive")
} else {
    print("not positive")
}
)";
    auto tokens = tokenize(program);

    // Should have many tokens ending with END_OF_FILE
    EXPECT_GT(tokens.size(), 10);
    expectToken(tokens.back(), TokenType::END_OF_FILE);

    // Check some key tokens
    bool foundInt = false, foundIf = false, foundElse = false;
    for (const auto& token : tokens) {
        if (token.getType() == TokenType::INT) foundInt = true;
        if (token.getType() == TokenType::IF) foundIf = true;
        if (token.getType() == TokenType::ELSE) foundElse = true;
    }
    EXPECT_TRUE(foundInt);
    EXPECT_TRUE(foundIf);
    EXPECT_TRUE(foundElse);
}

TEST_F(TokenizerTest, FunctionProgram) {
    std::string program = R"(
def fibonacci(n: int) -> int: {
    if (n <= 1) {
        return n
    }
    return fibonacci(n-1) + fibonacci(n-2)
}
)";
    auto tokens = tokenize(program);

    EXPECT_GT(tokens.size(), 15);
    expectToken(tokens.back(), TokenType::END_OF_FILE);

    // Should contain def, return keywords
    bool foundDef = false, foundReturn = false;
    for (const auto& token : tokens) {
        if (token.getType() == TokenType::DEF) foundDef = true;
        if (token.getType() == TokenType::RETURN) foundReturn = true;
    }
    EXPECT_TRUE(foundDef);
    EXPECT_TRUE(foundReturn);
}

// ============= POSITION TRACKING TESTS =============

TEST_F(TokenizerTest, LineNumberTracking) {
    auto tokens = tokenize("x\ny\nz");
    ASSERT_GE(tokens.size(), 6);

    // First token should be on line 1
    EXPECT_EQ(tokens[0].getLine(), 1);

    // After first newline, should be on line 2
    bool foundLine2 = false;
    for (const auto& token : tokens) {
        if (token.getLine() == 2) foundLine2 = true;
    }
    EXPECT_TRUE(foundLine2);
}

TEST_F(TokenizerTest, ColumnNumberTracking) {
    auto tokens = tokenize("abc def");
    ASSERT_GE(tokens.size(), 3);

    // First token should start at column 1
    EXPECT_EQ(tokens[0].getColumn(), 1);

    // Second token should be at a higher column
    EXPECT_GT(tokens[1].getColumn(), tokens[0].getColumn());
}

// ============= STRESS TESTS =============

TEST_F(TokenizerTest, LongInput) {
    std::string longInput;
    for (int i = 0; i < 1000; ++i) {
        longInput += "x" + std::to_string(i) + " ";
    }

    auto tokens = tokenize(longInput);
    EXPECT_EQ(tokens.size(), 1001); // 1000 identifiers + END_OF_FILE
}

TEST_F(TokenizerTest, DeeplyNestedExpression) {
    std::string nested = "";
    for (int i = 0; i < 100; ++i) {
        nested += "(";
    }
    nested += "x";
    for (int i = 0; i < 100; ++i) {
        nested += ")";
    }

    auto tokens = tokenize(nested);
    EXPECT_EQ(tokens.size(), 202); // 100 LPAREN + 1 IDENTIFIER + 100 RPAREN + END_OF_FILE
}

// ============= ADDITIONAL EDGE CASE TESTS =============

TEST_F(TokenizerTest, CompoundOperatorEdgeCases) {
    auto tokens = tokenize("<= >= && || ->");
    ASSERT_EQ(tokens.size(), 6);
    expectToken(tokens[0], TokenType::LEQ, "<=");
    expectToken(tokens[1], TokenType::GEQ, ">=");
    expectToken(tokens[2], TokenType::LOGIC_AND, "&&");
    expectToken(tokens[3], TokenType::LOGIC_OR, "||");
    expectToken(tokens[4], TokenType::ARROW, "->");
    expectToken(tokens[5], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, BitwiseOperators) {
    auto tokens = tokenize("& | ^");
    ASSERT_EQ(tokens.size(), 4);
    expectToken(tokens[0], TokenType::BITWISE_AND, "&");
    expectToken(tokens[1], TokenType::BITWISE_OR, "|");
    expectToken(tokens[2], TokenType::BITWISE_XOR, "^");
    expectToken(tokens[3], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, OperatorPrecedenceTokenization) {
    auto tokens = tokenize("!x && y || z");
    ASSERT_EQ(tokens.size(), 7);
    expectToken(tokens[0], TokenType::NOT, "!");
    expectToken(tokens[1], TokenType::IDENTIFIER, "x");
    expectToken(tokens[2], TokenType::LOGIC_AND, "&&");
    expectToken(tokens[3], TokenType::IDENTIFIER, "y");
    expectToken(tokens[4], TokenType::LOGIC_OR, "||");
    expectToken(tokens[5], TokenType::IDENTIFIER, "z");
    expectToken(tokens[6], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, NumberEdgeCases) {
    EXPECT_THROW(tokenize("123.456.789"), InvalidSyntaxException);
}

TEST_F(TokenizerTest, NumberAtEndOfInput) {
    auto tokens = tokenize("42");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::NUMBER, "42");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, DotEdgeCases) {
    auto tokens = tokenize(".5 + 5.");
    ASSERT_EQ(tokens.size(), 4);
    expectToken(tokens[0], TokenType::NUMBER, ".5");
    expectToken(tokens[1], TokenType::PLUS, "+");
    expectToken(tokens[2], TokenType::NUMBER, "5.");
    expectToken(tokens[3], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, StringEscapeSequenceHandling) {
    auto tokens = tokenize("\"line1\\nline2\"");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::STRING, "line1\\nline2");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, StringWithQuotes) {
    auto tokens = tokenize("\"He said 'hello'\" 'She said \"hi\"'");
    ASSERT_EQ(tokens.size(), 3);
    expectToken(tokens[0], TokenType::STRING, "He said 'hello'");
    expectToken(tokens[1], TokenType::STRING, "She said \"hi\"");
    expectToken(tokens[2], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, EmptyStringVariations) {
    auto tokens = tokenize("'' \"\" \"   \" '   '");
    ASSERT_EQ(tokens.size(), 5);
    expectToken(tokens[0], TokenType::STRING, "");
    expectToken(tokens[1], TokenType::STRING, "");
    expectToken(tokens[2], TokenType::STRING, "   ");
    expectToken(tokens[3], TokenType::STRING, "   ");
    expectToken(tokens[4], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, IdentifierStartingWithKeyword) {
    auto tokens = tokenize("iff elsee whilex forloop defx returner");
    ASSERT_EQ(tokens.size(), 7);
    expectToken(tokens[0], TokenType::IDENTIFIER, "iff");
    expectToken(tokens[1], TokenType::IDENTIFIER, "elsee");
    expectToken(tokens[2], TokenType::IDENTIFIER, "whilex");
    expectToken(tokens[3], TokenType::IDENTIFIER, "forloop");
    expectToken(tokens[4], TokenType::IDENTIFIER, "defx");
    expectToken(tokens[5], TokenType::IDENTIFIER, "returner");
    expectToken(tokens[6], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, UnderscoreIdentifiers) {
    auto tokens = tokenize("_ __ _var var_ _var_ __var__");
    ASSERT_EQ(tokens.size(), 7);
    expectToken(tokens[0], TokenType::IDENTIFIER, "_");
    expectToken(tokens[1], TokenType::IDENTIFIER, "__");
    expectToken(tokens[2], TokenType::IDENTIFIER, "_var");
    expectToken(tokens[3], TokenType::IDENTIFIER, "var_");
    expectToken(tokens[4], TokenType::IDENTIFIER, "_var_");
    expectToken(tokens[5], TokenType::IDENTIFIER, "__var__");
    expectToken(tokens[6], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, MixedCaseKeywords) {
    auto tokens = tokenize("If IF iF Else ELSE ElSe");
    ASSERT_EQ(tokens.size(), 7);
    expectToken(tokens[0], TokenType::IDENTIFIER, "If");
    expectToken(tokens[1], TokenType::IDENTIFIER, "IF");
    expectToken(tokens[2], TokenType::IDENTIFIER, "iF");
    expectToken(tokens[3], TokenType::IDENTIFIER, "Else");
    expectToken(tokens[4], TokenType::IDENTIFIER, "ELSE");
    expectToken(tokens[5], TokenType::IDENTIFIER, "ElSe");
    expectToken(tokens[6], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, ComplexExpressionWithoutSpaces) {
    auto tokens = tokenize("x+y*z/w-v==u!=t&&s||r");
    ASSERT_EQ(tokens.size(), 18);
    expectToken(tokens[0], TokenType::IDENTIFIER, "x");
    expectToken(tokens[1], TokenType::PLUS, "+");
    expectToken(tokens[2], TokenType::IDENTIFIER, "y");
    expectToken(tokens[3], TokenType::MULT, "*");
    expectToken(tokens[17], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, NestedParenthesesAndBraces) {
    auto tokens = tokenize("({[()]})");
    ASSERT_EQ(tokens.size(), 9);
    expectToken(tokens[0], TokenType::LPAREN, "(");
    expectToken(tokens[1], TokenType::LBRACE, "{");
    expectToken(tokens[2], TokenType::LSQUARE, "[");
    expectToken(tokens[3], TokenType::LPAREN, "(");
    expectToken(tokens[4], TokenType::RPAREN, ")");
    expectToken(tokens[5], TokenType::RSQUARE, "]");
    expectToken(tokens[6], TokenType::RBRACE, "}");
    expectToken(tokens[7], TokenType::RPAREN, ")");
    expectToken(tokens[8], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, OperatorCombinations) {
    auto tokens = tokenize("x += y -= z");
    ASSERT_EQ(tokens.size(), 6);
    expectToken(tokens[0], TokenType::IDENTIFIER, "x");
    expectToken(tokens[1], TokenType::PLUS_EQ, "+=");
    expectToken(tokens[2], TokenType::IDENTIFIER, "y");
    expectToken(tokens[3], TokenType::MINUS_EQ, "-=");
    expectToken(tokens[4], TokenType::IDENTIFIER, "z");
    expectToken(tokens[5], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, WhitespaceAndNewlineHandling) {
    auto tokens = tokenize("x\n\n\ny\t\tz\r\n\rw");
    ASSERT_GE(tokens.size(), 7);

    bool foundNewlines = false;
    int newlineCount = 0;
    for (const auto& token : tokens) {
        if (token.getType() == TokenType::NEWLINE) {
            foundNewlines = true;
            newlineCount++;
        }
    }
    EXPECT_TRUE(foundNewlines);
    EXPECT_GE(newlineCount, 2);
}

TEST_F(TokenizerTest, NumberIdentifierBoundaries) {
    auto tokens = tokenize("123abc abc123 123.45abc");
    ASSERT_EQ(tokens.size(), 6);
    expectToken(tokens[0], TokenType::NUMBER, "123");
    expectToken(tokens[1], TokenType::IDENTIFIER, "abc");
    expectToken(tokens[2], TokenType::IDENTIFIER, "abc123");
    expectToken(tokens[3], TokenType::NUMBER, "123.45");
    expectToken(tokens[4], TokenType::IDENTIFIER, "abc");
    expectToken(tokens[5], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, TokenPositionAccuracy) {
    auto tokens = tokenize("abc\nxyz\n\ngh");

    EXPECT_EQ(tokens[0].getLine(), 1);
    EXPECT_EQ(tokens[0].getColumn(), 1);

    bool foundLine2 = false, foundLine4 = false;
    for (const auto& token : tokens) {
        if (token.getType() == TokenType::IDENTIFIER && token.getValue() == "xyz") {
            EXPECT_EQ(token.getLine(), 2);
            foundLine2 = true;
        }
        if (token.getType() == TokenType::IDENTIFIER && token.getValue() == "gh") {
            EXPECT_EQ(token.getLine(), 4);
            foundLine4 = true;
        }
    }
    EXPECT_TRUE(foundLine2);
    EXPECT_TRUE(foundLine4);
}

TEST_F(TokenizerTest, StringSpanningLines) {
    auto tokens = tokenize("\"hello\nworld\ntest\"");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::STRING, "hello\nworld\ntest");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, ResetFunctionality) {
    auto tokens1 = tokenize("x = 1");
    tokenizer.reset("y = 2");
    auto tokens2 = tokenizer.tokenize();

    ASSERT_EQ(tokens2.size(), 4);
    expectToken(tokens2[0], TokenType::IDENTIFIER, "y");
    expectToken(tokens2[1], TokenType::ASSIGN, "=");
    expectToken(tokens2[2], TokenType::NUMBER, "2");
    expectToken(tokens2[3], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, LargeFloatingPointNumbers) {
    auto tokens = tokenize("999999999.999999999 0.000000001");
    ASSERT_EQ(tokens.size(), 3);
    expectToken(tokens[0], TokenType::NUMBER, "999999999.999999999");
    expectToken(tokens[1], TokenType::NUMBER, "0.000000001");
    expectToken(tokens[2], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, ConsecutiveStringTokens) {
    auto tokens = tokenize("\"first\"\"second\"'third''fourth'");
    ASSERT_EQ(tokens.size(), 5);
    expectToken(tokens[0], TokenType::STRING, "first");
    expectToken(tokens[1], TokenType::STRING, "second");
    expectToken(tokens[2], TokenType::STRING, "third");
    expectToken(tokens[3], TokenType::STRING, "fourth");
    expectToken(tokens[4], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, ArrowOperatorEdgeCases) {
    auto tokens = tokenize("- -> - > -->");
    ASSERT_EQ(tokens.size(), 7);
    expectToken(tokens[0], TokenType::MINUS, "-");
    expectToken(tokens[1], TokenType::ARROW, "->");
    expectToken(tokens[2], TokenType::MINUS, "-");
    expectToken(tokens[3], TokenType::GREATER, ">");
    expectToken(tokens[4], TokenType::MINUS, "-");
    expectToken(tokens[5], TokenType::ARROW, "->");
    expectToken(tokens[6], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, SquareBrackets) {
    auto tokens = tokenize("[ ] [x] [1, 2, 3]");
    ASSERT_EQ(tokens.size(), 13);
    expectToken(tokens[0], TokenType::LSQUARE, "[");
    expectToken(tokens[1], TokenType::RSQUARE, "]");
    expectToken(tokens[2], TokenType::LSQUARE, "[");
    expectToken(tokens[3], TokenType::IDENTIFIER, "x");
    expectToken(tokens[4], TokenType::RSQUARE, "]");
    expectToken(tokens[5], TokenType::LSQUARE, "[");
    expectToken(tokens[6], TokenType::NUMBER, "1");
    expectToken(tokens[7], TokenType::COMMA, ",");
    expectToken(tokens[8], TokenType::NUMBER, "2");
    expectToken(tokens[9], TokenType::COMMA, ",");
    expectToken(tokens[10], TokenType::NUMBER, "3");
    expectToken(tokens[11], TokenType::RSQUARE, "]");
    expectToken(tokens[12], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, AllSingleCharacterTokens) {
    auto tokens = tokenize("(){}[],:+-*/=<>!&|");
    ASSERT_EQ(tokens.size(), 18);
    expectToken(tokens[0], TokenType::LPAREN, "(");
    expectToken(tokens[1], TokenType::RPAREN, ")");
    expectToken(tokens[2], TokenType::LBRACE, "{");
    expectToken(tokens[3], TokenType::RBRACE, "}");
    expectToken(tokens[4], TokenType::LSQUARE, "[");
    expectToken(tokens[5], TokenType::RSQUARE, "]");
    expectToken(tokens[6], TokenType::COMMA, ",");
    expectToken(tokens[7], TokenType::COLON, ":");
    expectToken(tokens[17], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, BoundaryConditionsAtEndOfInput) {
    auto tokens = tokenize("test");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::IDENTIFIER, "test");
    expectToken(tokens[1], TokenType::END_OF_FILE);

    tokens = tokenize("123");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::NUMBER, "123");
    expectToken(tokens[1], TokenType::END_OF_FILE);

    tokens = tokenize("\"test\"");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::STRING, "test");
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

// ============= COMMENT TESTS =============

TEST_F(TokenizerTest, BasicSingleLineComment) {
    auto tokens = tokenize("// This is a comment");
    ASSERT_EQ(tokens.size(), 1);
    expectToken(tokens[0], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, CommentWithCodeBefore) {
    auto tokens = tokenize("x = 5 // This is a comment");
    ASSERT_EQ(tokens.size(), 4);
    expectToken(tokens[0], TokenType::IDENTIFIER, "x");
    expectToken(tokens[1], TokenType::ASSIGN);
    expectToken(tokens[2], TokenType::NUMBER, "5");
    expectToken(tokens[3], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, CommentWithCodeAfter) {
    auto tokens = tokenize("// Comment\nx = 5");
    ASSERT_EQ(tokens.size(), 5);
    expectToken(tokens[0], TokenType::NEWLINE);
    expectToken(tokens[1], TokenType::IDENTIFIER, "x");
    expectToken(tokens[2], TokenType::ASSIGN);
    expectToken(tokens[3], TokenType::NUMBER, "5");
    expectToken(tokens[4], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, CommentWithNewline) {
    auto tokens = tokenize("// Comment\n");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::NEWLINE);
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, MultipleCommentsOnSeparateLines) {
    auto tokens = tokenize("// First comment\n// Second comment\nx = 1");
    ASSERT_EQ(tokens.size(), 6);
    expectToken(tokens[0], TokenType::NEWLINE);
    expectToken(tokens[1], TokenType::NEWLINE);
    expectToken(tokens[2], TokenType::IDENTIFIER, "x");
    expectToken(tokens[3], TokenType::ASSIGN);
    expectToken(tokens[4], TokenType::NUMBER, "1");
    expectToken(tokens[5], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, CommentWithSpecialCharacters) {
    auto tokens = tokenize("// Comment with @#$% special chars!");
    ASSERT_EQ(tokens.size(), 1);
    expectToken(tokens[0], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, CommentVsDivision) {
    // Test that single '/' is still division, not comment
    auto tokens = tokenize("x / y");
    ASSERT_EQ(tokens.size(), 4);
    expectToken(tokens[0], TokenType::IDENTIFIER, "x");
    expectToken(tokens[1], TokenType::DIV);
    expectToken(tokens[2], TokenType::IDENTIFIER, "y");
    expectToken(tokens[3], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, CommentVsDivisionAssignment) {
    // Test that '/=' is still division assignment, not comment
    auto tokens = tokenize("x /= 5");
    ASSERT_EQ(tokens.size(), 4);
    expectToken(tokens[0], TokenType::IDENTIFIER, "x");
    expectToken(tokens[1], TokenType::DIV_EQ);
    expectToken(tokens[2], TokenType::NUMBER, "5");
    expectToken(tokens[3], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, EmptyCommentLine) {
    auto tokens = tokenize("//\n");
    ASSERT_EQ(tokens.size(), 2);
    expectToken(tokens[0], TokenType::NEWLINE);
    expectToken(tokens[1], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, CommentAtEndOfFile) {
    auto tokens = tokenize("x = 5 // Comment at end");
    ASSERT_EQ(tokens.size(), 4);
    expectToken(tokens[0], TokenType::IDENTIFIER, "x");
    expectToken(tokens[1], TokenType::ASSIGN);
    expectToken(tokens[2], TokenType::NUMBER, "5");
    expectToken(tokens[3], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, CommentInExpression) {
    auto tokens = tokenize("3 + 5 // Adding numbers\n* 2");
    ASSERT_EQ(tokens.size(), 7);
    expectToken(tokens[0], TokenType::NUMBER, "3");
    expectToken(tokens[1], TokenType::PLUS);
    expectToken(tokens[2], TokenType::NUMBER, "5");
    expectToken(tokens[3], TokenType::NEWLINE);
    expectToken(tokens[4], TokenType::MULT);
    expectToken(tokens[5], TokenType::NUMBER, "2");
    expectToken(tokens[6], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, SpecialCharacterCombinations) {
    EXPECT_THROW(tokenize("@#$"), InvalidSyntaxException);
    EXPECT_THROW(tokenize("#define"), InvalidSyntaxException);
    EXPECT_THROW(tokenize("$variable"), InvalidSyntaxException);
    EXPECT_THROW(tokenize("test%"), InvalidSyntaxException);
}

// ============= COMPREHENSIVE OPERATOR COVERAGE TESTS =============

TEST_F(TokenizerTest, AllComparisonOperators) {
    auto tokens = tokenize("== != < > <= >=");
    ASSERT_EQ(tokens.size(), 7);
    expectToken(tokens[0], TokenType::EQUAL_EQUAL, "==");
    expectToken(tokens[1], TokenType::NOT_EQUAL, "!=");
    expectToken(tokens[2], TokenType::LESS, "<");
    expectToken(tokens[3], TokenType::GREATER, ">");
    expectToken(tokens[4], TokenType::LEQ, "<=");
    expectToken(tokens[5], TokenType::GEQ, ">=");
    expectToken(tokens[6], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, AllLogicalOperators) {
    auto tokens = tokenize("! && ||");
    ASSERT_EQ(tokens.size(), 4);
    expectToken(tokens[0], TokenType::NOT, "!");
    expectToken(tokens[1], TokenType::LOGIC_AND, "&&");
    expectToken(tokens[2], TokenType::LOGIC_OR, "||");
    expectToken(tokens[3], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, AllBitwiseOperators) {
    auto tokens = tokenize("& | ^");
    ASSERT_EQ(tokens.size(), 4);
    expectToken(tokens[0], TokenType::BITWISE_AND, "&");
    expectToken(tokens[1], TokenType::BITWISE_OR, "|");
    expectToken(tokens[2], TokenType::BITWISE_XOR, "^");
    expectToken(tokens[3], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, AllCompoundAssignmentOperators) {
    auto tokens = tokenize("+= -= *= /=");
    ASSERT_EQ(tokens.size(), 5);
    expectToken(tokens[0], TokenType::PLUS_EQ, "+=");
    expectToken(tokens[1], TokenType::MINUS_EQ, "-=");
    expectToken(tokens[2], TokenType::MULT_EQ, "*=");
    expectToken(tokens[3], TokenType::DIV_EQ, "/=");
    expectToken(tokens[4], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, ArrowOperatorVariations) {
    auto tokens = tokenize("-> -->-->");
    ASSERT_EQ(tokens.size(), 6);
    expectToken(tokens[0], TokenType::ARROW, "->");        // ->
    expectToken(tokens[1], TokenType::MINUS, "-");         // -
    expectToken(tokens[2], TokenType::ARROW, "->");        // ->
    expectToken(tokens[3], TokenType::MINUS, "-");         // -
    expectToken(tokens[4], TokenType::ARROW, "->");        // ->
    expectToken(tokens[5], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, OperatorPrecedenceTokens) {
    auto tokens = tokenize("a && b || c == d != e < f > g <= h >= i");
    ASSERT_EQ(tokens.size(), 18);
    expectToken(tokens[0], TokenType::IDENTIFIER, "a");
    expectToken(tokens[1], TokenType::LOGIC_AND, "&&");
    expectToken(tokens[2], TokenType::IDENTIFIER, "b");
    expectToken(tokens[3], TokenType::LOGIC_OR, "||");
    expectToken(tokens[4], TokenType::IDENTIFIER, "c");
    expectToken(tokens[5], TokenType::EQUAL_EQUAL, "==");
    expectToken(tokens[6], TokenType::IDENTIFIER, "d");
    expectToken(tokens[7], TokenType::NOT_EQUAL, "!=");
    expectToken(tokens[8], TokenType::IDENTIFIER, "e");
    expectToken(tokens[9], TokenType::LESS, "<");
    expectToken(tokens[10], TokenType::IDENTIFIER, "f");
    expectToken(tokens[11], TokenType::GREATER, ">");
    expectToken(tokens[12], TokenType::IDENTIFIER, "g");
    expectToken(tokens[13], TokenType::LEQ, "<=");
    expectToken(tokens[14], TokenType::IDENTIFIER, "h");
    expectToken(tokens[15], TokenType::GEQ, ">=");
    expectToken(tokens[16], TokenType::IDENTIFIER, "i");
    expectToken(tokens[17], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, MixedOperatorSequences) {
    auto tokens = tokenize("x+=y*=z/=w-=v");
    ASSERT_EQ(tokens.size(), 10);
    expectToken(tokens[0], TokenType::IDENTIFIER, "x");
    expectToken(tokens[1], TokenType::PLUS_EQ, "+=");
    expectToken(tokens[2], TokenType::IDENTIFIER, "y");
    expectToken(tokens[3], TokenType::MULT_EQ, "*=");
    expectToken(tokens[4], TokenType::IDENTIFIER, "z");
    expectToken(tokens[5], TokenType::DIV_EQ, "/=");
    expectToken(tokens[6], TokenType::IDENTIFIER, "w");
    expectToken(tokens[7], TokenType::MINUS_EQ, "-=");
    expectToken(tokens[8], TokenType::IDENTIFIER, "v");
    expectToken(tokens[9], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, BitwiseVsLogicalOperators) {
    auto tokens = tokenize("a & b && c | d || e");
    ASSERT_EQ(tokens.size(), 10);
    expectToken(tokens[0], TokenType::IDENTIFIER, "a");
    expectToken(tokens[1], TokenType::BITWISE_AND, "&");
    expectToken(tokens[2], TokenType::IDENTIFIER, "b");
    expectToken(tokens[3], TokenType::LOGIC_AND, "&&");
    expectToken(tokens[4], TokenType::IDENTIFIER, "c");
    expectToken(tokens[5], TokenType::BITWISE_OR, "|");
    expectToken(tokens[6], TokenType::IDENTIFIER, "d");
    expectToken(tokens[7], TokenType::LOGIC_OR, "||");
    expectToken(tokens[8], TokenType::IDENTIFIER, "e");
    expectToken(tokens[9], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, OperatorAtTokenBoundaries) {
    auto tokens = tokenize("&&& ||| === !== <<< >>>>");
    ASSERT_EQ(tokens.size(), 16);
    expectToken(tokens[0], TokenType::LOGIC_AND, "&&");   // &&
    expectToken(tokens[1], TokenType::BITWISE_AND, "&");  // &
    expectToken(tokens[2], TokenType::LOGIC_OR, "||");    // ||
    expectToken(tokens[3], TokenType::BITWISE_OR, "|");   // |
    expectToken(tokens[4], TokenType::EQUAL_EQUAL, "=="); // ==
    expectToken(tokens[5], TokenType::ASSIGN, "=");       // =
    expectToken(tokens[6], TokenType::NOT_EQUAL, "!=");   // !=
    expectToken(tokens[7], TokenType::ASSIGN, "=");       // =
    expectToken(tokens[8], TokenType::LESS, "<");         // <
    expectToken(tokens[9], TokenType::LESS, "<");         // <
    expectToken(tokens[10], TokenType::LESS, "<");        // <
    expectToken(tokens[11], TokenType::GREATER, ">");     // >
    expectToken(tokens[12], TokenType::GREATER, ">");     // >
    expectToken(tokens[13], TokenType::GREATER, ">");     // >
    expectToken(tokens[14], TokenType::GREATER, ">");     // >
    expectToken(tokens[15], TokenType::END_OF_FILE);
}

// ============= EDGE CASE KEYWORD TESTS =============

TEST_F(TokenizerTest, AllKeywordVariations) {
    auto tokens = tokenize("if else while for def return int str bool float double true false");
    ASSERT_EQ(tokens.size(), 14);
    expectToken(tokens[0], TokenType::IF, "if");
    expectToken(tokens[1], TokenType::ELSE, "else");
    expectToken(tokens[2], TokenType::WHILE, "while");
    expectToken(tokens[3], TokenType::FOR, "for");
    expectToken(tokens[4], TokenType::DEF, "def");
    expectToken(tokens[5], TokenType::RETURN, "return");
    expectToken(tokens[6], TokenType::INT, "int");
    expectToken(tokens[7], TokenType::STR, "str");
    expectToken(tokens[8], TokenType::BOOL, "bool");
    expectToken(tokens[9], TokenType::FLOAT, "float");
    expectToken(tokens[10], TokenType::DOUBLE, "double");
    expectToken(tokens[11], TokenType::TRUE, "true");
    expectToken(tokens[12], TokenType::FALSE, "false");
    expectToken(tokens[13], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, KeywordInComplexContext) {
    auto tokens = tokenize("def sum(a: int, b: int) -> int: return a + b");
    ASSERT_EQ(tokens.size(), 19);
    expectToken(tokens[0], TokenType::DEF, "def");
    expectToken(tokens[1], TokenType::IDENTIFIER, "sum");
    expectToken(tokens[2], TokenType::LPAREN, "(");
    expectToken(tokens[3], TokenType::IDENTIFIER, "a");
    expectToken(tokens[4], TokenType::COLON, ":");
    expectToken(tokens[5], TokenType::INT, "int");
    expectToken(tokens[6], TokenType::COMMA, ",");
    expectToken(tokens[7], TokenType::IDENTIFIER, "b");
    expectToken(tokens[8], TokenType::COLON, ":");
    expectToken(tokens[9], TokenType::INT, "int");
    expectToken(tokens[10], TokenType::RPAREN, ")");
    expectToken(tokens[11], TokenType::ARROW, "->");
    expectToken(tokens[12], TokenType::INT, "int");
    expectToken(tokens[13], TokenType::COLON, ":");
    expectToken(tokens[14], TokenType::RETURN, "return");
    expectToken(tokens[15], TokenType::IDENTIFIER, "a");
    expectToken(tokens[16], TokenType::PLUS, "+");
    expectToken(tokens[17], TokenType::IDENTIFIER, "b");
    expectToken(tokens[18], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, KeywordBoundaryEdgeCases) {
    // Ensure keywords only match when they are complete words
    auto tokens = tokenize("iffy elsewise whiles forex defined returned");
    ASSERT_EQ(tokens.size(), 7);
    expectToken(tokens[0], TokenType::IDENTIFIER, "iffy");
    expectToken(tokens[1], TokenType::IDENTIFIER, "elsewise");
    expectToken(tokens[2], TokenType::IDENTIFIER, "whiles");
    expectToken(tokens[3], TokenType::IDENTIFIER, "forex");
    expectToken(tokens[4], TokenType::IDENTIFIER, "defined");
    expectToken(tokens[5], TokenType::IDENTIFIER, "returned");
    expectToken(tokens[6], TokenType::END_OF_FILE);
}

// ============= ARRAY LEXER TESTS =============

TEST_F(TokenizerTest, ArrayTypeDeclaration) {
    auto tokens = tokenize("Array[int]");
    ASSERT_EQ(tokens.size(), 5);
    expectToken(tokens[0], TokenType::IDENTIFIER, "Array");
    expectToken(tokens[1], TokenType::LSQUARE);
    expectToken(tokens[2], TokenType::INT);
    expectToken(tokens[3], TokenType::RSQUARE);
    expectToken(tokens[4], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, ArrayLiteralEmpty) {
    auto tokens = tokenize("[]");
    ASSERT_EQ(tokens.size(), 3);
    expectToken(tokens[0], TokenType::LSQUARE);
    expectToken(tokens[1], TokenType::RSQUARE);
    expectToken(tokens[2], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, ArrayLiteralWithNumbers) {
    auto tokens = tokenize("[1, 2, 3]");
    ASSERT_EQ(tokens.size(), 8);
    expectToken(tokens[0], TokenType::LSQUARE);
    expectToken(tokens[1], TokenType::NUMBER, "1");
    expectToken(tokens[2], TokenType::COMMA);
    expectToken(tokens[3], TokenType::NUMBER, "2");
    expectToken(tokens[4], TokenType::COMMA);
    expectToken(tokens[5], TokenType::NUMBER, "3");
    expectToken(tokens[6], TokenType::RSQUARE);
    expectToken(tokens[7], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, ArrayLiteralWithStrings) {
    auto tokens = tokenize("[\"hello\", \"world\"]");
    ASSERT_EQ(tokens.size(), 6);
    expectToken(tokens[0], TokenType::LSQUARE);
    expectToken(tokens[1], TokenType::STRING, "hello");
    expectToken(tokens[2], TokenType::COMMA);
    expectToken(tokens[3], TokenType::STRING, "world");
    expectToken(tokens[4], TokenType::RSQUARE);
    expectToken(tokens[5], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, ArrayIndexAccess) {
    auto tokens = tokenize("arr[0]");
    ASSERT_EQ(tokens.size(), 5);
    expectToken(tokens[0], TokenType::IDENTIFIER, "arr");
    expectToken(tokens[1], TokenType::LSQUARE);
    expectToken(tokens[2], TokenType::NUMBER, "0");
    expectToken(tokens[3], TokenType::RSQUARE);
    expectToken(tokens[4], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, NestedArrayTypes) {
    auto tokens = tokenize("Array[Array[int]]");
    ASSERT_EQ(tokens.size(), 8);
    expectToken(tokens[0], TokenType::IDENTIFIER, "Array");
    expectToken(tokens[1], TokenType::LSQUARE);
    expectToken(tokens[2], TokenType::IDENTIFIER, "Array");
    expectToken(tokens[3], TokenType::LSQUARE);
    expectToken(tokens[4], TokenType::INT);
    expectToken(tokens[5], TokenType::RSQUARE);
    expectToken(tokens[6], TokenType::RSQUARE);
    expectToken(tokens[7], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, ArrayVariableDeclaration) {
    auto tokens = tokenize("numbers: Array[int] = Array.new()");
    ASSERT_EQ(tokens.size(), 13);
    expectToken(tokens[0], TokenType::IDENTIFIER, "numbers");
    expectToken(tokens[1], TokenType::COLON);
    expectToken(tokens[2], TokenType::IDENTIFIER, "Array");
    expectToken(tokens[3], TokenType::LSQUARE);
    expectToken(tokens[4], TokenType::INT);
    expectToken(tokens[5], TokenType::RSQUARE);
    expectToken(tokens[6], TokenType::ASSIGN);
    expectToken(tokens[7], TokenType::IDENTIFIER, "Array");
    expectToken(tokens[8], TokenType::DOT);
    expectToken(tokens[9], TokenType::IDENTIFIER, "new");
    expectToken(tokens[10], TokenType::LPAREN, "(");
    expectToken(tokens[11], TokenType::RPAREN, ")");
}

TEST_F(TokenizerTest, ArrayMethodCalls) {
    auto tokens = tokenize("numbers.push(42)");
    ASSERT_EQ(tokens.size(), 7);
    expectToken(tokens[0], TokenType::IDENTIFIER, "numbers");
    expectToken(tokens[1], TokenType::DOT);
    expectToken(tokens[2], TokenType::IDENTIFIER, "push");
    expectToken(tokens[3], TokenType::LPAREN);
    expectToken(tokens[4], TokenType::NUMBER, "42");
    expectToken(tokens[5], TokenType::RPAREN);
    expectToken(tokens[6], TokenType::END_OF_FILE);
}

TEST_F(TokenizerTest, ArrayMethodChaining) {
    auto tokens = tokenize("arr.push(1).push(2).length()");
    ASSERT_EQ(tokens.size(), 16);
    expectToken(tokens[0], TokenType::IDENTIFIER, "arr");
    expectToken(tokens[1], TokenType::DOT);
    expectToken(tokens[2], TokenType::IDENTIFIER, "push");
    expectToken(tokens[3], TokenType::LPAREN);
    expectToken(tokens[4], TokenType::NUMBER, "1");
    expectToken(tokens[5], TokenType::RPAREN);
    expectToken(tokens[6], TokenType::DOT);
    expectToken(tokens[7], TokenType::IDENTIFIER, "push");
    expectToken(tokens[8], TokenType::LPAREN);
    expectToken(tokens[9], TokenType::NUMBER, "2");
    expectToken(tokens[10], TokenType::RPAREN);
    expectToken(tokens[11], TokenType::DOT);
    expectToken(tokens[12], TokenType::IDENTIFIER, "length");
    expectToken(tokens[13], TokenType::LPAREN);
    expectToken(tokens[14], TokenType::RPAREN);
}

TEST_F(TokenizerTest, ComplexArrayExpression) {
    auto tokens = tokenize("matrix[i][j] = arr[0] + 5");
    ASSERT_EQ(tokens.size(), 15);
    expectToken(tokens[0], TokenType::IDENTIFIER, "matrix");
    expectToken(tokens[1], TokenType::LSQUARE);
    expectToken(tokens[2], TokenType::IDENTIFIER, "i");
    expectToken(tokens[3], TokenType::RSQUARE);
    expectToken(tokens[4], TokenType::LSQUARE);
    expectToken(tokens[5], TokenType::IDENTIFIER, "j");
    expectToken(tokens[6], TokenType::RSQUARE);
    expectToken(tokens[7], TokenType::ASSIGN);
    expectToken(tokens[8], TokenType::IDENTIFIER, "arr");
    expectToken(tokens[9], TokenType::LSQUARE);
    expectToken(tokens[10], TokenType::NUMBER, "0");
    expectToken(tokens[11], TokenType::RSQUARE);
    expectToken(tokens[12], TokenType::PLUS);
}