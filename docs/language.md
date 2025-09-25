# Forge

Forge is a programming language that was born mostly as a way of teaching myself, for fun, how programming languages work under the hood by trying to implement one. The following is a way of documenting what the implementation looks like, and what each bit and bob does. 

## Lexing 
Lexing is fairly straightfoward, as it simply consists of iterating through chars and to find what they match to. To do that, the following class is used
```c++
class Tokenizer { 
    private:
        // The source to tokenize
        std::string source;
        // Current position in the char* array
        size_t idx; 
        // Current line 
        int line;
        // Current column
        int column; 

        // Safely get the current char, if we are at the end returns '\0'
        char current(); 
        // Used to look ahead safely (return '\0' if end is reached)
        char peek(int offset = 1);
        // Returns the char at idx and increases idx by one.
        void advance();
        // Used to check if end of file is reached.
        bool isAtEnd();
        
        // Is the current char a digit?
        bool isDigit(char c);
        // Is the current char an alphabetic character?
        bool isAlpha(char c);
        // Is the current char a digit or an alphabetic character?
        bool isAlphaNumeric(char c);
        // Is the current char a ' ', '\r', '\n' or '\t'?
        bool isWhitespace(char c);
        // Mapping to plain words to Token that are reserved for keywords (e.g. "if") 
        TokenType getKeywordType(const std::string& text);

        // Used to go over a presumed number, which can be either an integer or a real number (also supporting scientific notation)
        Token scanNumber();
        // If the current thing we are looking at contains letters and numbers (but starts with a least a letter) we use this function to create an identifier token
        Token scanIdentifier();
        // Creates tokens for string, at the time of writing it only supports
        // string like 'hello' or "world", not multiline strings
        Token scanString(char startingChar = '"');
        // Matches an operator to its equivalent token, if supported
        Token scanOperator(char ch);

        // Skips trailing whitespaces (to avoid indentation)
        void skipWhitespace();

        // Used to inform the user that something went wrong during lexing.
        void errorToken(const std::string& message);
    public:
        Tokenizer(const std::string& source) : 
            source(source), idx(0), line(1), column(1) {};
        std::vector<Token> tokenize();
        void reset(const std::string& source);
};
```
TokenType relies on an X-Macro to define the enum of TokenType(s) and string associated with an enum (used for debugging purposes).
For development purposes the most important aspect of the Lexer is how to define a new operator and/or a new keyword. Regardless of which one of the two one might want to implement, first it is necessary to define a new TokenType associated to it, inside the TokenType.hpp header file. The macro used to define a TokenType and its string is called `TOKEN_TYPES(X)`.
Defining a new keyword then requires to simply add it inside the `keywords` `unordered_map` inside the Tokenizer.cpp file. 
For operators instead, one needs to define it inside the Operator.hpp header; specifically, a new operator is defined as an entry in the `operatorTable`, which maps a char to a struct containing the tokenType, the char it represents and possible _compound_ operators (i.e., operators that can be appended to the single, one character one, to make a new operator).

## Parsing
AST generation is unfortunately not as straightforward as tokenization, as it tries to move around tokens in order to translate the semantic structure of a program into one that is more easily translatable into LLVM IR instructions.
To understand how the parser works, one needs to first define what an AST is, what are the nodes that compose the AST and finally how is it generated for the correct traversal by the IR Compiler. 
Here, AST nodes are defined with two base classes `Expression` and `Statement`.
An Expression is, intuitively, a node capable of being associated with a value, examples of strings that can be turned into expressions are literal expression such as `1+2` or `fibonacci(20)`.
On the other hand, a Statement inherently is not associated with a value, and can be instead though of as one or more actions, and evaluate to nothing. Examples of statements are `if (x > 0) {print(x)}`, `while (n > 0) {}` or `return value` (the last specifies that the function evaluation needs to return a value).
At the time of writing, the nodes defined for the Statement are the following: 
```c++
// Abstract node
class Statement {} 
// Defines the root node of the AST
class Program {} 
// As the root node of the AST is a program, ExpressionStatements are used to connect Expression nodes to Statements
class ExpressionStatement {
    std::unique_ptr<Expression> expression;
}
// Defines the rule and collects the information within a variable declaration
// The data associated with it must be available, meaning that no variable declaration can be done without initializing it with a value (a la c/c++ style).
class VariableDeclaration {
    Token variable;
    Token type;
    std::unique_ptr<Expression> expr; 
}

// Assignment of already initialized values
class Assignment{
    std::unique_ptr<Expression> lvalue;  // Left-hand side expression (*ptr, arr[i], obj.field, etc.)
    std::unique_ptr<Expression> rvalue;  // Right-hand side expression
}

// Used to assign via indexing ([] operator) values 
// to an object.
class IndexAssignment {
    std::unique_ptr<Expression> lvalue;  // The object[index] expression
    std::unique_ptr<Expression> rvalue;  // The value to assign
}

// Scoping as a critical aspect of programming and here a scope is defined in the form of a block, which is just a list of statements (within {})
class BlockStatement {
    std::vector<std::unique_ptr<Statement>> statements;
}

// If the expression evaluates to true execute the statements in the 
// then block, otherwise go to the else block (if it exists)
class IfStatement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<BlockStatement> thenBlock;
    std::unique_ptr<BlockStatement> elseBlock;
}

// Similar to the if statement, if the condition evaluates to true, execute the body
class WhileStatement {
    std::unique_ptr<Expression> condition;
    std::unique_ptr<BlockStatement> body;
}

// Used for the function and method definition
struct StatementParameter {
    Token name;
    Token type; 
};

// A function definition consists of a name, what it will return (even -> void has to be specified), and optionally its parameters. The body of the function is self-explanatory
class FunctionDefinition {
    public:
        Token functionName;
        Token functionReturnType;
        std::vector<Token> typeParameters;
        std::vector<StatementParameter> parameters;
        std::unique_ptr<BlockStatement> body;
}

// What should be evaluated when a function returns?
class ReturnStatement {
    std::unique_ptr<Expression> returnValue;
}

// Similary to argument definition
struct FieldDefinition {
    Token name;
    Token type;    
};

// Functions, but for classes
struct MethodDefinition {
    Token methodName;
    Token returnType;
    std::vector<StatementParameter> parameters;
    std::unique_ptr<BlockStatement> body;
}

// Define the components of a class, all but the classname are optional
class ClassDefinition {
    Token className;
    std::vector<Token> genericParameters;
    std::vector<FieldDefinition> fields;
    std::vector<MethodDefinition> methods;
}


// Used to call for freeing up memory for a variable
class DeferStatement {
    std::unique_ptr<Expression> expression;
};

// Used to expose APIs from a C/C++ library into Forge
class ExternStatement {
    Token functionName;
    std::vector<StatementParameter> parameters; 
    Token returnType;
}
```
The nodes defined for the Expression are these instead 
```c++
// Base class
class Expression {}

// An expression that evaluates a constant (defined in the token)
class LiteralExpression {
    Token value;
};

// An expression that evaluates an array
class ArrayLiteralExpression {
    std::vector<std::unique_ptr<Expression>> arrayValues;
};

// An expression that evaluates an array access at a specific index 
class IndexAccessExpression {
    std::unique_ptr<Expression> array;
    std::unique_ptr<Expression> index;
};

// An expression that evalutes to a member of a class (either a method or attribute)
class MemberAccessExpression {
    std::unique_ptr<Expression> object;  
    std::string memberName;              
    std::vector<std::unique_ptr<Expression>> arguments;
    bool isMethodCall; 
}

// Evaluates a value stored in a variable
class IdentifierExpression {
    std::string name;
}

// Evaluates an expression that uses binary operator
class BinaryExpression {
    std::unique_ptr<Expression> left; 
    Token operator_;
    std::unique_ptr<Expression> right;
}

// Evaluates an expression that uses a unary operator
class UnaryExpression {
    Token operator_;
    std::unique_ptr<Expression> operand;
}

// Evaluates a function call with optional arguments of a given type
class FunctionCall {
    std::string functionName;
    std::vector<Token> typeArguments;
    std::vector<std::unique_ptr<Expression>> arguments; 
}

// Similarly to function calls this evaluates a specific function, the constructor
class ObjectInstantiation {
    Token className;
    std::vector<std::unique_ptr<Expression>> arguments;
}

// Same as object instantiation but when a type is defined
// like in an array
class GenericInstantiation {
    Token className;
    std::vector<Token> typeArguments;  // [int], [string, int]
    std::vector<std::unique_ptr<Expression>> arguments;
}

// Move expression, kind of a funky function
class MoveExpression {
    Token moveToken;
    std::unique_ptr<Expression> operand;
}
```
The actual parsing process consists of then "visiting" nodes in the tree when the correct condition is found.
The following is the current definition of the Parser class
```c++
class Parser {
    private:
        std::vector<Token> tokens; 
        size_t idx = 0;

        Token current();
        Token peek(int offset = 1);
        Token advance();
        bool isValidTypeToken(TokenType type);

        std::unique_ptr<Expression> parsePrimary();
        std::unique_ptr<Expression> parseLiteral();
        std::unique_ptr<Expression> parseIdentifierExpression();
        std::unique_ptr<Expression> parseParenthesizedExpression();
        std::unique_ptr<Expression> parseArrayLiteral();
        std::unique_ptr<Expression> parseNewExpression();
        std::unique_ptr<Expression> parseGenericInstantiation(Token className); 
        std::unique_ptr<Expression> parsePostfix();
        std::unique_ptr<Expression> parseUnary();
        std::unique_ptr<Expression> parseFactor();
        std::unique_ptr<Expression> parseTerm();
        std::unique_ptr<Expression> parseComparison();
        std::unique_ptr<Expression> parseEquality();
        std::unique_ptr<Expression> parseBitwiseAnd();
        std::unique_ptr<Expression> parseBitwiseXor();
        std::unique_ptr<Expression> parseBitwiseOr();
        std::unique_ptr<Expression> parseLogicalAnd();
        std::unique_ptr<Expression> parseLogicalOr();
        
        std::vector<std::unique_ptr<Expression>> parseArgumentList();

        Token expect(TokenType expectedType, const std::string& errorMessage);

        std::unique_ptr<Statement> parseExpressionStatement();
        std::unique_ptr<Statement> parseClassDefinition();
        FieldDefinition parseFieldDefinition();
        MethodDefinition parseMethodDefinition();
        std::unique_ptr<Statement> parseVariableDeclaration();
        std::unique_ptr<Statement> parseAssignment(); 
        std::unique_ptr<Statement> parseInferredDeclaration();
        std::unique_ptr<Statement> parseIndexAssignmentOrExpression();
        std::unique_ptr<Statement> parseAssignmentOrExpression();
        std::unique_ptr<Statement> parseIfStatement();
        std::unique_ptr<Statement> parseWhileStatement();
        std::unique_ptr<Statement> parseFunctionDefinition();
        std::unique_ptr<Statement> parseReturnStatement();
        std::unique_ptr<Statement> parseDeferStatement();
        std::unique_ptr<Statement> parseExternStatement();
        std::unique_ptr<BlockStatement> parseBlockStatement();

        void skipNewLines();
    public:
        Parser(const std::vector<Token>& tokens) : tokens(tokens) {};
        std::optional<ParsedType> parseType();
        std::unique_ptr<Statement> parseStatement();
        std::unique_ptr<Expression> parseExpression();
        std::unique_ptr<Statement> parseProgram();
        void reset(const std::vector<Token>& tokens);
        bool isAtEnd();
        Token getCurrentToken();
}
```

Calls used to parse Expression are defined (in the header) in order of precedence. As an example, lets look at the expression: `2-5*2`
Such an expression will yield, following lexing the tokens:
```
NUMBER (2) at 1:1
MINUS (-) at 1:2
NUMBER (5) at 1:3
MULT (*) at 1:4
NUMBER (2) at 1:5
END_OF_FILE () at 1:6
```
The entry point of the AST generation is the `parseProgram` function, which simply iterates until the pointer to the array of tokens reaches the end.
At each iteration it will call the `parseStatement` function, and will append each statement to a list of statements.
This function is at the core of the parser, as it determines which actual process of parsing needs to be started: 
for example if a statement starts with if, the parsing of the if statement is initiated, same applies for the while, etc.
It also distinguishes between cases of a simple assignment (think `x: int = 1`) and more sofisticated assignments (such as the type inferred `x := new Array[int](10))`)
or assignment involving the use of different syntatic sugar such as pointers, array notation, and others.
Of course in our case we are not interested in that, but rather in evaluating an expression, which first goes through the parseAssignmentOrExpression function (which checks if the user is trying to assign a value or to simply evaluate something), and then executes it.
Our next step will be to visit the `parseExpression` function, which unlike the `parseStatement` function will call a series of fuction, going from the operation with lowest operator precedence to the highest. In this case our expression starts with a -, which is evaluated in the `parseTerm` function.
To actually enforce operator precedence, the note generated will have on its left the value returned by the next function which represents factors (and evaluates multiplications and division), and on its right the current evaluation. Therefore, the final tree obtained will be:
```
=== AST ===
Program:
  ExpressionStatement:
    BinaryExpression: -
      Left:
        Literal: 2 (NUMBER)
      Right:
        BinaryExpression: *
          Left:
            Literal: 5 (NUMBER)
          Right:
            Literal: 2 (NUMBER)
```
For developers, implementing the correct parsing steps requires in most instances either implementing a new node type (either of type expression or statement), or to modify one of the existing one to support a new/different syntax. Once that is done, changing the rules applied to parse the tokens in the Parser.cpp will allow to obtain the correct behaviour. It is __strongly__ advised to first implement tests for a new feature and to iteratively work on it until all tests passes during modificaiton for the parsing rules.