#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "sema/SemanticAnalyzer.cpp"
#include "codegen/CodeGen.h"
#include <llvm/Support/raw_ostream.h>

int main() {
    std::string src = "<a,b> = 1,2";

    Lexer lex(src);
    Parser parser(lex);
    auto program = parser.parseProgram();

    SemanticAnalyzer sema;
    sema.analyze(*program);

    LLVMState llvm;
    CodeGen codegen(llvm);
    codegen.generate(*program);

    llvm.module->print(llvm::outs(), nullptr);
}
