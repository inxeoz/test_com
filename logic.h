#ifndef LOGIC_H
#define LOGIC_H

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <llvm/IR/IRBuilder.h>
#include "definition.h"

// Helper function to log output to a debug file
// Appends a string value to a debug log file for inspection
inline void save_output_to_debug(const std::string& value) {
    if (std::ofstream log("debug.log", std::ios::app); log.is_open()) {
        log << ";" << value << std::endl; // Log the value with a semicolon separator
    } else {
        std::cerr << "Error opening debug.log for writing." << std::endl; // Error in case file can't be opened
    }
}

// Converts TokenType to string for logging purposes
// Used to get a readable string representation of a token type for debugging
inline std::string getTokenString(const TokenType type) {
    switch (type) {
        case TokenType::ADD: return "ADD";
        case TokenType::SUB: return "SUB";
        case TokenType::MULT: return "MULT";
        case TokenType::DIV: return "DIV";
        case TokenType::SEMI: return "SEMI";
        default: return "UNKNOWN";
    }
}

// Lexer implementation
// Lexer constructor initializes the input string and starts tokenizing from the beginning
inline Lexer::Lexer(std::string text) : input(std::move(text)) {}

// getNextToken retrieves the next token from the input stream
// Skips whitespace and identifies digits or operators
inline Token Lexer::getNextToken() {
    while (pos < input.length() && std::isspace(input[pos])) { pos += 1; } // Skip whitespace
    if (pos >= input.length()) { return Token(TokenType::END); } // End of input

    const char currentChar = input[pos];
    if (std::isdigit(currentChar)) { // If it's a number
        int value = 0;
        while (pos < input.length() && std::isdigit(input[pos])) {
            value = value * 10 + (input[pos] - '0'); // Convert string digits to integer
            pos += 1;
        }
        return {TokenType::NUMBER, value};
    }

    pos += 1; // Move past the operator
    switch (currentChar) { // Handle operators
        case '+': return Token ( TokenType::ADD );
        case '-': return Token ( TokenType::SUB);
        case '*': return Token ( TokenType::MULT);
        case '/': return Token ( TokenType::DIV);
        case ';': return Token ( TokenType::SEMI);
        default:
            throw std::invalid_argument("Unexpected character: " + std::string(1, currentChar)); // Error handling
    }
}

// Number class represents a numeric literal
// Provides a code generation method to return the integer value as an LLVM value
inline llvm::Value* Number::codegen(llvm::IRBuilder<> &builder) {
    return builder.getInt32(value); // Create an integer LLVM value from the literal
}

// BinOp class represents a binary operation (e.g., addition, subtraction)
// Takes two operands (left and right) and an operation type (e.g., ADD, SUB)
// Generates code based on the operation type
inline BinOp::BinOp(std::unique_ptr<AST> left, const TokenType operation, std::unique_ptr<AST> right)
    : left(std::move(left)), operation(operation), right(std::move(right)) {}

// Code generation for binary operations
// Depending on the operation, the appropriate LLVM instruction (add, sub, mul, div) is created
inline llvm::Value* BinOp::codegen(llvm::IRBuilder<> &builder) {
    llvm::Value* R = right->codegen(builder); // Right operand codegen
    llvm::Value* L = left->codegen(builder);  // Left operand codegen

    // Logging the binOp for debugging purposes
    const std::string binOpStr = "<binOp <" + getTokenString(operation) + ", " + std::to_string(reinterpret_cast<intptr_t>(L)) + ", " + std::to_string(reinterpret_cast<intptr_t>(R)) + ">>";
    save_output_to_debug(binOpStr); // Save the operation details to debug log

    switch (operation) {
        case TokenType::ADD:
            return builder.CreateAdd(L, R, "addtmp");
        case TokenType::SUB:
            return builder.CreateSub(L, R, "subtmp");
        case TokenType::MULT:
            return builder.CreateMul(L, R, "multmp");
        case TokenType::DIV:
            return builder.CreateSDiv(L, R, "divtmp");
        default:
            throw std::invalid_argument("unexpected operation in BinOp"); // Error if unknown operation
    }
}

// Parser implementation
// Initializes the parser with an input string and the first token
inline Parser::Parser(const std::string& text) : lexer(text), currentToken(lexer.getNextToken()) {}

// parse processes the input and returns a list of parsed expressions (ASTs)
inline std::unique_ptr<std::vector<std::unique_ptr<AST>>> Parser::parse() {
    auto exprs = std::make_unique<std::vector<std::unique_ptr<AST>>>();

    while (currentToken.type != TokenType::END) { // Parse until the end token is encountered
        exprs->push_back(expr()); // Parse each expression
        eat(TokenType::SEMI); // Eat the semicolon (statement separator)
    }

    return exprs; // Return the vector of parsed expressions
}

// eat consumes a specific token type and advances to the next token
// Throws an error if the expected token is not found
inline void Parser::eat(const TokenType type) {
    if (currentToken.type == type) {
        currentToken = lexer.getNextToken(); // Move to the next token
    } else {
        throw std::invalid_argument("Unexpected token in syntax"); // Error if token type doesn't match
    }
}

// factor handles number literals, which are the basic building blocks of expressions
inline std::unique_ptr<AST> Parser::factor() {
    if (currentToken.type == TokenType::NUMBER) { // If it's a number
        auto node = std::make_unique<Number>(currentToken.value); // Create a Number AST node
        eat(TokenType::NUMBER); // Consume the number token
        return node; // Return the number node
    }
    throw std::invalid_argument("Unexpected factor token"); // Error if it's not a number
}

// term handles multiplication and division
inline std::unique_ptr<AST> Parser::term() {
    auto node = factor(); // Start with a factor (a number)
    while (currentToken.type == TokenType::MULT || currentToken.type == TokenType::DIV) { // Look for * or /
        TokenType operation = currentToken.type;
        eat(operation); // Consume the operator
        node = std::make_unique<BinOp>(std::move(node), operation, factor()); // Build a binary operation node
    }
    return node; // Return the resulting term
}

// expr handles addition and subtraction (higher-level operations)
inline std::unique_ptr<AST> Parser::expr() {
    auto node = term(); // Start with a term
    while (currentToken.type == TokenType::ADD || currentToken.type == TokenType::SUB) { // Look for + or -
        TokenType operation = currentToken.type;
        eat(operation); // Consume the operator
        node = std::make_unique<BinOp>(std::move(node), operation, term()); // Build a binary operation node
    }
    return node; // Return the resulting expression
}

#endif // LOGIC_H
