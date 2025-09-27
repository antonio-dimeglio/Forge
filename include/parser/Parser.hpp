#pragma once
#include "../ast/Expression.hpp"
#include "../ast/Statement.hpp"
#include "../ast/ParsedType.hpp"
#include "../lexer/Token.hpp"
#include <vector>
#include <memory>
#include <optional>

using namespace forge::ast;

class Parser {
    private:
        std::vector<Token> tokens; 
        size_t idx = 0;

        Token current() const;
        Token peek(int offset = 1) const;
        Token advance();
        bool isValidTypeToken(TokenType type);

        std::unique_ptr<forge::ast::Expression> parsePrimary();
        std::unique_ptr<forge::ast::Expression> parseOptional();
        std::unique_ptr<forge::ast::Expression> parseLiteral();
        std::unique_ptr<forge::ast::Expression> parseIdentifierExpression();
        std::unique_ptr<forge::ast::Expression> parseParenthesizedExpression();
        std::unique_ptr<forge::ast::Expression> parseArrayLiteral();
        std::unique_ptr<forge::ast::Expression> parseNewExpression();
        std::unique_ptr<forge::ast::Expression> parseGenericInstantiation(Token className);
        std::unique_ptr<forge::ast::Expression> parsePostfix();
        std::unique_ptr<forge::ast::Expression> parseUnary();
        std::unique_ptr<forge::ast::Expression> parseFactor();
        std::unique_ptr<forge::ast::Expression> parseTerm();
        std::unique_ptr<forge::ast::Expression> parseComparison();
        std::unique_ptr<forge::ast::Expression> parseEquality();
        std::unique_ptr<forge::ast::Expression> parseBitwiseAnd();
        std::unique_ptr<forge::ast::Expression> parseBitwiseXor();
        std::unique_ptr<forge::ast::Expression> parseBitwiseOr();
        std::unique_ptr<forge::ast::Expression> parseLogicalAnd();
        std::unique_ptr<forge::ast::Expression> parseLogicalOr();
        
        std::vector<std::unique_ptr<forge::ast::Expression>> parseArgumentList();

        Token expect(TokenType expectedType, const std::string& errorMessage);

        std::unique_ptr<forge::ast::Statement> parseExpressionStatement();
        std::unique_ptr<forge::ast::Statement> parseClassDefinition();
        forge::ast::FieldDefinition parseFieldDefinition();
        forge::ast::MethodDefinition parseMethodDefinition();
        std::unique_ptr<forge::ast::Statement> parseVariableDeclaration();
        std::unique_ptr<forge::ast::Statement> parseAssignment();
        std::unique_ptr<forge::ast::Statement> parseInferredDeclaration();
        std::unique_ptr<forge::ast::Statement> parseIndexAssignmentOrExpression();
        std::unique_ptr<forge::ast::Statement> parseAssignmentOrExpression();
        std::unique_ptr<forge::ast::Statement> parseIfStatement();
        std::unique_ptr<forge::ast::Statement> parseWhileStatement();
        std::unique_ptr<forge::ast::Statement> parseFunctionDefinition();
        std::unique_ptr<forge::ast::Statement> parseReturnStatement();
        std::unique_ptr<forge::ast::Statement> parseDeferStatement();
        std::unique_ptr<forge::ast::Statement> parseExternStatement();
        std::unique_ptr<forge::ast::BlockStatement> parseBlockStatement();
        bool hasReturnStatement(const forge::ast::BlockStatement* block);

        void skipNewLines();

        // Helper function to create source location from current token
        forge::errors::SourceLocation getCurrentLocation() const;
        forge::errors::SourceLocation getTokenLocation(const Token& token) const;
    public:
        Parser(const std::vector<Token>& tokens) : tokens(tokens) {};
        std::optional<forge::ast::ParsedType> parseType();
        std::unique_ptr<forge::ast::Statement> parseStatement();
        std::unique_ptr<forge::ast::Expression> parseExpression();
        std::unique_ptr<forge::ast::Statement> parseProgram();
        void reset(const std::vector<Token>& tokens);
        bool isAtEnd();
        Token getCurrentToken();
};