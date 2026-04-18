# Nexa Programming Language

Nexa is a modern compiled programming language designed for **performance, simplicity, and built-in AI capabilities**. The language is built with a custom compiler pipeline and uses **LLVM** as its backend for optimized native machine code generation.

The goal of Nexa is to combine the **speed of low-level languages** with the **ease of high-level scripting**, while providing **native tools for machine learning and artificial intelligence training**.

---

# Project Goals

The Nexa project aims to build a language that:

* Compiles directly to native machine code
* Uses LLVM for high performance optimization
* Provides a simple and readable syntax
* Supports modern programming constructs
* Includes built-in AI and machine learning tools
* Allows developers to train and run AI models directly in Nexa

---

# Language Features

Current language features include:

### Functions

```
fn add(int a, int b) -> int {
    return a + b;
}

print(add(2,3));
```

### Loops

```
loop(0,3){
    print("hello");
}
```

### Variables

```
int x = 10;
int y = 20;
print(x + y);
```

### Function Calls

```
print(add(5,6));
```

---

# Compiler Architecture

Nexa follows a traditional compiler design:

```
Source Code
   ↓
Lexer
   ↓
Parser
   ↓
AST (Abstract Syntax Tree)
   ↓
LLVM Code Generation
   ↓
Native Machine Code
```

### Components

Lexer
Responsible for tokenizing the source code.

Parser
Builds the Abstract Syntax Tree (AST) from tokens.

AST
Represents the program structure internally.

Code Generator
Transforms the AST into LLVM Intermediate Representation.

LLVM Backend
Compiles LLVM IR into optimized native machine code.

---

# Building Nexa

Requirements:

* C++ Compiler
* LLVM
* Make

Clone the repository:

```
git clone https://github.com/yourname/nexa
cd nexa
```

Build the compiler:

```
make
```

---

# Running Nexa Programs

Example:

```
./nexa program.nx
```

Example file:

```
fn mul(int a, int b)-> int{
    return a*b;
}

print(mul(2,3));
```

---

# Project Structure

```
nexa/
│
├── src/
│   ├── lexer.cpp
│   ├── parser.cpp
│   ├── ast.cpp
│   ├── codegen.cpp
│
├── runtime/
│   ├── memory.cpp
│   ├── builtin.cpp
│
├── tests/
│   ├── hello.nx
│   ├── loop.nx
│
├── Makefile
└── README.md
```

---

# Planned Features

Future versions of Nexa will include:

### Arrays and Data Structures

```
int[] arr = [10,20,30];
print(arr[1]);
```

### Conditional Statements

```
if(x > 5){
    print("big");
}
```

### Modules and Package System

```
import math
import ai
```

### Standard Library

* File IO
* Data structures
* Networking
* JSON parsing

---



# Contributing

Contributions are welcome.

You can help improve:

* Compiler optimizations
* Language features
* Runtime performance
* AI modules
* Standard library

---

# License

This project is released under the MIT License.

---

# Vision

Nexa aims to become a **modern high-performance programming language with built-in AI capabilities**, combining:

* the simplicity of scripting languages
* the performance of compiled languages
* the power of modern machine learning frameworks

---

# Status

Nexa is currently in **active development**.


in case of new pulls these lines are required:
rm -rf build  && rm -rf CMakeFiles && rm -f CMakeCache.txt  && mkdir build  && cd build  && cmake ..
