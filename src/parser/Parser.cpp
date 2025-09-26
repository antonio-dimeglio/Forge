#include "../../include/parser/Parser.hpp"
#include "../../include/parser/ParserException.hpp"
#include "../../include/parser/ParsedType.hpp"
#include <iostream>

Token Parser::expect(TokenType expectedType, const std::string& errorMessage) {
    if (current().getType() != expectedType) {
        throw ParsingException(errorMessage, current().getLine(), current().getColumn());
    }
    return advance();
}

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

bool Parser::isValidTypeToken(TokenType type) {
    switch (type) {
        case TokenType::INT:
        case TokenType::FLOAT:
        case TokenType::DOUBLE:
        case TokenType::BOOL:
        case TokenType::STR:
        case TokenType::VOID:
        case TokenType::IDENTIFIER:
        case TokenType::UNIQUE:
        case TokenType::SHARED:
        case TokenType::WEAK:
            return true;
        default:
            return false;
    }
}

std::optional<ParsedType> Parser::parseType() {
    // Handle smart pointer keywords first: unique, shared, weak
    Token token = current();

    SmartPointerType smartPtrType = SmartPointerType::None;
    bool isOptional = false;

    // First check if the type is an optional one, then check its content
    if (token.getType() == TokenType::MAYBE) {
        isOptional = true;
        advance();  // consume MAYBE
        expect(TokenType::LESS, "Expected <");
        token = current();  // Update token to the inner type
    }

    switch (token.getType()) {
        case TokenType::UNIQUE: 
            smartPtrType = SmartPointerType::Unique;
            advance();
            break;
        case TokenType::SHARED:
            smartPtrType = SmartPointerType::Shared;
            advance();
            break;
        case TokenType::WEAK:
            smartPtrType = SmartPointerType::Weak;
            advance();
            break;
    }

    // Handle recursive pointer/reference operators: **, ***, &*, etc.
    int pointerNesting = 0;
    bool isPtr = false, isRef = false, isMutref = false;

    // Keep consuming pointer/reference operators until we find a type
    while (current().getType() == TokenType::MULT || current().getType() == TokenType::BITWISE_AND) {
        if (current().getType() == TokenType::MULT) {
            pointerNesting++;
            advance();
            isPtr = true;
        } else if (current().getType() == TokenType::BITWISE_AND) {
            advance();
            if (current().getType() == TokenType::IDENTIFIER &&
                current().getValue() == "mut") {
                isMutref = true;
                advance();
            } else {
                isRef = true;
            }
        }
    }

    // Now we should have a valid type token
    if (!isValidTypeToken(current().getType())) {
        return std::nullopt;
    }

    Token baseType = advance();

    // Keep the base type clean - don't mix pointer prefixes with the type name
    Token primaryType = baseType;

    ParsedType result {
        .primaryType = primaryType,
        .typeParameters = {},
        .nestingLevel = pointerNesting,  // Store the pointer nesting level
        .isPointer = isPtr,
        .isReference = isRef,
        .isMutReference = isMutref,
        .isOptional = isOptional,
        .smartPointerType = smartPtrType
    };
    
    // Handle Array[T], Map[K,V], etc.
    if (current().getType() == TokenType::LSQUARE) {
        advance();

        auto innerTypeResult = parseType();
        if (!innerTypeResult.has_value()) {
            return std::nullopt;
        }

        result.typeParameters.push_back(innerTypeResult->primaryType);
        result.nestingLevel = innerTypeResult->nestingLevel + 1;
        while (current().getType() == TokenType::COMMA) {
            advance(); // consume ','
            auto paramResult = parseType();
            if (!paramResult.has_value()) {
                return std::nullopt;
            }
            result.typeParameters.push_back(paramResult->primaryType);
        }

        expect(TokenType::RSQUARE, "Expected ]");
    }

    if (isOptional) expect(TokenType::GREATER, "Expected >");

    return result;
}

std::unique_ptr<Statement> Parser::parseStatement() {
    skipNewLines();

    if (isAtEnd() || current().getType() == TokenType::END_OF_FILE) {
        return nullptr;
    }

    TokenType currentType = current().getType();

    switch (currentType) {
        case TokenType::IF: return parseIfStatement();
        case TokenType::WHILE: return parseWhileStatement();
        case TokenType::DEF: return parseFunctionDefinition();
        case TokenType::RETURN: return parseReturnStatement();
        case TokenType::DEFER: return parseDeferStatement();
        case TokenType::EXTERN: return parseExternStatement();
        case TokenType::LBRACE: return std::unique_ptr<Statement>(parseBlockStatement().release());
        case TokenType::CLASS: return parseClassDefinition();
        case TokenType::IDENTIFIER: {
            TokenType nextType = peek().getType();
            if (nextType == TokenType::COLON) {
                return parseVariableDeclaration();
            }
            // For all other cases with IDENTIFIER, use unified assignment parsing
            auto stmt = parseAssignmentOrExpression();

            // Check if there are leftover tokens on the same line
            if (stmt != nullptr && !isAtEnd() && current().getType() != TokenType::END_OF_FILE &&
                current().getType() != TokenType::NEWLINE) {
                auto remainingToken = current();
                throw ParsingException(
                    "Unexpected token '" + remainingToken.getValue() + "' after statement",
                    remainingToken.getLine(),
                    remainingToken.getColumn()
                );
            }

            return stmt;
        }
        default:
            // For expressions that could be assignments (*ptr = value, arr[i] = value, etc.)
            auto stmt = parseAssignmentOrExpression();

            // Check if there are leftover tokens on the same line
            // (This catches errors like "x 42" where x is valid but 42 is leftover)
            if (stmt != nullptr && !isAtEnd() && current().getType() != TokenType::END_OF_FILE &&
                current().getType() != TokenType::NEWLINE) {
                auto remainingToken = current();
                throw ParsingException(
                    "Unexpected token '" + remainingToken.getValue() + "' after statement",
                    remainingToken.getLine(),
                    remainingToken.getColumn()
                );
            }

            return stmt;
    }
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
        case TokenType::MINUS:
        case TokenType::BITWISE_AND:
        case TokenType::MULT: {
            advance();
            auto operand = parseUnary();
            return std::make_unique<UnaryExpression>(token, std::move(operand));
        }
        case TokenType::MOVE: {
            advance();
            auto operand = parseUnary();
            return std::make_unique<MoveExpression>(token, std::move(operand));
        }
        default:
            return parsePostfix();
    }
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
            
            expect(TokenType::RSQUARE, "Expected ']'");
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
                expect(TokenType::RPAREN, "Expected ')' after argument list");
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
    switch (current().getType()) {
        case TokenType::NUMBER:
        case TokenType::STRING:
        case TokenType::TRUE:
        case TokenType::FALSE:
        case TokenType::NULL_:
            return parseLiteral();
        case TokenType::IDENTIFIER:
            return parseIdentifierExpression();
        case TokenType::LPAREN:
            return parseParenthesizedExpression();
        case TokenType::LSQUARE:
            return parseArrayLiteral();
        case TokenType::NEW:
            return parseNewExpression();
        case TokenType::SOME:
        case TokenType::NONE:
            return parseOptional();
        default:
            throw ParsingException("Unexpected token in expression", current().getLine(), current().getColumn());
    }
}

std::unique_ptr<Statement> Parser::parseVariableDeclaration() {
    Token identifier = advance();

    if (identifier.getType() != TokenType::IDENTIFIER){
        throw ParsingException("Expected identifier", identifier.getLine(), identifier.getColumn());
    }

    expect(TokenType::COLON, "Expected :");

    auto typeResult = parseType();
    if (!typeResult.has_value()) {
        throw ParsingException("Expected valid type", current().getLine(), current().getColumn());
    }
    
    ParsedType parsedType = typeResult.value();

    // Accept both = and := for variable declarations with explicit types
    if (current().getType() == TokenType::ASSIGN || current().getType() == TokenType::INFER_ASSIGN) {
        advance();
    } else {
        throw ParsingException("Expected '=' or ':=' after type", current().getLine(), current().getColumn());
    }

    auto expression = parseExpression();

    return std::make_unique<VariableDeclaration>(
        identifier,
        parsedType,
        std::move(expression)
    );
}

std::unique_ptr<Statement> Parser::parseAssignment() {
    Token identifier = advance();
    Token t(TokenType::END_OF_FILE, -1, -1);

    if (identifier.getType() != TokenType::IDENTIFIER) {
        throw ParsingException("Expected identifier", identifier.getLine(), identifier.getColumn());
    }

    expect(TokenType::ASSIGN, "Expected =");

    auto rvalue = parseExpression();

    // Create identifier expression for LHS
    auto lvalue = std::make_unique<IdentifierExpression>(identifier.getValue());

    return std::make_unique<Assignment>(
        std::move(lvalue),
        std::move(rvalue)
    );
}

std::unique_ptr<Statement> Parser::parseAssignmentOrExpression() {
    // Always parse LHS as expression first
    auto lhs = parseExpression();

    // Check if next token is assignment operator
    if (current().getType() == TokenType::ASSIGN) {
        advance(); // consume '='
        auto rhs = parseExpression();
        return std::make_unique<Assignment>(std::move(lhs), std::move(rhs));
    }
    else if (current().getType() == TokenType::INFER_ASSIGN) {
        // Handle type inference assignment (identifier := expression)
        advance(); // consume ':='

        // LHS must be a simple identifier for type inference
        auto identifierExpr = dynamic_cast<IdentifierExpression*>(lhs.get());
        if (!identifierExpr) {
            throw ParsingException("Type inference assignment requires simple identifier on left side", current().getLine(), current().getColumn());
        }

        auto rhs = parseExpression();

        // Create VariableDeclaration with auto type
        Token identifierToken(TokenType::IDENTIFIER, identifierExpr->name, current().getLine(), current().getColumn());
        Token autoType(TokenType::IDENTIFIER, "auto", current().getLine(), current().getColumn());
        ParsedType parsedAutoType {
            .primaryType = autoType,
            .typeParameters = {},
            .nestingLevel = 0,
            .isPointer = false,
            .isReference = false,
            .isMutReference = false,
            .isOptional = false,
            .smartPointerType = SmartPointerType::None
        };

        return std::make_unique<VariableDeclaration>(identifierToken, parsedAutoType, std::move(rhs));
    }
    else {
        // Not an assignment, return as expression statement
        return std::make_unique<ExpressionStatement>(std::move(lhs));
    }
}

std::unique_ptr<Statement> Parser::parseInferredDeclaration() {
    Token identifier = advance();

    if (identifier.getType() != TokenType::IDENTIFIER) {
        throw ParsingException("Expected identifier", identifier.getLine(), identifier.getColumn());
    }

    expect(TokenType::INFER_ASSIGN, "Expected :=");

    auto expression = parseExpression();

    // For now, we'll create a special VariableDeclaration with inferred type
    // The type will be inferred from the expression during semantic analysis
    Token inferredType = Token(TokenType::IDENTIFIER, "auto", identifier.getLine(), identifier.getColumn());
    ParsedType parsedInferredType {
        .primaryType = inferredType,
        .typeParameters = {},
        .nestingLevel = 0,
        .isPointer = false,
        .isReference = false,
        .isMutReference = false,
        .isOptional = false,
        .smartPointerType = SmartPointerType::None
    };

    return std::make_unique<VariableDeclaration>(
        identifier,
        parsedInferredType,  // Placeholder - actual type inference happens later
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

    expect(TokenType::LBRACE, "Expected {");

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

    expect(TokenType::RBRACE, "Expected }");

    return std::make_unique<BlockStatement>(std::move(statements));
}

std::unique_ptr<Statement> Parser::parseIfStatement() {
    Token t(TokenType::END_OF_FILE, -1, -1);

    // if ((t = advance()).getType() != TokenType::IF) {
    //     throw ParsingException("Expected if", t.getLine(), t.getColumn());
    // }
    expect(TokenType::IF, "Expected if");
    expect(TokenType::LPAREN, "Expected (");

    auto expression = parseExpression();

    expect(TokenType::RPAREN, "Expected )");

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

    expect(TokenType::WHILE, "Expected while");
    expect(TokenType::LPAREN, "Expected (");

    auto expression = parseExpression();

    expect(TokenType::RPAREN, "Expected )");

    auto body = parseBlockStatement();

    return std::make_unique<WhileStatement>(
        std::move(expression),
        std::move(body)
    );
}

std::unique_ptr<Statement> Parser::parseReturnStatement() {
    Token t(TokenType::END_OF_FILE, -1, -1);

    expect(TokenType::RETURN, "Expected return");

    auto value = parseExpression();

    return std::make_unique<ReturnStatement>(
        std::move(value)
    );
}

std::unique_ptr<Statement> Parser::parseFunctionDefinition() {
    expect(TokenType::DEF, "Expected def");

    Token functionName = expect(TokenType::IDENTIFIER, "Expected function name");

    // Parse optional type parameters: def func<T, U>
    std::vector<Token> typeParameters;
    if (current().getType() == TokenType::LESS) {
        advance(); // consume '<'

        if (current().getType() != TokenType::GREATER) {
            while (true) {
                Token typeParam = expect(TokenType::IDENTIFIER, "Expected type parameter name");
                typeParameters.push_back(typeParam);

                if (current().getType() == TokenType::COMMA) {
                    advance(); // consume ','
                    continue;
                } else if (current().getType() == TokenType::GREATER) {
                    break;
                } else {
                    throw ParsingException("Expected ',' or '>' in function type parameters", current().getLine(), current().getColumn());
                }
            }
        }

        expect(TokenType::GREATER, "Expected '>' after function type parameters");
    }

    expect(TokenType::LPAREN, "Expected (");

    std::vector<StatementParameter> params;
    
    if (current().getType() != TokenType::RPAREN) {
        while (true) {
            Token name = expect(TokenType::IDENTIFIER, "Expected parameter name");

            expect(TokenType::COLON, "Expected :");

            auto typeResult = parseType();
            if (!typeResult.has_value()) {
                throw ParsingException("Expected valid parameter type", current().getLine(), current().getColumn());
            }

            params.push_back(StatementParameter {
                .name = name,
                .type = *typeResult
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

    expect(TokenType::RPAREN, "Expected )");
    expect(TokenType::ARROW, "Expected ->");

    auto returnTypeResult = parseType();
    if(!returnTypeResult.has_value()) {
        throw ParsingException("Expected valid return type", current().getLine(), current().getColumn());
    }

    auto body = parseBlockStatement();

    return std::make_unique<FunctionDefinition>(
            functionName,
            returnTypeResult->primaryType,
            typeParameters,
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

std::unique_ptr<Expression> Parser::parseGenericInstantiation(Token className) {
    // Parse generic type arguments: [int] or [string, int]
    advance(); // consume '['

    std::vector<Token> typeArguments;

    if (current().getType() != TokenType::RSQUARE) {
        while (true) {
            if (!isValidTypeToken(current().getType())) {
                throw ParsingException("Expected type in generic instantiation", current().getLine(), current().getColumn());
            }

            Token typeArg = current();
            advance();
            typeArguments.push_back(typeArg);

            if (current().getType() == TokenType::COMMA) {
                advance(); // consume ','
                continue;
            } else if (current().getType() == TokenType::RSQUARE) {
                break;
            } else {
                throw ParsingException("Expected ',' or ']' in generic type arguments", current().getLine(), current().getColumn());
            }
        }
    }

    advance(); // consume ']'

    // Parse constructor arguments: (10) or ("Alice", 25)
    expect(TokenType::LPAREN, "Expected '(' after generic instantiation");

    std::vector<std::unique_ptr<Expression>> arguments;

    if (current().getType() != TokenType::RPAREN) {
        while (true) {
            arguments.push_back(parseExpression());

            if (current().getType() == TokenType::COMMA) {
                advance(); // consume ','
                continue;
            } else if (current().getType() == TokenType::RPAREN) {
                break;
            } else {
                throw ParsingException("Expected ',' or ')' in constructor arguments", current().getLine(), current().getColumn());
            }
        }
    }

    advance(); // consume ')'

    return std::make_unique<GenericInstantiation>(className, std::move(typeArguments), std::move(arguments));
}

std::unique_ptr<Statement> Parser::parseClassDefinition() {
    advance(); // consume Class
    Token className = advance();
    std::vector<Token> genericParameters;
    std::vector<FieldDefinition> fields;
    std::vector<MethodDefinition> methods;

    if (className.getType() != TokenType::IDENTIFIER) {
        throw ParsingException("Cannot use name for class definition", className.getLine(), className.getColumn());
    }

    if (current().getType() == TokenType::LSQUARE) {
        advance();
        // Cannot have empty squares like Array[] without a T
        if (current().getType() == TokenType::RSQUARE) {
            throw ParsingException("Cannot have empty generic class definition", current().getLine(), current().getColumn());
        }
        while (true) {
            Token name = current();
            if (name.getType() != TokenType::IDENTIFIER) {
                throw ParsingException("Expected generic identifier", name.getLine(), name.getColumn());
            }
            genericParameters.push_back(name);
            advance(); // consume the identifier

            // Check what comes after the identifier
            if (current().getType() == TokenType::RSQUARE) {
                advance(); // consume the ']'
                break;
            } else if (current().getType() == TokenType::COMMA) {
                advance(); // consume the comma
                continue;
            } else {
                throw ParsingException("Expected ',' or ']' after generic parameter", current().getLine(), current().getColumn());
            }
        }
    }

    expect(TokenType::LBRACE, "Expected '{' after class declaration");

    while (current().getType() != TokenType::RBRACE) {
        if (current().getType() == TokenType::IDENTIFIER) {
            fields.push_back(parseFieldDefinition());
        } else if (current().getType() == TokenType::DEF) {
            methods.push_back(parseMethodDefinition()); // Must be defined
        } else {
            throw ParsingException("Expected method or attribute", current().getLine(), current().getColumn());
        }
    }
    expect(TokenType::RBRACE, "Expected '}' to close class definition");

    return std::make_unique<ClassDefinition>(className, std::move(genericParameters), std::move(fields), std::move(methods));
}

FieldDefinition Parser::parseFieldDefinition() {
    Token name = expect(TokenType::IDENTIFIER, "Expected identifier for field definition.");
    expect(TokenType::COLON, "Expected :");

    if (!isValidTypeToken(current().getType())) {
        throw ParsingException("Expected valid type", current().getLine(), current().getColumn());
    }

    Token type = advance();

    return FieldDefinition {
        .name = name,
        .type = type
    };
}

MethodDefinition Parser::parseMethodDefinition() {
    std::vector<StatementParameter> parameters;
    std::unique_ptr<BlockStatement> body;

    expect(TokenType::DEF, "Expected def");
    Token methodName = expect(TokenType::IDENTIFIER, "Expected identifier for method definition.");
    expect(TokenType::LPAREN, "Expected (");

    if (current().getType() != TokenType::RPAREN) {
        while (true) {
            Token name = expect(TokenType::IDENTIFIER, "Expected parameter name");
            if (name.getType() != TokenType::IDENTIFIER) {
                throw ParsingException("Expected parameter name", name.getLine(), name.getColumn());
            }

            expect(TokenType::COLON, "Expected :");

            auto typeResult = parseType();
            if (!typeResult.has_value()) {
                throw ParsingException("Expected valid parameter type", current().getLine(), current().getColumn());
            }

            parameters.push_back(StatementParameter {
                .name = name,
                .type = *typeResult
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

    expect(TokenType::ARROW, "Expected ->");
    Token returnType = advance();

    if (!isValidTypeToken(returnType.getType())) {
        throw ParsingException("Invalid return type for method " + methodName.getValue(), current().getLine(), current().getColumn());
    }

    return MethodDefinition(
        methodName,
        returnType,
        std::move(parameters),
        parseBlockStatement() 
    );
}

std::unique_ptr<Statement> Parser::parseDeferStatement() {
    expect(TokenType::DEFER, "Expected defer");

    auto expression = parseExpression();

    return std::make_unique<DeferStatement>(std::move(expression));
}

std::unique_ptr<Statement> Parser::parseExternStatement() {
    expect(TokenType::EXTERN, "Expected extern");
    expect(TokenType::DEF, "Expected def");

    Token functionName = expect(TokenType::IDENTIFIER, "Expected function name");
    expect(TokenType::LPAREN, "Expected (");

    std::vector<StatementParameter> params;

    if (current().getType() != TokenType::RPAREN) {
        while (true) {
            Token name = expect(TokenType::IDENTIFIER, "Expected parameter name");
            expect(TokenType::COLON, "Expected :");

            auto typeResult = parseType();
            if (!typeResult.has_value()) {
                throw ParsingException("Expected valid parameter type", current().getLine(), current().getColumn());
            }

            params.push_back(StatementParameter {
                .name = name,
                .type = *typeResult
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

    expect(TokenType::RPAREN, "Expected )");
    expect(TokenType::ARROW, "Expected ->");

    auto returnTypeResult = parseType();
    if (!returnTypeResult.has_value()) {
        throw ParsingException("Expected valid return type", current().getLine(), current().getColumn());
    }

    return std::make_unique<ExternStatement>(functionName, std::move(params), *returnTypeResult);
}

std::unique_ptr<Expression> Parser::parseLiteral() {
    Token token = current();
    advance();
    return std::make_unique<LiteralExpression>(token);
}

std::unique_ptr<Expression> Parser::parseIdentifierExpression() {
    Token identifier = advance();

    if (current().getType() == TokenType::LSQUARE) {
        // Need to distinguish between generic instantiation (Array[int](10))
        // and array access (arr[0]) by looking ahead for parentheses
        size_t savedIdx = idx;

        // Skip past the bracket content to see if there are parentheses after
        advance(); // consume '['
        int bracketDepth = 1;
        while (bracketDepth > 0 && !isAtEnd()) {
            if (current().getType() == TokenType::LSQUARE) {
                bracketDepth++;
            } else if (current().getType() == TokenType::RSQUARE) {
                bracketDepth--;
            }
            if (bracketDepth > 0) advance();
        }

        bool hasParensAfter = false;
        if (!isAtEnd() && current().getType() == TokenType::RSQUARE) {
            advance(); // consume ']'
            if (!isAtEnd() && current().getType() == TokenType::LPAREN) {
                hasParensAfter = true;
            }
        }

        // Restore position
        idx = savedIdx;

        if (hasParensAfter) {
            // This is generic instantiation: Array[int](10)
            return parseGenericInstantiation(identifier);
        } else {
            // This is array access, return identifier and let postfix handle it
            return std::make_unique<IdentifierExpression>(identifier.getValue());
        }
    } else if (current().getType() == TokenType::LPAREN) {
        // Regular function call: func(args)
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

        expect(TokenType::RPAREN, "Expected ')'");

        return std::make_unique<FunctionCall>(identifier.getValue(), std::vector<Token>{}, std::move(arguments));
    } else {
        // Just a variable: identifier
        return std::make_unique<IdentifierExpression>(identifier.getValue());
    }
}

std::unique_ptr<Expression> Parser::parseParenthesizedExpression() {
    advance();
    auto expr = parseExpression(); 
    expect(TokenType::RPAREN, "Expected ')'");
    return expr;
}

std::unique_ptr<Expression> Parser::parseNewExpression() {
    advance(); // consume 'new'

    // Check if this is a class instantiation (new ClassName(...)) or value instantiation (new value)
    if (current().getType() == TokenType::IDENTIFIER && peek().getType() == TokenType::LPAREN) {
        // Class instantiation: new ClassName(args)
        Token className = expect(TokenType::IDENTIFIER, "Expected class name after 'new'");
        expect(TokenType::LPAREN, "Expected '(' after class name");

        std::vector<std::unique_ptr<Expression>> arguments;
        if (current().getType() != TokenType::RPAREN) {
            while (true) {
                arguments.push_back(parseExpression());

                if (current().getType() == TokenType::COMMA) {
                    advance();
                    continue;
                } else if (current().getType() == TokenType::RPAREN) {
                    break;
                } else {
                    throw ParsingException("Expected ',' or ')' in constructor arguments", current().getLine(), current().getColumn());
                }
            }
        }

        expect(TokenType::RPAREN, "Expected ')' after constructor arguments");
        return std::make_unique<ObjectInstantiation>(className, std::move(arguments));
    } else {
        // Value instantiation: new value
        auto valueExpression = parseExpression();
        return std::make_unique<NewExpression>(std::move(valueExpression));
    }
}

std::unique_ptr<Expression> Parser::parseOptional() {
    Token token = current();
    advance(); // Consume the SOME or NONE token
    if (token.getType() == TokenType::SOME) {
        return std::make_unique<OptionalExpression>(token, parseExpression());
    } else {
        return std::make_unique<OptionalExpression>(token, nullptr);
    }
}