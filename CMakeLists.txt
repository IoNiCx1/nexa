cmake_minimum_required(VERSION 3.16)
project(nexa LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------
# LLVM
# -----------------------------
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Using LLVM ${LLVM_PACKAGE_VERSION}")

include_directories(
    ${LLVM_INCLUDE_DIRS}
    compiler
    compiler/lexer
    compiler/parser
    compiler/ast
    compiler/sema
    compiler/ir
)

add_definitions(${LLVM_DEFINITIONS})

# -----------------------------
# Executable
# -----------------------------
add_executable(nexa
    compiler/main.cpp
    compiler/lexer/Lexer.cpp
    compiler/parser/Parser.cpp
    compiler/sema/SemanticAnalyzer.cpp
    compiler/ir/CodeGen.cpp
)

# -----------------------------
# LLVM Linking
# -----------------------------
llvm_map_components_to_libnames(LLVM_LIBS
    core
    support
    native
    irreader
)

target_link_libraries(nexa PRIVATE ${LLVM_LIBS})

# -----------------------------
# Prevent legacy files from being auto-picked
# -----------------------------
set_target_properties(nexa PROPERTIES
    LINKER_LANGUAGE CXX
)
