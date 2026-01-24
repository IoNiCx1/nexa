cmake_minimum_required(VERSION 3.16)
project(nexa LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Using LLVM ${LLVM_PACKAGE_VERSION}")

# Add the specific subfolders so #include "Ast.h" or "Lexer.h" 
# works regardless of which folder the .cpp file is in.
include_directories(
    ${LLVM_INCLUDE_DIRS}
    compiler
    compiler/lexer
    compiler/parser
    compiler/ast
    compiler/sema
    compiler/ir
    runtime
)

add_definitions(${LLVM_DEFINITIONS})

add_executable(nexa
    compiler/main.cpp
    compiler/lexer/Lexer.cpp
    compiler/parser/Parser.cpp
    compiler/sema/SemanticAnalyzer.cpp
    compiler/ir/CodeGen.cpp
)

llvm_map_components_to_libnames(LLVM_LIBS
    core
    support
    native
)

target_link_libraries(nexa PRIVATE ${LLVM_LIBS})
