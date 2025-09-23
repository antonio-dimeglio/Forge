#include "../../include/lexer/Tokenizer.hpp"
#include "../../include/lexer/LexerException.hpp"
#include <cctype>
#include <unordered_map>

static const std::unordered_map<std::string, TokenType> keywords = {
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"while", TokenType::WHILE},
    {"for", TokenType::FOR},
    {"def", TokenType::DEF},
    {"return", TokenType::RETURN},
    {"int", TokenType::INT},
    {"str", TokenType::STR},
    {"bool", TokenType::BOOL},
    {"float", TokenType::FLOAT},
    {"double", TokenType::DOUBLE},
    {"Array", TokenType::ARRAY},
    {"true", TokenType::TRUE},
    {"false", TokenType::FALSE}
};

char Tokenizer::current() {
    if (idx >= source.length()) {
        return '\0';
    } 

    return source[idx];
}

char Tokenizer::peek(int offset) {
    size_t actual = idx + offset;

    if (actual >= source.length()) {
        return '\0';
    }

    return source[actual];
}

void Tokenizer::advance() {
    if (idx >= source.length()){
        return; 
    }

    if (source[idx] == '\n') {
        line += 1;
        column = 1;
    } else {
        column += 1;
    }


    idx++;
}

bool Tokenizer::isAtEnd() {
    return idx == source.length();
}

bool Tokenizer::isDigit(char c) {
    return isdigit(c);
}

bool Tokenizer::isAlpha(char c) {
    return isalpha(c);
}

bool Tokenizer::isAlphaNumeric(char c) {
    return isalnum(c);
}

bool Tokenizer::isWhitespace(char c) {
    return isspace(c);
}

TokenType Tokenizer::getKeywordType(const std::string& text) {
    auto it = keywords.find(text);
    if (it != keywords.end()) {
        return it->second;  
    }
    return TokenType::IDENTIFIER;
} 

Token Tokenizer::scanNumber() {
    bool foundDot = false; 
    char ch;
    std::string value = "";
    auto start_column = column;
    while (!isAtEnd() && 
        (isDigit(ch = current()) || ch == '.')) {
        if (ch == '.') {
            if (foundDot){
                throw InvalidSyntaxException(
                    "Found invalid numerical value", line, column);
            }
            else {
                foundDot = true; 
            }
        }
        value.push_back(ch);
        advance();
    }

    if (!isAtEnd() && current() == 'f') {
        value.push_back('f');
        advance();
    }

    return Token(TokenType::NUMBER, value, line, start_column);
}

Token Tokenizer::scanIdentifier() {
    // Theoretically this should be unreachable, because if the first 
    // character of an identifieris a digit/. it will fall in the 
    // scanNumber function 
    if (!isAlpha(current()) && current() != '_') {
        throw InvalidSyntaxException(
            "Found identifier starting with non alphabetical or underscore character",
            line,
            column
        );
    }
    char ch;
    std::string value; 
    auto start_column = column;
    while(!isAtEnd() && (isAlphaNumeric(ch = current()) || ch == '_')) {
        value.push_back(ch);
        advance();
    }

    TokenType type = getKeywordType(value);
    return Token(type, value, line, start_column);
}

Token Tokenizer::scanString(char startingChar) {
    // I am not exactly sure how to handle newlines for strings, for now
    // I will allow the user to just continue a string on a new line

    std::string value;
    advance();
    auto start_column = column;
    char ch;
    while(!isAtEnd() && (ch = current()) != startingChar) {
        value.push_back(ch);
        advance();
    }

    if (isAtEnd()) {
        throw InvalidSyntaxException("Unterminated string", line, column);
    }

    advance();
    return Token(TokenType::STRING, value, line, start_column);
}

void Tokenizer::skipWhitespace() {
    while (!isAtEnd() && isWhitespace(current()) && current() != '\n'){
        advance();
    }
}

void Tokenizer::errorToken(const std::string& message) {
    throw InvalidSyntaxException(message, line, column);
}

std::vector<Token> Tokenizer::tokenize() {
    std::vector<Token> tokens; 

    while(!isAtEnd()) {
        char ch = current(); 
        if (ch == '.' && !isDigit(peek())) {
            tokens.push_back(Token(TokenType::DOT, ".", line, column));
            advance();
        } else if (isDigit(ch) || ch == '.') {
            tokens.push_back(scanNumber());
        } else if (isAlpha(ch) || ch == '_') {
            tokens.push_back(scanIdentifier());
        } else if (ch == '\'' || ch == '"') {
            tokens.push_back(scanString(ch));
        } else if (ch == '\n') {
            tokens.push_back(Token(TokenType::NEWLINE, line, column));
            advance();
        } else if (isWhitespace(ch)) {
            skipWhitespace();
        } else {
            switch (ch) {
                case '+':
                    if (peek() == '=') {
                        tokens.push_back(Token(TokenType::PLUS_EQ, "+=", line, column));
                        advance();
                    } else {
                        tokens.push_back(Token(TokenType::PLUS, "+", line, column));
                    }
                    break;
                case '-':
                    if (peek() == '>') {
                        tokens.push_back(Token(TokenType::ARROW, "->", line, column));
                        advance();
                    } else if (peek() == '=') {
                        tokens.push_back(Token(TokenType::MINUS_EQ, "-=", line, column));
                        advance();
                    } else {
                        tokens.push_back(Token(TokenType::MINUS, "-", line, column));
                    }
                    break;
                case '*':
                    if (peek() == '=') {
                        tokens.push_back(Token(TokenType::MULT_EQ, "*=", line, column));
                        advance();
                    } else {
                        tokens.push_back(Token(TokenType::MULT, "*", line, column));
                    }
                    break;
                case '/':
                    if (peek() == '=') {
                        tokens.push_back(Token(TokenType::DIV_EQ, "/=", line, column));
                        advance();
                    }   else if (peek() == '/') {
                            while(current() != '\n' && !isAtEnd()) {
                                advance();
                            }                           
                    } 
                    else {
                        tokens.push_back(Token(TokenType::DIV, "/", line, column));
                    }
                    break;
                case '=':
                    if (peek() == '=') {
                        tokens.push_back(Token(TokenType::EQUAL_EQUAL, "==", line, column));
                        advance();
                    } else {
                        tokens.push_back(Token(TokenType::ASSIGN, "=", line, column));
                    }
                    break;
                case '!':
                    if (peek() == '=') {
                        tokens.push_back(Token(TokenType::NOT_EQUAL, "!=", line, column));
                        advance();
                    } else {
                        tokens.push_back(Token(TokenType::NOT, "!", line, column));
                    }
                    break;
                case '[':
                    tokens.push_back(Token(TokenType::LSQUARE, "[", line, column));
                    break;
                case ']':
                    tokens.push_back(Token(TokenType::RSQUARE, "]", line, column));
                    break;
                case '(':
                    tokens.push_back(Token(TokenType::LPAREN, "(", line, column));
                    break; 
                case ')':
                    tokens.push_back(Token(TokenType::RPAREN, ")", line, column));
                    break; 
                case '{':
                    tokens.push_back(Token(TokenType::LBRACE, "{", line, column));
                    break; 
                case '}':
                    tokens.push_back(Token(TokenType::RBRACE, "}", line, column));
                    break; 
                case ',':
                    tokens.push_back(Token(TokenType::COMMA, ",", line, column));
                    break;
                case ':':
                    tokens.push_back(Token(TokenType::COLON, ":", line, column));
                    break;
                case '>':
                    if (peek() == '=') {
                        tokens.push_back(Token(TokenType::GEQ, ">=", line, column));
                        advance();
                    } else {
                        tokens.push_back(Token(TokenType::GREATER, ">", line, column));
                    }
                    break;
                case '<':
                    if (peek() == '=') {
                        tokens.push_back(Token(TokenType::LEQ, "<=", line, column));
                        advance();
                    } else {
                        tokens.push_back(Token(TokenType::LESS, "<", line, column));
                    }
                    break;
                case '&': 
                    if (peek() == '&') {
                        tokens.push_back(Token(TokenType::LOGIC_AND, "&&", line, column));
                        advance();
                    } else {
                        tokens.push_back(Token(TokenType::BITWISE_AND, "&", line, column));
                    }
                    break;
                case '|':
                    if (peek() == '|') {
                        tokens.push_back(Token(TokenType::LOGIC_OR, "||", line, column));
                        advance();
                    } else {
                        tokens.push_back(Token(TokenType::BITWISE_OR, "|", line, column));
                    }
                    break;
                case '^':
                    tokens.push_back(Token(TokenType::BITWISE_XOR, "^", line, column));
                    break;
                default:
                    throw InvalidSyntaxException("Found invalid character", line, column);
            }
            advance();
            
        }
    }
    tokens.push_back(Token(TokenType::END_OF_FILE, line, column));
    return tokens;
}

void Tokenizer::reset(const std::string& source) {
    this->source = source; 
    idx = 0;
    line = 1;
    column = 1;
} 