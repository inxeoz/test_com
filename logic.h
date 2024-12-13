#ifndef LOGIC_H
#define LOGIC_H

#include "definition.h"
#include <llvm/Support/raw_ostream.h>

// Lexer method to get the next token
inline Token Lexer::getNextToken() {
    while (pos < input.length() && std::isspace(input[pos])) { pos++; }
    if (pos >= input.length()) { return Token(TokenType::END); }

    const char currentChar = input[pos];
    if (std::isdigit(currentChar)) {
        int value = 0;
        while (pos < input.length() && std::isdigit(input[pos])) {
            value = value * 10 + input[pos++] - '0';
        }
        return Token(TokenType::NUMBER, value);
    }

    pos++; // Skip current non-digit character.

    switch (currentChar) {
        case '+': return Token(TokenType::ADD);
        case '-': return Token(TokenType::SUB);
        case '*': return Token(TokenType::MULT);
        case '/': return Token(TokenType::DIV);
        case ';': return Token(TokenType::SEMI);
        default:
            throw std::invalid_argument("Unexpected character: " + std::string(1, currentChar));
    }
}

// Number's codegen method
inline Value* Number::codegen(IRBuilder<> &builder) {
    return builder.getInt32(value);
}

// BinOp's codegen method
inline Value* BinOp::codegen(IRBuilder<> &builder) {
    Value* R = right->codegen(builder);
    Value* L = left->codegen(builder);


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
            throw std::invalid_argument("unexpected operation in BinOp");
    }
}

// Parser method to consume tokens
inline void Parser::eat(const TokenType type) {
    if (currentToken.type == type) {
        currentToken = lexer.getNextToken();
    } else {
        throw std::invalid_argument("Unexpected token in syntax");
    }
}

// Parser method for factor (numbers and parentheses)
inline std::unique_ptr<AST> Parser::factor() {
    if (currentToken.type == TokenType::NUMBER) {
        auto node = std::make_unique<Number>(currentToken.value);
        eat(TokenType::NUMBER);
        return node;
    }
    throw std::invalid_argument("Unexpected factor token");
}

// Parser method for terms (multiplication/division)
inline std::unique_ptr<AST> Parser::term() {
    auto node = factor();
    while (currentToken.type == TokenType::MULT || currentToken.type == TokenType::DIV) {
        TokenType operation = currentToken.type;
        eat(operation);
        node = std::make_unique<BinOp>(std::move(node), operation, factor());
    }
    return node;
}

// Parser method for expressions (addition/subtraction)
inline std::unique_ptr<AST> Parser::expr() {
    auto node = term();
    while (currentToken.type == TokenType::ADD || currentToken.type == TokenType::SUB) {
        TokenType operation = currentToken.type;
        eat(operation);
        node = std::make_unique<BinOp>(std::move(node), operation, term());
    }
    return node;
}

// Constructor for Parser
inline Parser::Parser(const std::string& text) : lexer(text), currentToken(lexer.getNextToken()) {}

// Parser's parse method to get the AST
inline std::unique_ptr<AST> Parser::parse() {
    return expr();
}

// Constructor definition for Lexer
inline Lexer::Lexer(std::string text) : input(std::move(text)) {}

// Constructor definition for BinOp
inline BinOp::BinOp(std::unique_ptr<AST> left, TokenType operation, std::unique_ptr<AST> right)
    : left(std::move(left)), operation(operation), right(std::move(right)) {}

#endif // LOGIC_H
