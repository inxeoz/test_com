//
// Created by inxeoz on 12/13/24.
//
#pragma once
#ifndef AST_H
#define AST_H
#include <mutex>          // For std::mutex
#include <iostream>
#include <string>
#include <utility>
#include <cctype>
#include <memory>
#include <stdexcept>
#include <llvm/IR/IRBuilder.h>

using namespace llvm;

enum class TokenType {
    NUMBER, ADD, SUB, MULT, DIV, SEMI, END
};

struct Token {
    TokenType type;
    int value;

    explicit Token(TokenType t, int v = 0) : type(t), value(v) {}
};

class Lexer {
    std::string input;
    size_t pos = 0;

public:
    explicit Lexer(std::string text) : input(std::move(text)) {}

    Token getNextToken() {
        while (pos < input.length() && std::isspace(input[pos])) { pos++; }
        if (pos >= input.length()) { return Token(TokenType::END); }

        char currentChar = input[pos];
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
                throw std::invalid_argument(std::string("unexpected character: ") + currentChar);
        }
    }
};

struct AST {
    virtual ~AST() = default;
};

struct Number final : AST {
    int value;
    explicit Number(int v) : value(v) {}
};

struct BinOp : AST {
    std::unique_ptr<AST> left;
    TokenType operation;
    std::unique_ptr<AST> right;
    BinOp(std::unique_ptr<AST> left, TokenType operation, std::unique_ptr<AST> right)
        : left(std::move(left)), operation(operation), right(std::move(right)) {}
};

class Parser {
    Lexer lexer;
    Token currentToken;

    void eat(const TokenType type) {
        if (currentToken.type == type) {
            currentToken = lexer.getNextToken();
        } else {
            throw std::invalid_argument("unexpected token: expected " + std::to_string(static_cast<int>(type)) +
                            ", got " + std::to_string(static_cast<int>(currentToken.type)));
        }
    }

    std::unique_ptr<AST> factor() {
        if (currentToken.type == TokenType::NUMBER) {
            auto node = std::make_unique<Number>(currentToken.value);
            eat(TokenType::NUMBER); // Eat currentToken and update currentToken
            return node;
        }
        throw std::invalid_argument("unexpected syntax");
    }

    std::unique_ptr<AST> term() {
    auto node = factor(); // This ensures that node is a single number or parenthesized expression
    while (currentToken.type == TokenType::MULT || currentToken.type == TokenType::DIV) {
        TokenType operation = currentToken.type;
        eat(operation); // Eat currentToken and update currentToken
        node = std::make_unique<BinOp>(std::move(node), operation, factor());
    }
    return node;
}

std::unique_ptr<AST> expr() {
    auto node = term(); // Parse the first term
    while (currentToken.type == TokenType::ADD || currentToken.type == TokenType::SUB) {
        TokenType operation = currentToken.type;
        eat(operation); // Eat currentToken and update currentToken
        node = std::make_unique<BinOp>(std::move(node), operation, term()); // Parse another term
    }
    return node;
}


    void consumeSemicolon() {
        if (currentToken.type == TokenType::SEMI) {
            eat(TokenType::SEMI);
        } else {
            throw std::invalid_argument("Expected semicolon");
        }
    }

public:
    explicit Parser(const std::string& text) : lexer(text), currentToken(lexer.getNextToken()) {}

    std::vector<std::unique_ptr<AST>> parse() {
        std::vector<std::unique_ptr<AST>> statements;

        while (currentToken.type != TokenType::END) {
            auto statement = expr(); // Parse the current expression
            statements.push_back(std::move(statement));
            consumeSemicolon(); // Consume the semicolon after the statement
        }

        return statements;
    }
};


void printAST(const AST* node, int depth = 0) {
    if (const auto* number = dynamic_cast<const Number*>(node)) {
        std::cout << std::string(depth, ' ') << "Number: " << number->value << std::endl;
    } else if (const auto* binOp = dynamic_cast<const BinOp*>(node)) {
        std::cout << std::string(depth, ' ') << "Operator ";
        switch (binOp->operation) {
            case TokenType::ADD: std::cout << '+'; break;
            case TokenType::SUB: std::cout << '-'; break;
            case TokenType::MULT: std::cout << '*'; break;
            case TokenType::DIV: std::cout << '/'; break;
            default: std::cout << '?'; break;
        }
        std::cout << std::endl;
        printAST(binOp->left.get(), depth + 2);
        printAST(binOp->right.get(), depth + 2);
    }
}

int calculate(const AST* root ) {
    if (const auto* number = dynamic_cast<const Number*>(root)) {
        return number->value;
    }
    if (const auto* binOp = dynamic_cast<const BinOp*>(root)) {
        int leftValue = calculate(binOp->left.get());
        int rightValue = calculate(binOp->right.get());
        switch (binOp->operation) {
            case TokenType::ADD: return leftValue + rightValue;
            case TokenType::SUB: return leftValue - rightValue;
            case TokenType::MULT: return leftValue * rightValue;
            case TokenType::DIV:
                if (rightValue == 0)
                    throw std::invalid_argument("division by zero");
                return leftValue / rightValue;
            default: throw std::invalid_argument("expected number or operator");
        }
    }
    throw std::invalid_argument("unexpected syntax");
}

#endif //AST_H
