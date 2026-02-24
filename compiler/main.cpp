#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "sema/SemanticAnalyzer.h"
#include "ir/CodeGen.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>

#include <llvm/Support/raw_ostream.h>

using namespace nexa;

// =============================
// Read File (with shebang support)
// =============================

std::string readFile(const std::string& path) {

    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "Could not open file: "
                  << path << "\n";
        exit(1);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    std::string content = buffer.str();

    // Remove shebang (#!...)
    if (content.size() >= 2 &&
        content[0] == '#' &&
        content[1] == '!') {

        size_t newline = content.find('\n');
        if (newline != std::string::npos)
            content.erase(0, newline + 1);
    }

    return content;
}

// =============================
// Compile to IR
// =============================

void compileToIR(const std::string& source,
                 const std::string& outputFile) {

    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    Parser parser(tokens);
    auto program = parser.parseProgram();

    SemanticAnalyzer sema;
    sema.analyze(*program);

    CodeGen codegen;
    codegen.generate(*program);

    std::error_code EC;
    llvm::raw_fd_ostream dest(outputFile, EC);

    if (EC) {
        std::cerr << "Could not open IR file.\n";
        exit(1);
    }

    codegen.getModule()->print(dest, nullptr);
    dest.flush();
}

// =============================
// Main
// =============================

int main(int argc, char** argv) {

    if (argc < 2) {
        std::cout << "Usage: nexa <file.nx>\n";
        return 0;
    }

    std::string file = argv[1];

    std::string source = readFile(file);

    std::string irFile = "output.ll";
    std::string exeFile = "a.out";

    compileToIR(source, irFile);

    std::string clangCmd =
        "clang " + irFile + " -o " + exeFile;

    if (system(clangCmd.c_str()) != 0) {
        std::cerr << "Clang compilation failed.\n";
        return 1;
    }

    std::string runCmd = "./" + exeFile;
    system(runCmd.c_str());

    return 0;
}