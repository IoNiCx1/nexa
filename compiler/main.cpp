#include <iostream>
#include <fstream>
#include <sstream>

// LLVM includes for file output
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "sema/SemanticAnalyzer.h"
#include "ir/CodeGen.h"
#include "ir/LLVMContext.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: nexa <file.nx> [--emit-ir] [-o output]\n";
        return 1;
    }

    std::string inputFile;
    std::string outputFile = "output.ll"; // Defaulting to .ll since we emit IR
    bool emitIR = false;

    // ---- Parse arguments ----
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "--emit-ir") {
            emitIR = true;
        }
        else if (arg == "-o" && i + 1 < argc) {
            outputFile = argv[++i];
        }
        else {
            inputFile = arg;
        }
    }

    // ---- Read source file ----
    std::ifstream file(inputFile);
    if (!file) {
        std::cerr << "Error: Cannot open file " << inputFile << "\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    // ---- Frontend ----
    Lexer lexer(source);
    Parser parser(lexer);
    auto program = parser.parseProgram();

    if (!program) {
        std::cerr << "Parsing failed\n";
        return 1;
    }

    // ---- Semantic Analysis ----
    try {
        SemanticAnalyzer sema;
        sema.analyze(*program);
    } catch (const std::exception& e) {
        std::cerr << "Semantic Error: " << e.what() << "\n";
        return 1;
    }

    // ---- Codegen ----
    LLVMState state;
    CodeGen generator(state);
    generator.generate(*program);

    // ---- Output to Terminal ----
    if (emitIR) {
        state.module->print(llvm::outs(), nullptr);
    }

    // ---- Output to File ----
    std::error_code EC;
    llvm::raw_fd_ostream dest(outputFile, EC, llvm::sys::fs::OF_None);

    if (EC) {
        std::cerr << "Error: Could not open file for writing: " << EC.message() << "\n";
        return 1;
    }

    state.module->print(dest, nullptr);
    dest.flush();

    std::cout << "Compiled successfully → " << outputFile << "\n";
    return 0;
}
