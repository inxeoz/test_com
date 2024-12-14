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
    if (argc != 2) { // Ensure the correct number of arguments
        std::cerr << "Usage: " << argv[0] << " <filename>\n"; // Print usage message if not correct
        return 1;
    }

    std::ifstream file(argv[1]); // Open the file specified in the arguments
    if (!file) { // Check if the file was opened successfully
        std::cerr << "Error: Could not open file " << argv[1] << "\n"; // Print error if file cannot be opened
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf(); // Read the contents of the file into a string stream
    std::string input = buffer.str(); // Convert the buffer to a string

    Parser parser(input); // Create a parser with the input string
    auto ast = parser.parse(); // Parse the input and generate an AST

    LLVMContext context; // Create an LLVM context for code generation
    Module module("my_module", context); // Create an LLVM module
    IRBuilder<> builder(context); // Create an IRBuilder for generating LLVM instructions

    // Create the main function in the module with a return type of int32
    FunctionType* funcType = FunctionType::get(builder.getInt32Ty(), false);
    Function* mainFunction = Function::Create(funcType, Function::ExternalLinkage, "main", module);
    BasicBlock* entry = BasicBlock::Create(context, "entry", mainFunction); // Create an entry basic block for main
    builder.SetInsertPoint(entry); // Set the builder to insert instructions into the entry block

    // Generate code for each expression in the AST
    for (auto& expression : *ast) {
        expression->codegen(builder); // Generate the code for each expression
    }

    // Add a declaration for printf function
    FunctionType* printfType = FunctionType::get(builder.getInt32Ty(), builder.getInt8PtrTy(), true);
    Function* printfFunc = Function::Create(printfType, Function::ExternalLinkage, "printf", module);

    // Create a format string for printf
    Value* formatStr = builder.CreateGlobalStringPtr("%d\n");

    // Call printf to print each expression result
    for (auto& expression : *ast) {
        Value* result = expression->codegen(builder); // Generate code for the expression
        builder.CreateCall(printfFunc, {formatStr, result}); // Call printf with the format string and result
    }

    builder.CreateRet(builder.getInt32(0)); // Return 0 from the main function

    // Verify the module to ensure it is valid
    if (verifyModule(module, &errs())) {
        std::cerr << "Module verification failed!\n"; // Print error if module verification fails
        return 1;
    }

    module.print(outs(), nullptr); // Print the module to standard output
    return 0; // Return success
}
