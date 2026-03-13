#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "sema/SemanticAnalyzer.h"
#include "ir/CodeGen.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <string>

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
// Extract base filename
// =============================

std::string getBaseName(const std::string& path) {

    size_t slash = path.find_last_of("/\\");
    std::string filename =
        (slash == std::string::npos)
        ? path
        : path.substr(slash + 1);

    size_t dot = filename.find_last_of('.');
    std::string base =
        (dot == std::string::npos)
        ? filename
        : filename.substr(0, dot);

    return base;
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

    // Read source
    std::string source = readFile(file);

    // Extract filename
    std::string base = getBaseName(file);

    std::string irFile = base + ".ll";

#ifdef _WIN32
    std::string exeFile = base + ".exe";
#else
    std::string exeFile = base;
#endif

    // Compile to LLVM IR
    compileToIR(source, irFile);

    // Link with the AI Runtime silently
    std::string runtimeObj = "runtime/ai/tensor.o";
    std::string clangCmd = "clang++ " + irFile + " " + runtimeObj + " -o " + exeFile + " 2>/dev/null";

    if (system(clangCmd.c_str()) != 0) {
        std::cerr << "Error: Linker failed. Ensure runtime/ai/tensor.o exists.\n";
        return 1;
    }

    // Run executable
#ifdef _WIN32
    std::string runCmd = exeFile;
#else
    std::string runCmd = "./" + exeFile;
#endif

    // Execute the compiled nexa program
    system(runCmd.c_str());

    // Cleanup temporary IR file
    remove(irFile.c_str());

    return 0;
}