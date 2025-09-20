#include "../../include/parser/Parser.hpp"
#include "../../include/parser/ParserException.hpp"

Token Parser::current() {
    if (idx >= tokens.size()) {
        return Token(TokenType::END_OF_FILE, -1, -1);
    }

    return tokens[idx];
}

Token Parser::peek(int offset) {
    size_t actual = idx + offset; 

    if (actual >= tokens.size()) {
        return Token(TokenType::END_OF_FILE, -1, -1);
    }

    return tokens[actual];
}

Token Parser::advance() {
    Token token = current();
    if (idx < tokens.size()) idx++;
    return token;
}

bool Parser::isAtEnd() {
    return idx == tokens.size();
}

void Parser::reset(const std::vector<Token>& tokens) {
    this->tokens = tokens;
    idx = 0;
}


std::unique_ptr<Expression> Parser::generateAST() {
    auto root = expression();
    
    if (current().getType() != TokenType::END_OF_FILE) {
        throw ParsingException("Failed to parse file, stopped at ", current().getLine(), current().getColumn());
    }
    return root; 
}

std::unique_ptr<Expression> Parser::expression() {
    return equality();
}

std::unique_ptr<Expression> Parser::equality() {
    auto expr = comparison();

    while (current().getType() == TokenType::EQUAL_EQUAL ||
            current().getType() == TokenType::NOT_EQUAL) {

        Token operator_ = advance();
        auto right = comparison();

        expr = std::make_unique<BinaryExpression>(
            std::move(expr), operator_, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::comparison() {
    auto expr = term();

    while (current().getType() == TokenType::GREATER ||
        current().getType() == TokenType::GEQ || 
        current().getType() == TokenType::LESS || 
        current().getType() == TokenType::LEQ ) {
        
        Token operator_ = advance(); 
        auto right = term();
        expr = std::make_unique<BinaryExpression>(
            std::move(expr), operator_, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::term() {
    auto expr = factor();

    while (current().getType() == TokenType::PLUS ||
            current().getType() == TokenType::MINUS) {

        Token operator_ = advance();
        auto right = factor();

        expr = std::make_unique<BinaryExpression>(
            std::move(expr), operator_, std::move(right));
    }

    return expr; 
}

std::unique_ptr<Expression> Parser::factor() {
    auto expr = unary(); 

    while (current().getType() == TokenType::MULT ||
            current().getType() == TokenType::DIV) {
        Token operator_ = advance(); 
        auto right = unary(); 
        expr = std::make_unique<BinaryExpression>(
            std::move(expr), operator_, std::move(right));
    }

    return expr; 
}

std::unique_ptr<Expression> Parser::unary() {
    Token token = current(); 

    switch (token.getType()) {
        case TokenType::NOT:
        case TokenType::MINUS: {
                advance();
                auto operand = unary();
                return std::make_unique<UnaryExpression>(token, std::move(operand));
            }
        default:
            return primary();
    }
}

std::unique_ptr<Expression> Parser::primary() {
    Token token = current();

    switch (token.getType()) {
        case TokenType::NUMBER:
        case TokenType::STRING:
        case TokenType::TRUE:
        case TokenType::FALSE:
            advance();
            return std::make_unique<LiteralExpression>(token);
        case TokenType::IDENTIFIER:
            advance();
            return std::make_unique<IdentifierExpression>(token.getValue());
        case TokenType::LPAREN: {
                advance();
                auto expr = expression(); 
                if (current().getType() != TokenType::RPAREN) {
                    throw ParsingException("Expected )", current().getLine(), current().getColumn());
                }
                advance();
                return expr;
            }
        default:
            throw ParsingException("Invalid expression ", current().getLine(), current().getColumn());
    };
}