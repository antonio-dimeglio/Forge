#include "../../include/parser/Parser.hpp"
#include "../../include/parser/ParserException.hpp"
#include <iostream>

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

Token Parser::getCurrentToken() {
    return current();
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
    if (currentType == TokenType::LBRACE) {
        return std::unique_ptr<Statement>(parseBlockStatement().release());
    }
    if (currentType == TokenType::IDENTIFIER) {
        if (nextType == TokenType::COLON) {
            return parseVariableDeclaration();
        }
        if (nextType == TokenType::ASSIGN) {
            return parseAssignment();
        }
        if (nextType == TokenType::LSQUARE) {
            // This could be array index assignment: arr[index] = value
            // We need to look ahead further to see if there's an = after the ]
            return parseIndexAssignmentOrExpression();
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
            return parsePostfix();
    }
}

Token Parser::expect(TokenType expectedType, const std::string& errorMessage) {
    if (current().getType() != expectedType) {
        throw ParsingException(errorMessage, current().getLine(), current().getColumn());
    }
    return advance();
}

std::vector<std::unique_ptr<Expression>> Parser::parseArgumentList() {
    std::vector<std::unique_ptr<Expression>> arguments;

    if (current().getType() != TokenType::RPAREN) {
        while (true) {
            arguments.push_back(parseExpression());

            if (current().getType() == TokenType::COMMA) {
                advance(); // consume comma
                continue;
            } else if (current().getType() == TokenType::RPAREN) {
                break; // end of arguments
            } else {
                throw ParsingException("Expected ',' or ')' in argument list", current().getLine(), current().getColumn());
            }
        }
    }

    return arguments;
}

std::unique_ptr<Expression> Parser::parsePostfix() {
    auto expr = parsePrimary();

    while (true) {
        if (current().getType() == TokenType::LSQUARE) {
            // Array indexing: arr[index]
            advance(); // consume [
            auto index = parseExpression();
            if (current().getType() != TokenType::RSQUARE) {
                throw ParsingException("Expected ']'", current().getLine(), current().getColumn());
            }
            advance(); // consume ]
            expr = std::make_unique<IndexAccessExpression>(std::move(expr), std::move(index));
        } else if (current().getType() == TokenType::DOT) {
            // Member access: obj.member or obj.method(args)
            advance(); // consume .
            if (current().getType() != TokenType::IDENTIFIER) {
                throw ParsingException("Expected member name after '.'", current().getLine(), current().getColumn());
            }
            std::string memberName = current().getValue();
            advance(); // consume member name

            if (current().getType() == TokenType::LPAREN) {
                // Method call: obj.method(args)
                advance(); // consume (
                auto arguments = parseArgumentList();
                if (current().getType() != TokenType::RPAREN) {
                    throw ParsingException("Expected ')' after argument list", current().getLine(), current().getColumn());
                }
                advance(); // consume )
                expr = std::make_unique<MemberAccessExpression>(std::move(expr), memberName, std::move(arguments), true);
            } else {
                // Simple member access: obj.member
                std::vector<std::unique_ptr<Expression>> emptyArgs;
                expr = std::make_unique<MemberAccessExpression>(std::move(expr), memberName, std::move(emptyArgs), false);
            }
        } else {
            break;
        }
    }

    return expr;
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
        case TokenType::ARRAY:
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
        case TokenType::LSQUARE:
            return parseArrayLiteral();
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
        case TokenType::ARRAY: {
            // Handle Array[T] syntax - Array is a keyword
            if (current().getType() != TokenType::LSQUARE) {
                throw ParsingException("Expected '[' after Array", current().getLine(), current().getColumn());
            }
            advance(); // consume [

            Token innerType = current();
            switch (innerType.getType()) {
                case TokenType::INT:
                case TokenType::FLOAT:
                case TokenType::DOUBLE:
                case TokenType::BOOL:
                case TokenType::STR:
                    advance(); // consume simple type
                    break;
                case TokenType::ARRAY: {
                    // Recursive parsing for nested arrays like Array[int]
                    advance(); // consume ARRAY
                    if (current().getType() != TokenType::LSQUARE) {
                        throw ParsingException("Expected '[' after nested Array", current().getLine(), current().getColumn());
                    }
                    advance(); // consume [

                    // Recursively parse the inner type
                    Token nestedType = current();
                    switch (nestedType.getType()) {
                        case TokenType::INT:
                        case TokenType::FLOAT:
                        case TokenType::DOUBLE:
                        case TokenType::BOOL:
                        case TokenType::STR:
                            advance(); // consume nested type
                            break;
                        default:
                            throw ParsingException("Invalid nested array element type", nestedType.getLine(), nestedType.getColumn());
                    }

                    if (current().getType() != TokenType::RSQUARE) {
                        throw ParsingException("Expected ']' after nested array type", current().getLine(), current().getColumn());
                    }
                    advance(); // consume ]
                    break;
                }
                default:
                    throw ParsingException("Invalid array element type", innerType.getLine(), innerType.getColumn());
            }

            if (current().getType() != TokenType::RSQUARE) {
                throw ParsingException("Expected ']' after array type", current().getLine(), current().getColumn());
            }
            advance(); // consume ]
            break;
        }
        case TokenType::IDENTIFIER: {
            if (type.getValue() == "Array") {
                // Parse Array[T] syntax: consume [, type, ]
                if (current().getType() != TokenType::LSQUARE) {
                    throw ParsingException("Expected '[' after Array", current().getLine(), current().getColumn());
                }
                advance(); // consume [

                Token innerType = current();
                switch (innerType.getType()) {
                    case TokenType::INT:
                    case TokenType::FLOAT:
                    case TokenType::DOUBLE:
                    case TokenType::BOOL:
                    case TokenType::STR:
                        advance(); // consume simple type
                        break;
                    case TokenType::ARRAY: {
                        // Recursive parsing for nested arrays like Array[int]
                        advance(); // consume ARRAY
                        if (current().getType() != TokenType::LSQUARE) {
                            throw ParsingException("Expected '[' after nested Array", current().getLine(), current().getColumn());
                        }
                        advance(); // consume [

                        // Recursively parse the inner type
                        Token nestedType = current();
                        switch (nestedType.getType()) {
                            case TokenType::INT:
                            case TokenType::FLOAT:
                            case TokenType::DOUBLE:
                            case TokenType::BOOL:
                            case TokenType::STR:
                                advance(); // consume nested type
                                break;
                            default:
                                throw ParsingException("Invalid nested array element type", nestedType.getLine(), nestedType.getColumn());
                        }

                        if (current().getType() != TokenType::RSQUARE) {
                            throw ParsingException("Expected ']' after nested array type", current().getLine(), current().getColumn());
                        }
                        advance(); // consume ]
                        break;
                    }
                    default:
                        throw ParsingException("Invalid array element type", innerType.getLine(), innerType.getColumn());
                }

                if (current().getType() != TokenType::RSQUARE) {
                    throw ParsingException("Expected ']' after array type", current().getLine(), current().getColumn());
                }
                advance(); // consume ]
            } else {
                throw ParsingException("Unknown type: " + type.getValue(), type.getLine(), type.getColumn());
            }
            break;
        }
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

std::unique_ptr<Statement> Parser::parseIndexAssignmentOrExpression() {
    // We know current token is IDENTIFIER and next is LSQUARE
    // Parse the full lvalue expression first
    auto lvalueExpr = parseExpression();

    // Check if this is followed by an assignment
    if (current().getType() == TokenType::ASSIGN) {
        advance(); // consume =
        auto rvalueExpr = parseExpression();
        return std::make_unique<IndexAssignment>(std::move(lvalueExpr), std::move(rvalueExpr));
    } else {
        // This is just an expression statement
        return std::make_unique<ExpressionStatement>(std::move(lvalueExpr));
    }
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
                case TokenType::ARRAY: {
                    // Parse Array[ElementType] syntax
                    if (advance().getType() != TokenType::LSQUARE) {
                        throw ParsingException("Expected '[' after Array", current().getLine(), current().getColumn());
                    }
                    Token innerType = advance();
                    switch (innerType.getType()) {
                        case TokenType::INT:
                        case TokenType::FLOAT:
                        case TokenType::BOOL:
                        case TokenType::STR:
                        case TokenType::DOUBLE:
                            // Simple type, don't advance again
                            break;
                        case TokenType::ARRAY: {
                            // Recursive parsing for nested arrays in function parameters
                            if (advance().getType() != TokenType::LSQUARE) {
                                throw ParsingException("Expected '[' after nested Array", current().getLine(), current().getColumn());
                            }
                            Token nestedType = advance();
                            switch (nestedType.getType()) {
                                case TokenType::INT:
                                case TokenType::FLOAT:
                                case TokenType::BOOL:
                                case TokenType::STR:
                                case TokenType::DOUBLE:
                                    // Don't advance again
                                    break;
                                default:
                                    throw ParsingException("Invalid nested array element type in function parameter", nestedType.getLine(), nestedType.getColumn());
                            }
                            if (advance().getType() != TokenType::RSQUARE) {
                                throw ParsingException("Expected ']' after nested array type in function parameter", current().getLine(), current().getColumn());
                            }
                            break;
                        }
                        default:
                            throw ParsingException("Expected element type in Array", innerType.getLine(), innerType.getColumn());
                    }
                    if (current().getType() != TokenType::RSQUARE) {
                        throw ParsingException("Expected ']' after Array element type", current().getLine(), current().getColumn());
                    }
                    advance(); // consume ]
                    break;
                }
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
        case TokenType::ARRAY: {
            // Parse Array[ElementType] syntax for return type
            if (advance().getType() != TokenType::LSQUARE) {
                throw ParsingException("Expected '[' after Array", current().getLine(), current().getColumn());
            }
            Token innerType = advance();
            switch (innerType.getType()) {
                case TokenType::INT:
                case TokenType::FLOAT:
                case TokenType::BOOL:
                case TokenType::STR:
                case TokenType::DOUBLE:
                    // Simple type, don't advance again
                    break;
                case TokenType::ARRAY: {
                    // Recursive parsing for nested arrays in return type
                    if (advance().getType() != TokenType::LSQUARE) {
                        throw ParsingException("Expected '[' after nested Array", current().getLine(), current().getColumn());
                    }
                    Token nestedType = advance();
                    switch (nestedType.getType()) {
                        case TokenType::INT:
                        case TokenType::FLOAT:
                        case TokenType::BOOL:
                        case TokenType::STR:
                        case TokenType::DOUBLE:
                            // Don't advance again
                            break;
                        default:
                            throw ParsingException("Invalid nested array element type in return type", nestedType.getLine(), nestedType.getColumn());
                    }
                    if (advance().getType() != TokenType::RSQUARE) {
                        throw ParsingException("Expected ']' after nested array type in return type", current().getLine(), current().getColumn());
                    }
                    break;
                }
                default:
                    throw ParsingException("Expected element type in Array", innerType.getLine(), innerType.getColumn());
            }
            if (current().getType() != TokenType::RSQUARE) {
                throw ParsingException("Expected ']' after Array element type", current().getLine(), current().getColumn());
            }
            advance(); // consume ]
            break;
        }
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

std::unique_ptr<Expression> Parser::parseArrayLiteral() {
    advance(); // consume [

    std::vector<std::unique_ptr<Expression>> elements;

    if (current().getType() != TokenType::RSQUARE) {
        while (true) {
            elements.push_back(parseExpression());

            if (current().getType() == TokenType::COMMA) {
                advance();
                continue;
            } else if (current().getType() == TokenType::RSQUARE) {
                break;
            } else {
                throw ParsingException("Expected ',' or ']' in array literal", current().getLine(), current().getColumn());
            }
        }
    }

    advance(); // consume ]
    return std::make_unique<ArrayLiteralExpression>(std::move(elements));
}

