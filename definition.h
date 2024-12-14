#ifndef DEFINITION_H
#define DEFINITION_H
#include <llvm/IR/IRBuilder.h>
#include <string>
#include <memory>

// Enum representing different types of tokens in the lexer
enum class TokenType {
    ADD, SUB, MULT, DIV, NUMBER, SEMI, END // Define token types (operations, numbers, semicolons)
};

// Token class represents a lexical token
// It holds the token's type and its associated value (for numbers)
class Token {
public:
    TokenType type; // Token type (ADD, SUB, etc.)
    int value; // Token value (used for numbers)

    explicit Token(const TokenType type) : type(type), value(0) {} // Constructor for operator tokens
    Token(const TokenType type, const int value) : type(type), value(value) {} // Constructor for number tokens
};

// Lexer class is responsible for breaking down input text into tokens
class Lexer {
public:
    explicit Lexer(std::string text); // Constructor that initializes the lexer with the input text
    Token getNextToken(); // Method to retrieve the next token from the input stream

private:
    std::string input; // Input string to tokenize
    size_t pos = 0; // Position in the input string
};

// Abstract class representing an Abstract Syntax Tree (AST) node
// All AST nodes inherit from this class and must implement codegen to generate LLVM IR
class AST {
public:
    virtual ~AST() = default; // Virtual destructor
    virtual llvm::Value* codegen(llvm::IRBuilder<> &builder) = 0; // Pure virtual function for code generation
};

// Number class represents a numeric literal in the AST
class Number : public AST {
public:
    int value; // Value of the number

    explicit Number(const int value) : value(value) {} // Constructor
    llvm::Value* codegen(llvm::IRBuilder<> &builder) override; // Code generation for a number
};

// BinOp class represents a binary operation (e.g., addition, multiplication)
class BinOp final : public AST {
public:
    std::unique_ptr<AST> left, right; // Left and right operands of the binary operation
    TokenType operation; // Type of the operation (ADD, SUB, etc.)

    BinOp(std::unique_ptr<AST> left, TokenType operation, std::unique_ptr<AST> right); // Constructor
    llvm::Value* codegen(llvm::IRBuilder<> &builder) override; // Code generation for the binary operation
};

// Parser class is responsible for parsing the input text and producing an AST
class Parser {
public:
    explicit Parser(const std::string& text); // Constructor that initializes the parser with the input
    std::unique_ptr<std::vector<std::unique_ptr<AST>>> parse(); // Method to parse the input and generate AST

private:
    Lexer lexer; // Lexer instance to tokenize the input
    Token currentToken; // Current token being processed

    void eat(TokenType type); // Helper function to consume a specific token
    std::unique_ptr<AST> factor(); // Handle factor-level operations (numbers)
    std::unique_ptr<AST> term(); // Handle term-level operations (multiplication/division)
    std::unique_ptr<AST> expr(); // Handle expression-level operations (addition/subtraction)
};

#endif // DEFINITION_H
