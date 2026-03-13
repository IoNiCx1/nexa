cmake_minimum_required(VERSION 3.16)
project(nexa LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------
# LLVM
# -----------------------------
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Using LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM dir: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# -----------------------------
# Source files
# -----------------------------
add_executable(nexa
    compiler/main.cpp
    compiler/lexer/Lexer.cpp
    compiler/parser/Parser.cpp
    compiler/sema/SemanticAnalyzer.cpp
    compiler/ir/CodeGen.cpp
    runtime/ai/Tensor.cpp
    runtime/ai/ai_runtime.cpp
)

# -----------------------------
# Include directories
# -----------------------------
target_include_directories(nexa PRIVATE
    compiler
    compiler/lexer
    compiler/parser
    compiler/ast
    compiler/sema
    compiler/ir
)

# -----------------------------
# LLVM linking
# -----------------------------
llvm_map_components_to_libnames(LLVM_LIBS
    core
    support
    native
    irreader
)

target_link_libraries(nexa PRIVATE ${LLVM_LIBS})

# -----------------------------
# Properties
# -----------------------------
set_target_properties(nexa PROPERTIES
    LINKER_LANGUAGE CXX
)