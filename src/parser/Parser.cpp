#include "../../include/parser/Parser.hpp"
#include "../../include/parser/ParserException.hpp"

void Parser::skipNewLines() {
    while (current().getType() == TokenType::NEWLINE && !isAtEnd()){
        advance();
    }
}

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


std::unique_ptr<Statement> Parser::parseStatement() {
    skipNewLines();

    if (isAtEnd() || current().getType() == TokenType::END_OF_FILE) {
        return nullptr;
    }

    TokenType currentType = current().getType();
    TokenType nextType = peek().getType();

    if (currentType == TokenType::IF) {
        return parseIfStatement();
    }
    if (currentType == TokenType::WHILE) {
        return parseWhileStatement();
    }
    if (currentType == TokenType::DEF) {
        return parseFunctionDefinition();
    }
    if (currentType == TokenType::RETURN) {
        return parseReturnStatement();
    }
    if (currentType == TokenType::IDENTIFIER) {
        if (nextType == TokenType::COLON) {
            return parseVariableDeclaration();
        }
        if (nextType == TokenType::ASSIGN) {
            return parseAssignment();
        } // Here function call should go
        if (nextType == TokenType::NUMBER || nextType == TokenType::IDENTIFIER) {
            Token next = peek();
            throw ParsingException("Unexpected token after identifier. Did you mean to use '=' for assignment or ':' for declaration?", next.getLine(), next.getColumn());
        }
    }

    return parseExpressionStatement();
}

std::unique_ptr<Statement> Parser::parseExpressionStatement() {
    auto expr = parseExpression();

    Token t = current();
    if (t.getType() != TokenType::NEWLINE &&
        t.getType() != TokenType::END_OF_FILE) {
        throw ParsingException("Unproperly terminated line", t.getLine(), t.getColumn());
    }
    
    return std::make_unique<ExpressionStatement>(std::move(expr));
}

std::unique_ptr<Expression> Parser::parseExpression() {
    return parseLogicalOr();
}

std::unique_ptr<Expression> Parser::parseLogicalOr() {
    auto expr = parseLogicalAnd();

    while (current().getType() == TokenType::LOGIC_OR) {
        Token operator_ = advance();
        auto right = parseLogicalAnd();

        expr = std::make_unique<BinaryExpression>(
            std::move(expr), operator_, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::parseLogicalAnd() {
    auto expr = parseEquality();

    while (current().getType() == TokenType::LOGIC_AND) {
        Token operator_ = advance();
        auto right = parseEquality();

        expr = std::make_unique<BinaryExpression>(
            std::move(expr), operator_, std::move(right));
    }

    return expr;
}


std::unique_ptr<Expression> Parser::parseBitwiseOr() {
    auto expr = parseBitwiseXor();

    while (current().getType() == TokenType::BITWISE_OR) {
        Token operator_ = advance();
        auto right = parseBitwiseXor();

        expr = std::make_unique<BinaryExpression>(
            std::move(expr), operator_, std::move(right));
    }

    return expr;
}


std::unique_ptr<Expression> Parser::parseBitwiseXor() {
    auto expr = parseBitwiseAnd();

    while (current().getType() == TokenType::BITWISE_XOR) {
        Token operator_ = advance();
        auto right = parseBitwiseAnd();

        expr = std::make_unique<BinaryExpression>(
            std::move(expr), operator_, std::move(right));
    }

    return expr;
}


std::unique_ptr<Expression> Parser::parseBitwiseAnd() {
    auto expr = parseTerm();

    while (current().getType() == TokenType::BITWISE_AND) {
        Token operator_ = advance();
        auto right = parseTerm();

        expr = std::make_unique<BinaryExpression>(
            std::move(expr), operator_, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::parseEquality() {
    auto expr = parseComparison();

    while (current().getType() == TokenType::EQUAL_EQUAL ||
            current().getType() == TokenType::NOT_EQUAL) {

        Token operator_ = advance();
        auto right = parseComparison();

        expr = std::make_unique<BinaryExpression>(
            std::move(expr), operator_, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::parseComparison() {
    auto expr = parseBitwiseOr();

    while (current().getType() == TokenType::GREATER ||
        current().getType() == TokenType::GEQ || 
        current().getType() == TokenType::LESS || 
        current().getType() == TokenType::LEQ ) {
        
        Token operator_ = advance(); 
        auto right = parseBitwiseOr();
        expr = std::make_unique<BinaryExpression>(
            std::move(expr), operator_, std::move(right));
    }

    return expr;
}

std::unique_ptr<Expression> Parser::parseTerm() {
    auto expr = parseFactor();

    while (current().getType() == TokenType::PLUS ||
            current().getType() == TokenType::MINUS) {

        Token operator_ = advance();
        auto right = parseFactor();

        expr = std::make_unique<BinaryExpression>(
            std::move(expr), operator_, std::move(right));
    }

    return expr; 
}

std::unique_ptr<Expression> Parser::parseFactor() {
    auto expr = parseUnary(); 

    while (current().getType() == TokenType::MULT ||
            current().getType() == TokenType::DIV) {
        Token operator_ = advance(); 
        auto right = parseUnary(); 
        expr = std::make_unique<BinaryExpression>(
            std::move(expr), operator_, std::move(right));
    }

    return expr; 
}

std::unique_ptr<Expression> Parser::parseUnary() {
    Token token = current(); 

    switch (token.getType()) {
        case TokenType::NOT:
        case TokenType::MINUS: {
                advance();
                auto operand = parseUnary();
                return std::make_unique<UnaryExpression>(token, std::move(operand));
            }
        default:
            return parsePrimary();
    }
}

std::unique_ptr<Expression> Parser::parsePrimary() {
    Token token = current();

    switch (token.getType()) {
        case TokenType::NUMBER:
        case TokenType::STRING:
        case TokenType::TRUE:
        case TokenType::FALSE:
            advance();
            return std::make_unique<LiteralExpression>(token);
        case TokenType::IDENTIFIER:
            {
                Token identifier = advance();
                if (current().getType() == TokenType::LPAREN) {
                    advance(); 

                    std::vector<std::unique_ptr<Expression>> arguments;

                    if (current().getType() != TokenType::RPAREN) {
                        while (true) {
                            auto arg = parseExpression();
                            arguments.push_back(std::move(arg));
                           
                            if (current().getType() == TokenType::COMMA) {
                                advance();
                                continue;
                            } else if (current().getType() == TokenType::RPAREN) {
                                break; 
                            } else {
                                throw ParsingException("Expected ',' or ')' in function call", current().getLine(), current().getColumn());
                            }
                        }
                    }

                    if (current().getType() != TokenType::RPAREN) {
                        throw ParsingException("Expected )", current().getLine(), current().getColumn());
                    }
                    advance(); 

                    return std::make_unique<FunctionCall>(identifier.getValue(), std::move(arguments));
                } else {
                    return std::make_unique<IdentifierExpression>(identifier.getValue());
                }
            }
            return std::make_unique<IdentifierExpression>(token.getValue());
        case TokenType::LPAREN: {
                advance();
                auto expr = parseExpression(); 
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


std::unique_ptr<Statement> Parser::parseVariableDeclaration() {
    Token identifier = advance();
    Token t(TokenType::END_OF_FILE, -1, -1);

    if (identifier.getType() != TokenType::IDENTIFIER) {
        throw ParsingException("Expected identifier", identifier.getLine(), identifier.getColumn());
    }
    if ((t = advance()).getType() != TokenType::COLON) {
        throw ParsingException("Expected :", t.getLine(), t.getColumn());
    }

    Token type = advance();

    switch (type.getType()) {
        case TokenType::INT:
        case TokenType::FLOAT:
        case TokenType::DOUBLE:
        case TokenType::BOOL:
        case TokenType::STR:
            break; 
        default:
            throw ParsingException("Expected type ", type.getLine(), type.getColumn());
    }

    if (advance().getType() != TokenType::ASSIGN) {
        throw ParsingException("Expected =", current().getLine(), current().getColumn());
    }

    auto expression = parseExpression();

    return std::make_unique<VariableDeclaration>(
        identifier,   
        type,         
        std::move(expression)  
    );   
}

std::unique_ptr<Statement> Parser::parseAssignment() {
    Token identifier = advance();
    Token t(TokenType::END_OF_FILE, -1, -1);

    if (identifier.getType() != TokenType::IDENTIFIER) {
        throw ParsingException("Expected identifier", identifier.getLine(), identifier.getColumn());
    }

    if ((t = advance()).getType() != TokenType::ASSIGN) {
        throw ParsingException("Expected =", t.getLine(), t.getColumn());
    }

    auto expression = parseExpression();

    return std::make_unique<Assignment>(
        identifier,         
        std::move(expression)  
    ); 
}   

std::unique_ptr<BlockStatement> Parser::parseBlockStatement() {
    Token t(TokenType::END_OF_FILE, -1, -1);

    if ((t = advance()).getType() != TokenType::LBRACE) {
        throw ParsingException("Expected {", t.getLine(), t.getColumn());
    }

    std::vector<std::unique_ptr<Statement>> statements;
    while (current().getType() != TokenType::RBRACE) {
        if (isAtEnd()) {
            throw ParsingException("Expected }", current().getLine(), current().getColumn());
        }
        auto stmt = parseStatement();
        if (stmt != nullptr) {
            statements.push_back(std::move(stmt));
        }
        skipNewLines();
    }

    if ((t = advance()).getType() != TokenType::RBRACE) {
        throw ParsingException("Expected }", t.getLine(), t.getColumn());
    }

    return std::make_unique<BlockStatement>(std::move(statements));
}

std::unique_ptr<Statement> Parser::parseIfStatement() {
    Token t(TokenType::END_OF_FILE, -1, -1);

    if ((t = advance()).getType() != TokenType::IF) {
        throw ParsingException("Expected if", t.getLine(), t.getColumn());
    }

    if ((t = advance()).getType() != TokenType::LPAREN) {
        throw ParsingException("Expected (", t.getLine(), t.getColumn());
    }

    auto expression = parseExpression();

    if ((t = advance()).getType() != TokenType::RPAREN) {
        throw ParsingException("Expected )", t.getLine(), t.getColumn());
    }

    // Parse then block
    auto thenBlock = parseBlockStatement();

    // Check for else clause
    std::unique_ptr<BlockStatement> elseBlock = nullptr;
    if (current().getType() == TokenType::ELSE) {
        advance(); // consume ELSE
        elseBlock = parseBlockStatement();
    }

    return std::make_unique<IfStatement>(
        std::move(expression),
        std::move(thenBlock),
        std::move(elseBlock)
    );
}

std::unique_ptr<Statement> Parser::parseWhileStatement() {
    Token t(TokenType::END_OF_FILE, -1, -1);

    if ((t = advance()).getType() != TokenType::WHILE) {
        throw ParsingException("Expected while", t.getLine(), t.getColumn());
    }

    if ((t = advance()).getType() != TokenType::LPAREN) {
        throw ParsingException("Expected (", t.getLine(), t.getColumn());
    }

    auto expression = parseExpression();

    if ((t = advance()).getType() != TokenType::RPAREN) {
        throw ParsingException("Expected )", t.getLine(), t.getColumn());
    }

    auto body = parseBlockStatement();


    return std::make_unique<WhileStatement>(
        std::move(expression),
        std::move(body)
    );
}

std::unique_ptr<Statement> Parser::parseReturnStatement() {
    Token t(TokenType::END_OF_FILE, -1, -1);
    if ((t = advance()).getType() != TokenType::RETURN) {
        throw ParsingException("Expected return", t.getLine(), t.getColumn());
    }

    auto value = parseExpression();

    return std::make_unique<ReturnStatement>(
        std::move(value)
    );
}

std::unique_ptr<Statement> Parser::parseFunctionDefinition() {
    // TODO: REfactor multiple switch statements to check if token is a type.
    Token t(TokenType::END_OF_FILE, -1, -1);

    if ((t = advance()).getType() != TokenType::DEF) {
        throw ParsingException("Expected def", t.getLine(), t.getColumn());
    }

    Token functionName = advance();
    if (functionName.getType() != TokenType::IDENTIFIER) {
        throw ParsingException("Expected variable", functionName.getLine(), functionName.getColumn());
    }

    if ((t = advance()).getType() != TokenType::LPAREN) {
        throw ParsingException("Expected (", t.getLine(), t.getColumn());
    }

    std::vector<StatementParameter> params;

    
    if (current().getType() != TokenType::RPAREN) {
        while (true) {
            Token name = advance();
            if (name.getType() != TokenType::IDENTIFIER) {
                throw ParsingException("Expected parameter name", name.getLine(), name.getColumn());
            }

            if ((t = advance()).getType() != TokenType::COLON) {
                throw ParsingException("Expected :", t.getLine(), t.getColumn());
            }

            Token type = advance();
            switch (type.getType()) {
                case TokenType::INT:
                case TokenType::FLOAT:
                case TokenType::BOOL:
                case TokenType::STR:
                case TokenType::DOUBLE:
                    break;
                default:
                    throw ParsingException("Expected type", type.getLine(), type.getColumn());
            }

            params.push_back(StatementParameter {
                .name = name,
                .type = type
            });

            
            if (current().getType() == TokenType::COMMA) {
                advance(); 
                continue; 
            } else if (current().getType() == TokenType::RPAREN) {
                break;
            } else {
                throw ParsingException("Expected ',' or ')'", current().getLine(), current().getColumn());
            }
        }
    }

    
    if ((t = advance()).getType() != TokenType::RPAREN) {
        throw ParsingException("Expected )", t.getLine(), t.getColumn());
    }

    if ((t = advance()).getType() != TokenType::ARROW) {
        throw ParsingException("Expected ->", t.getLine(), t.getColumn());
    }

    Token functionReturnType = advance();

    switch (functionReturnType.getType()) {
        case TokenType::INT:
        case TokenType::FLOAT:
        case TokenType::BOOL:
        case TokenType::STR:
        case TokenType::DOUBLE:
            break;
        default:
            throw ParsingException("Expected type", functionReturnType.getLine(), functionReturnType.getColumn());
    }

    auto body = parseBlockStatement();

    return std::make_unique<FunctionDefinition>(
            functionName,
            functionReturnType,
            params,
            std::move(body)
        );
}

std::unique_ptr<Statement> Parser::parseProgram() {
    skipNewLines();
    std::vector<std::unique_ptr<Statement>> statements;

    while (!isAtEnd()) {
        auto stmt = parseStatement();
        if (stmt == nullptr) {
            break;  // No more statements to parse
        }
        statements.push_back(std::move(stmt));
        skipNewLines();
    }

    return std::make_unique<Program>(std::move(statements));
}

