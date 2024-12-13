#ifndef DEFINITION_H
#define DEFINITION_H

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <cctype>

using namespace llvm;

// Define TokenType enumeration for different types of tokens
enum class TokenType {
    NUMBER, ADD, SUB, MULT, DIV, SEMI, END
};

// Token structure representing each individual token in the input
struct Token {
    TokenType type;
    int value;

    explicit Token(TokenType t, int v = 0) : type(t), value(v) {}
};

// Lexer class to tokenize input string
class Lexer {
    std::string input;
    size_t pos = 0;

public:
    explicit Lexer(std::string text);
    Token getNextToken();
};

// Abstract base class for AST nodes
struct AST {
    virtual ~AST() = default;
    virtual Value* codegen(IRBuilder<> &builder) = 0;
};

// Number node representing an integer value
struct Number final : AST {
    int value;
    explicit Number(int v) : value(v) {}
    Value* codegen(IRBuilder<> &builder) override;
};

// Binary operation node representing operations like addition, subtraction, etc.
struct BinOp : AST {
    std::unique_ptr<AST> left;
    TokenType operation;
    std::unique_ptr<AST> right;

    BinOp(std::unique_ptr<AST> left, TokenType operation, std::unique_ptr<AST> right);
    Value* codegen(IRBuilder<> &builder) override;
};

// Parser class to parse the input into an AST
class Parser {
    Lexer lexer;


    void eat(TokenType type);
    std::unique_ptr<AST> factor();
    std::unique_ptr<AST> term();
    std::unique_ptr<AST> expr();

public:
    Token currentToken;
    explicit Parser(const std::string& text);
    std::unique_ptr<AST> parse();
};

#endif // DEFINITION_H
