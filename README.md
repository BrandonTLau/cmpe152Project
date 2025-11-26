## **Overview**

This project is a **Mini C Compiler** that processes a simplified subset of the C programming language.
The compiler performs three main stages:

1. **Lexical Analysis** – Converts raw source code into tokens
2. **Parsing & AST Construction** – Builds a structured Abstract Syntax Tree
3. **Interpretation** – Executes the program by evaluating the AST



---

# **Features**

## ✔ **Lexical Analyzer**

The lexer recognizes:

* Keywords: `int`, `float`, `char`, `void`, `return`, `if`, `else`, `while`, `for`, `printf`
* Identifiers
* Integer, float, char, and string literals
* Operators: arithmetic, relational, logical
* Delimiters and punctuation
* **Comments**

  * `// single-line`
  * `/* multi-line */`

The lexer also tracks **line and column numbers**, allowing meaningful error messages.

---

## ✔ **Parser (Recursive-Descent)**

The parser constructs an AST and supports:

### **Expressions**

* Arithmetic: `+ - * / %`
* Relational: `< > <= >= == !=`
* Logical: `&& || !`
* Grouping with `()`
* Function calls
* Array indexing

### **Statements**

* Variable declarations
* Assignments
* Return statements
* If / else
* While loops
* For loops
* Expression statements
* Code blocks `{ ... }`

### **Functions**

* Multiple parameters
* Local variable scopes
* Calls between user-defined functions

### **Arrays**

* Declaration: `int a[5];`
* Indexing: `a[i]`
* Assignment inside loops

Parser errors are reported with line/column numbers and allow limited synchronization.

---

## ✔ **Interpreter**

The interpreter evaluates the AST and supports:

* Function execution (starting at `main`)
* Integer arithmetic
* Conditionals and loops
* Local and nested scopes
* Recursion-ready call stack
* Arrays
* `printf` output
* Returning values from functions
* Final process exit code = return value of `main`

---

# **Project Structure**

```
cmpe152Project/
│
├── lexer.h / lexer.cpp
├── parser.h / parser.cpp
├── ast.h
├── tokens.h
├── interpreter.cpp
├── main.cpp
│
├── valid1.c
├── valid2.c
├── valid3.c
└── invalid1.c
```

Each component is self-contained and can be improved independently.

---

# **Sample Test Programs**

### **valid1.c**

Basic variables and arithmetic.

### **valid2.c**

Functions, loops, and nested expressions.

### **valid3.c**

Arrays and accumulation.

### **invalid1.c**

Triggers a parsing error (missing semicolon).

Each program can be executed like:

```
./compiler.exe valid2.c
```

---

# **How to Build**

```
g++ -std=c++17 *.cpp -o compiler.exe
```

# **How to Run**

```
./compiler.exe filename.c
```

---

# **How to Extend**

This codebase is designed to be easy to expand. Potential additions:

* Type checking
* Additional data types
* Improved error recovery
* Intermediate code representation
* Optimization passes
* Backend code generation (LLVM, bytecode, MIPS, etc.)

---

# **Purpose of the Project**

The goal of this project is to demonstrate the fundamentals of compiler design:

* Implementing a lexer
* Building a recursive-descent parser
* Constructing an AST
* Executing the semantics of a simple language

This project creates a working mini-compiler/interpreter pipeline that can serve as a learning tool or foundation for more advanced stages.


