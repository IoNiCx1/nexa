#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "sema/SemanticAnalyzer.h"
#include "ir/CodeGen.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <string>
#include <filesystem>
#include <stdexcept>

#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/Verifier.h>

using namespace nexa;
namespace fs = std::filesystem;

// ─────────────────────────────────────────────
// Global verbosity flag — set by --verbose
// ─────────────────────────────────────────────

static bool verbose = false;

// Log only when --verbose is passed
static void vlog(const std::string& msg) {
    if (verbose) std::cout << "[nexa] " << msg << "\n";
}

static void die(const std::string& msg) {
    std::cerr << "[nexa] error: " << msg << "\n";
    std::exit(1);
}

// Always-visible warning (not gated by verbose)
static void warn(const std::string& msg) {
    std::cerr << "[nexa] warning: " << msg << "\n";
}

static int execCmd(const std::string& cmd, const std::string& label) {
    if (verbose) std::cout << "[nexa] " << label << "\n  $ " << cmd << "\n";
    int ret = std::system(cmd.c_str());
    if (ret != 0 && verbose)
        std::cerr << "[nexa] '" << label << "' exited with code " << ret << "\n";
    return ret;
}

// ─────────────────────────────────────────────
// File utilities
// ─────────────────────────────────────────────

static std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) die("cannot open '" + path + "'");
    std::stringstream buf;
    buf << file.rdbuf();
    std::string src = buf.str();
    // Strip shebang
    if (src.size() >= 2 && src[0] == '#' && src[1] == '!') {
        auto nl = src.find('\n');
        if (nl != std::string::npos) src.erase(0, nl + 1);
    }
    return src;
}

static std::string baseName(const std::string& path) {
    return fs::path(path).stem().string();
}

static std::string dirOf(const std::string& path) {
    auto p = fs::path(path).parent_path().string();
    return p.empty() ? "." : p;
}

// ─────────────────────────────────────────────
// Token kind name (verbose diagnostics only)
// ─────────────────────────────────────────────

static const char* kindName(TokenKind k) {
    switch (k) {
        case TokenKind::IntegerLiteral: return "IntegerLiteral";
        case TokenKind::FloatLiteral:   return "FloatLiteral";
        case TokenKind::StringLiteral:  return "StringLiteral";
        case TokenKind::Identifier:     return "Identifier";
        case TokenKind::Int:            return "Int";
        case TokenKind::Double:         return "Double";
        case TokenKind::String:         return "String";
        case TokenKind::Bool:           return "Bool";
        case TokenKind::Print:          return "Print";
        case TokenKind::Loop:           return "Loop";
        case TokenKind::If:             return "If";
        case TokenKind::Else:           return "Else";
        case TokenKind::True:           return "True";
        case TokenKind::False:          return "False";
        case TokenKind::Fn:             return "Fn";
        case TokenKind::Return:         return "Return";
        case TokenKind::Plus:           return "Plus";
        case TokenKind::Minus:          return "Minus";
        case TokenKind::Star:           return "Star";
        case TokenKind::Slash:          return "Slash";
        case TokenKind::Assign:         return "Assign";
        case TokenKind::Less:           return "Less";
        case TokenKind::LessEqual:      return "LessEqual";
        case TokenKind::Greater:        return "Greater";
        case TokenKind::GreaterEqual:   return "GreaterEqual";
        case TokenKind::EqualEqual:     return "EqualEqual";
        case TokenKind::NotEqual:       return "NotEqual";
        case TokenKind::Tensor:         return "Tensor";
        case TokenKind::LeftParen:      return "LeftParen";
        case TokenKind::RightParen:     return "RightParen";
        case TokenKind::LeftBrace:      return "LeftBrace";
        case TokenKind::RightBrace:     return "RightBrace";
        case TokenKind::LeftBracket:    return "LeftBracket";
        case TokenKind::RightBracket:   return "RightBracket";
        case TokenKind::Comma:          return "Comma";
        case TokenKind::Semicolon:      return "Semicolon";
        case TokenKind::END:            return "END";
        case TokenKind::Invalid:        return "Invalid";
        default:                        return "Unknown";
    }
}

// ─────────────────────────────────────────────
// Compilation pipeline
// ─────────────────────────────────────────────

static void compileToIR(const std::string& source, const std::string& irPath) {

    // ── Stage 1: Lex ──────────────────────────
    vlog("stage 1/4 — lexing");
    std::vector<Token> tokens;
    try {
        Lexer lexer(source);
        tokens = lexer.tokenize();
    } catch (const std::exception& e) { die(std::string("lexer: ") + e.what()); }
      catch (...)                      { die("lexer: unknown exception"); }

    if (tokens.empty()) die("lexer produced zero tokens");

    if (verbose) {
        std::cout << "[nexa]   lexer OK — " << tokens.size() << " tokens\n";
        size_t preview = std::min(tokens.size(), (size_t)8);
        for (size_t i = 0; i < preview; ++i)
            std::cout << "         [" << i << "] "
                      << kindName(tokens[i].kind)
                      << " '" << tokens[i].lexeme << "'"
                      << "  L" << tokens[i].line
                      << ":C" << tokens[i].column << "\n";
        if (tokens.size() > preview)
            std::cout << "         ... (" << tokens.size() - preview << " more)\n";
    }

    // ── Stage 2: Parse ────────────────────────
    vlog("stage 2/4 — parsing");
    std::unique_ptr<Program> program;
    try {
        Parser parser(tokens);
        program = parser.parseProgram();
    } catch (const std::exception& e) { die(std::string("parser: ") + e.what()); }
      catch (...)                      { die("parser: unknown exception"); }

    if (!program) die("parser returned null program");
    vlog("parser OK — " + std::to_string(program->statements.size()) + " top-level statements");

    // ── Stage 3: Semantic analysis ────────────
    vlog("stage 3/4 — semantic analysis");
    try {
        SemanticAnalyzer sema;
        sema.analyze(*program);
    } catch (const std::exception& e) { die(std::string("sema: ") + e.what()); }
      catch (...)                      { die("sema: unknown exception"); }
    vlog("sema OK");

    // ── Stage 4: IR generation ────────────────
    vlog("stage 4/4 — IR generation");
    CodeGen codegen;
    try {
        codegen.generate(*program);
    } catch (const std::exception& e) { die(std::string("codegen: ") + e.what()); }
      catch (...)                      { die("codegen: unknown exception"); }

    auto* mod = codegen.getModule();
    if (!mod) die("codegen returned null module");

    // ── Verify IR ─────────────────────────────
    std::string verifyErr;
    llvm::raw_string_ostream vs(verifyErr);
    if (llvm::verifyModule(*mod, &vs)) {
        vs.flush();
        std::cerr << "[nexa] error: IR verification failed:\n" << verifyErr << "\n";
        if (verbose) mod->print(llvm::errs(), nullptr);
        die("aborting due to malformed IR");
    }
    vlog("IR verification OK");

    if (!mod->getFunction("main")) {
        warn("no 'main' function in generated IR");
        if (verbose) {
            std::cerr << "[nexa]   functions present:\n";
            for (auto& fn : *mod)
                std::cerr << "    - " << fn.getName().str() << "\n";
        }
    } else {
        vlog("'main' found — OK");
    }

    // ── Write IR ──────────────────────────────
    std::error_code ec;
    llvm::raw_fd_ostream dest(irPath, ec, llvm::sys::fs::OF_Text);
    if (ec) die("cannot write IR: " + ec.message());
    mod->print(dest, nullptr);
    dest.flush();
    vlog("IR written → " + irPath);
}

// ─────────────────────────────────────────────
// Usage
// ─────────────────────────────────────────────

static void printUsage(const char* argv0) {
    std::cout
        << "Usage: " << argv0 << " <file.nx> [options]\n"
        << "\n"
        << "Options:\n"
        << "  --verbose   Print compilation pipeline details\n"
        << "  --keep-ir   Keep the generated .ll file after compilation\n"
        << "  --no-run    Compile only, do not execute the output binary\n"
        << "  --help      Show this message\n";
}

// ─────────────────────────────────────────────
// Main
// ─────────────────────────────────────────────

int main(int argc, char** argv) {
    if (argc < 2) { printUsage(argv[0]); return 0; }

    // Parse flags
    std::string srcFile;
    bool keepIR = false;
    bool noRun  = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if      (arg == "--verbose") verbose = true;
        else if (arg == "--keep-ir") keepIR  = true;
        else if (arg == "--no-run")  noRun   = true;
        else if (arg == "--help")    { printUsage(argv[0]); return 0; }
        else if (arg[0] == '-')      { std::cerr << "[nexa] unknown flag: " << arg << "\n"; return 1; }
        else if (srcFile.empty())    srcFile = arg;
        else { std::cerr << "[nexa] unexpected argument: " << arg << "\n"; return 1; }
    }

    if (srcFile.empty()) { printUsage(argv[0]); return 1; }
    if (!fs::exists(srcFile)) die("source file not found: " + srcFile);

    const std::string base    = baseName(srcFile);
    const std::string outDir  = dirOf(srcFile);
    const std::string irFile  = outDir + "/" + base + ".ll";
#ifdef _WIN32
    const std::string exeFile = outDir + "/" + base + ".exe";
    const std::string runCmd  = exeFile;
#else
    const std::string exeFile = outDir + "/" + base;
    const std::string runCmd  = "./" + exeFile;
#endif

    if (verbose) {
        std::cout << "[nexa] ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
                  << "[nexa] source  : " << srcFile << "\n"
                  << "[nexa] IR      : " << irFile  << "\n"
                  << "[nexa] binary  : " << exeFile << "\n"
                  << "[nexa] ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    }

    // ── Step 1: Compile ───────────────────────
    const std::string source = readFile(srcFile);
    if (source.empty()) die("source file is empty");
    vlog("source: " + std::to_string(source.size()) + " bytes");

    compileToIR(source, irFile);
    if (!fs::exists(irFile)) die("IR file was not created");

    // ── Step 2: Find runtime ──────────────────
    // Search order:
    //   1. nexa_runtime.a next to the nexa binary  (installed / build dir)
    //   2. build/nexa_runtime.a                    (cmake build dir)
    //   3. runtime/ai/nexa_runtime.a               (source tree fallback)
    //   4. legacy tensor.o locations               (old layout)
    std::string runtimeLib;
    const std::string binDir = dirOf(argv[0]);

    auto tryPath = [&](const std::string& p) {
        if (runtimeLib.empty() && fs::exists(p)) runtimeLib = p;
    };

    tryPath(binDir + "/nexa_runtime.a");
    tryPath("build/nexa_runtime.a");
    tryPath("nexa_runtime.a");
    tryPath("runtime/ai/nexa_runtime.a");
    // legacy .o fallbacks
    tryPath(binDir + "/runtime/ai/tensor.o");
    tryPath("runtime/ai/tensor.o");

    const bool hasRuntime = !runtimeLib.empty();
    if (!hasRuntime)
        warn("runtime not found — tensor operations will fail to link.\n"
             "         Run 'make' from your build directory to compile nexa_runtime.a");
    else
        vlog("runtime: " + runtimeLib);

    // ── Step 3: Link ──────────────────────────
    std::string linkCmd = "clang++ -O1 " + irFile;
    if (hasRuntime) linkCmd += " " + runtimeLib;
    linkCmd += " -lm -lstdc++ -o " + exeFile;  // -lstdc++ needed for Tensor.cpp (std::vector, cout)
    if (!verbose) linkCmd += " 2>&1";

    if (execCmd(linkCmd, "linking") != 0) {
        // In quiet mode clang output was suppressed — re-run visibly so the
        // user actually sees the linker error
        if (!verbose) {
            std::cerr << "[nexa] link failed. Re-running with output:\n";
            std::system((linkCmd + " 2>&1").c_str());
        }
        die("link step failed — run with --verbose for details");
    }

    // ── Step 4: Run ───────────────────────────
    if (!noRun) {
        // In verbose mode print a separator; in quiet mode output is raw
        if (verbose)
            std::cout << "[nexa] ━━━━━━━━━━ output ━━━━━━━━━━\n";

        int code = std::system(runCmd.c_str());

        if (verbose) {
            std::cout << "[nexa] ━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n"
                      << "[nexa] exit code: " << code << "\n";
        }

        if (!keepIR) fs::remove(irFile);
        return code;
    }

    if (!keepIR) fs::remove(irFile);
    return 0;
}