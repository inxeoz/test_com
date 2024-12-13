
#include "logic.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/Verifier.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace llvm;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Error: Could not open file " << argv[1] << "\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string input = buffer.str();

    Parser parser(input);
    auto ast = parser.parse();




    LLVMContext context;
    Module module("my_module", context);
    IRBuilder<> builder(context);

    FunctionType* funcType = FunctionType::get(builder.getInt32Ty(), false);
    Function* mainFunction = Function::Create(funcType, Function::ExternalLinkage, "main", module);
    BasicBlock* entry = BasicBlock::Create(context, "entry", mainFunction);
    builder.SetInsertPoint(entry);

    Value* result = ast->codegen(builder);

    // for (auto expr : ast) {
    //     //Value* result = expr->codegen(builder);
    //     // Optionally, print the result or store it for further use.
    //     printf(expr);
    // }

    // Add printf declaration
    FunctionType* printfType = FunctionType::get(builder.getInt32Ty(), builder.getInt8PtrTy(), true);
    Function* printfFunc = Function::Create(printfType, Function::ExternalLinkage, "printf", module);

    // Create format string
    Value* formatStr = builder.CreateGlobalStringPtr("%d\n");

    // Call printf with result
    builder.CreateCall(printfFunc, {formatStr, result});

    builder.CreateRet(builder.getInt32(0));

    if (verifyModule(module, &errs())) {
        std::cerr << "Module verification failed!\n";
        return 1;
    }

    module.print(outs(), nullptr);
    return 0;
}



