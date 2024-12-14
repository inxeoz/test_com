#ifndef LOGIC_H
#define LOGIC_H

#include <fstream>
#include <iostream>

#include "definition.h"
#include <llvm/Support/raw_ostream.h>

// Lexer method to get the next token
inline  void save_output_to_debug(const std::string& value) {
    std::ofstream log("debug.log", std::ios::app);
    if (log.is_open()) {
        log << ";" << value << std::endl;
    } else {
        std::cerr << "Error opening debug.log for writing." << std::endl;
    }
}

inline Token Lexer::getNextToken() {
    save_output_to_debug(input + " with pos " + std::to_string(pos));

    while (pos < input.length() && std::isspace(input[pos])) { pos += 1; }
    if (pos >= input.length()) { return Token(TokenType::END); }

    const char currentChar = input[pos];
    if (std::isdigit(currentChar)) {
        int value = 0;
        while (pos < input.length() && std::isdigit(input[pos])) {
            value = value * 10 + (input[pos] - '0');
            pos += 1;
        }
        return Token(TokenType::NUMBER, value);
    }

    pos += 1;

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

inline Value* Number::codegen(IRBuilder<> &builder) {
    return builder.getInt32(value);
}

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

inline void Parser::eat(const TokenType type) {
    if (currentToken.type == type) {
        currentToken = lexer.getNextToken();
    } else {
        throw std::invalid_argument("Unexpected token in syntax");
    }
}

inline std::unique_ptr<AST> Parser::factor() {
    if (currentToken.type == TokenType::NUMBER) {
        auto node = std::make_unique<Number>(currentToken.value);
        eat(TokenType::NUMBER);
        return node;
    }
    throw std::invalid_argument("Unexpected factor token");
}

inline std::unique_ptr<AST> Parser::term() {
    auto node = factor();
    while (currentToken.type == TokenType::MULT || currentToken.type == TokenType::DIV) {
        TokenType operation = currentToken.type;
        eat(operation);
        node = std::make_unique<BinOp>(std::move(node), operation, factor());
    }
    return node;
}

inline std::unique_ptr<AST> Parser::expr() {
    auto node = term();
    while (currentToken.type == TokenType::ADD || currentToken.type == TokenType::SUB) {
        TokenType operation = currentToken.type;
        eat(operation);
        node = std::make_unique<BinOp>(std::move(node), operation, term());
    }
    return node;
}

inline Parser::Parser(const std::string& text) : lexer(text), currentToken(lexer.getNextToken()) {
}

inline std::unique_ptr<AST> Parser::parse() {
    return expr();
}

inline Lexer::Lexer(std::string text) : input(std::move(text)) {}

inline BinOp::BinOp(std::unique_ptr<AST> left, TokenType operation, std::unique_ptr<AST> right)
    : left(std::move(left)), operation(operation), right(std::move(right)) {}

#endif // LOGIC_H

