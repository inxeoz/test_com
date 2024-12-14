#ifndef DEFINITION_H
#define DEFINITION_H
#include <llvm/IR/IRBuilder.h>
#include <string>
#include <memory>



// Token types
enum class TokenType {
    ADD, SUB, MULT, DIV, NUMBER, SEMI, END
};

// Token class
class Token {
public:
    TokenType type;
    int value;

    explicit Token(const TokenType type) : type(type), value(0) {}
    Token(const TokenType type, const int value) : type(type), value(value) {}
};

// Lexer class
class Lexer {
public:
    explicit Lexer(std::string text);
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

    explicit Number(const int value) : value(value) {}
    llvm::Value* codegen(llvm::IRBuilder<> &builder) override;
};

class BinOp final : public AST {
public:
    std::unique_ptr<AST> left, right;
    TokenType operation;

    BinOp(std::unique_ptr<AST> left, TokenType operation, std::unique_ptr<AST> right);
    llvm::Value* codegen(llvm::IRBuilder<> &builder) override;
};

// Parser class
class Parser {
public:
    explicit Parser(const std::string& text);
    std::unique_ptr<std::vector<std::unique_ptr<AST>>> parse();

private:
    Lexer lexer;
    Token currentToken;

    void eat(TokenType type);
    std::unique_ptr<AST> factor();
    std::unique_ptr<AST> term();
    std::unique_ptr<AST> expr();
};

#endif // DEFINITION_H
