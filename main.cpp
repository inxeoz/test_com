#include <sys/types.h>   // For uid_t, gid_t, nlink_t
#include <mutex>          // For std::mutex

#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/Orc/ExecutionUtils.h>  // For execution utilities in Orc
#include "ast.h"


using namespace llvm;

LLVMContext Context;
IRBuilder<> Builder(Context);
std::unique_ptr<Module> TheModule = std::make_unique<Module>("my_module", Context);

// Generates LLVM IR based on AST
Value* generateLLVMCode(const AST* node) {
    if (const auto* number = dynamic_cast<const Number*>(node)) {
        return ConstantInt::get(Context, APInt(32, number->value));
    }

    if (const auto* binOp = dynamic_cast<const BinOp*>(node)) {
        Value* left = generateLLVMCode(binOp->left.get());
        Value* right = generateLLVMCode(binOp->right.get());

        switch (binOp->operation) {
            case TokenType::ADD: return Builder.CreateAdd(left, right, "add");
            case TokenType::SUB: return Builder.CreateSub(left, right, "sub");
            case TokenType::MULT: return Builder.CreateMul(left, right, "mul");
            case TokenType::DIV: return Builder.CreateSDiv(left, right, "div");
            default: throw std::invalid_argument("unexpected operator in LLVM code generation");
        }
    }

    throw std::invalid_argument("unexpected node in AST");
}

// Compiles and runs the code dynamically
void compileAndRun(std::unique_ptr<AST> ast) {
    if (!ast) {
        std::cerr << "Error: AST is null!" << std::endl;
        return;
    }

    FunctionType* funcType = FunctionType::get(Type::getInt32Ty(Context), false);
    Function* mainFunc = Function::Create(funcType, Function::ExternalLinkage, "main", TheModule.get());
    BasicBlock* entry = BasicBlock::Create(Context, "entry", mainFunc);
    Builder.SetInsertPoint(entry);

    Value* result = generateLLVMCode(ast.get());
    Builder.CreateRet(result);

    std::string errStr;
    ExecutionEngine* EE = EngineBuilder(std::move(TheModule))
        .setErrorStr(&errStr)
        .create();

    if (!EE) {
        std::cerr << "Error creating execution engine: " << errStr << std::endl;
        return;
    }

    EE->finalizeObject();
    auto mainFuncPtr = reinterpret_cast<int(*)()>(EE->getFunctionAddress("main"));
    if (!mainFuncPtr) {
        std::cerr << "Error: Could not find function 'main'" << std::endl;
        return;
    }

    std::cout << "Result: " << mainFuncPtr() << std::endl;
}



int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl;
        return 1;
    }

    std::string code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    try {
        Parser parser(code);
        auto statements = parser.parse();

        for (const auto& stmt : statements) {
            printAST(stmt.get());
            std::cout << "Result: " << calculate(stmt.get()) << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}


// TIP See CLion help at <a
// href="https://www.jetbrains.com/help/clion/">jetbrains.com/help/clion/</a>.
//  Also, you can try interactive lessons for CLion by selecting
//  'Help | Learn IDE Features' from the main menu.