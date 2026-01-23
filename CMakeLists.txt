cmake_minimum_required(VERSION 3.16)
project(nexa LANGUAGES C CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Using LLVM ${LLVM_PACKAGE_VERSION}")

include_directories(
    ${LLVM_INCLUDE_DIRS}
    compiler
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
