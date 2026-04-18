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
# AI + File Runtime — static library
# Linked into every compiled .nx program at the clang++ step in main.cpp
# -----------------------------
add_library(nexa_runtime STATIC
    runtime/ai/Tensor.cpp
    runtime/file/FileRuntime.cpp
)
target_include_directories(nexa_runtime PRIVATE
    runtime/ai
    runtime/file
)

# Copy nexa_runtime.a next to the nexa binary after every build
add_custom_command(TARGET nexa_runtime POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:nexa_runtime>
        ${CMAKE_BINARY_DIR}/nexa_runtime.a
    COMMAND ${CMAKE_COMMAND} -E copy          
        $<TARGET_FILE:nexa_runtime>           
        ${CMAKE_SOURCE_DIR}/nexa_runtime.a   
    COMMENT "Copying nexa_runtime.a → build dir"
)

# -----------------------------
# Compiler executable
# -----------------------------
add_executable(nexa
    compiler/main.cpp
    compiler/lexer/Lexer.cpp
    compiler/parser/Parser.cpp
    compiler/sema/SemanticAnalyzer.cpp
    compiler/sema/Type.cpp
    compiler/ir/CodeGen.cpp
)

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

set_target_properties(nexa PROPERTIES LINKER_LANGUAGE CXX)