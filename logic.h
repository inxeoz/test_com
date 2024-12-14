#ifndef LOGIC_H
#define LOGIC_H

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>

// Token types
enum class TokenType {
    ADD, SUB, MULT, DIV, NUMBER, SEMI, END
};

// Token class
class Token {
public:
    TokenType type;
    int value;

    Token(TokenType type) : type(type), value(0) {}
    Token(TokenType type, int value) : type(type), value(value) {}
};

// Lexer class
class Lexer {
public:
    Lexer(std::string text);
    Token getNextToken();

private:
    std::string input;
    size_t pos = 0;
};

// AST class and subclasses
class AST {
public:
    virtual ~AST() = default;
    virtual llvm::Value* codegen(llvm::IRBuilder<> &builder) = 0;
};

class Number : public AST {
public:
    int value;

    Number(int value) : value(value) {}
    llvm::Value* codegen(llvm::IRBuilder<> &builder) override;
};

class BinOp : public AST {
public:
    std::unique_ptr<AST> left, right;
    TokenType operation;

    BinOp(std::unique_ptr<AST> left, TokenType operation, std::unique_ptr<AST> right);
    llvm::Value* codegen(llvm::IRBuilder<> &builder) override;
};

// Parser class
class Parser {
public:
    Parser(const std::string& text);
    std::unique_ptr<std::vector<std::unique_ptr<AST>>> parse();

private:
    Lexer lexer;
    Token currentToken;

    void eat(TokenType type);
    std::unique_ptr<AST> factor();
    std::unique_ptr<AST> term();
    std::unique_ptr<AST> expr();
};

// Helper function to log output
inline void save_output_to_debug(const std::string& value) {
    std::ofstream log("debug.log", std::ios::app);
    if (log.is_open()) {
        log << ";" << value << std::endl;
    } else {
        std::cerr << "Error opening debug.log for writing." << std::endl;
    }
}

// TokenType to string for logging
inline std::string getTokenString(TokenType type) {
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
inline Lexer::Lexer(std::string text) : input(std::move(text)) {}

inline Token Lexer::getNextToken() {
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

// Number implementation
inline llvm::Value* Number::codegen(llvm::IRBuilder<> &builder) {
    return builder.getInt32(value);
}

// BinOp implementation
inline BinOp::BinOp(std::unique_ptr<AST> left, TokenType operation, std::unique_ptr<AST> right)
    : left(std::move(left)), operation(operation), right(std::move(right)) {}

inline llvm::Value* BinOp::codegen(llvm::IRBuilder<> &builder) {
    llvm::Value* R = right->codegen(builder);
    llvm::Value* L = left->codegen(builder);

    // Logging the binOp for debug
    std::string binOpStr = "<binOp <" + getTokenString(operation) + ", " + std::to_string(reinterpret_cast<intptr_t>(L)) + ", " + std::to_string(reinterpret_cast<intptr_t>(R)) + ">>";
    save_output_to_debug(binOpStr);

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

// Parser implementation
inline Parser::Parser(const std::string& text) : lexer(text), currentToken(lexer.getNextToken()) {}

inline std::unique_ptr<std::vector<std::unique_ptr<AST>>> Parser::parse() {
    auto exprs = std::make_unique<std::vector<std::unique_ptr<AST>>>();

    while (currentToken.type != TokenType::END) {
        exprs->push_back(expr());
        eat(TokenType::SEMI); // Eat the semicolon
    }

    return exprs;
}

inline void Parser::eat(TokenType type) {
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

#endif // LOGIC_H

