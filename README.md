
````markdown
# VoltScript ⚡  
**A programming language built from scratch in modern C++**

**Version:** `0.5.0`  
**Milestone:** 5 — Full interpreter with control flow  
**Status:** Stable, feature-complete for core execution 🚀

---

## ✨ What is VoltScript?

VoltScript is an **educational programming language implementation** written entirely from scratch in **C++20**.

This project is not about copying an existing language.  
It exists to **understand how real languages work internally** — by building every layer by hand:

- lexical analysis  
- parsing and AST construction  
- interpretation and runtime execution  
- variable scoping and control flow  

VoltScript is developed **incrementally**, one milestone at a time, with each milestone introducing a real subsystem found in production language runtimes.

---

## 🎯 Design philosophy

VoltScript is built with a few simple principles:

- **Correctness over shortcuts**  
- **Clarity over cleverness**  
- **Explicit logic over magic**  

Everything is written to be readable, debuggable, and educational — while still being *real*.

---

## ✅ Implemented features (Milestones 1–5)

### 🧩 Lexical Analysis (Lexer)
The lexer converts raw source code into tokens, supporting:

- **Literals**
  - Numbers: `42`, `3.14`
  - Strings: `"hello world"`
  - Booleans: `true`, `false`
  - Nil: `nil`
- **Identifiers**
  - `x`, `total_sum`, `_temp123`
- **Keywords**
  - `let`, `if`, `else`, `while`, `for`, `fn`, `return`, `print`
- **Operators**
  - Arithmetic: `+ - * / %`
  - Comparison: `== != < <= > >=`
  - Logical: `&& || !`
  - Assignment: `=`
- **Punctuation**
  - `(` `)` `{` `}` `;` `,`
- Line comments: `// comment`
- Accurate error messages with line numbers

---

### 🌳 Syntax Analysis (Parser)
A hand-written **recursive descent parser** that supports:

- Expressions:
  - unary, binary, logical, grouping, assignment
- Statements:
  - `print`
  - `let` declarations
  - blocks `{ ... }`
  - `if / else`
  - `while`
  - `for`
- Correct operator precedence and associativity
- Graceful syntax error recovery
- Clean **Abstract Syntax Tree (AST)** output

---

### 🧠 Abstract Syntax Tree (AST)
- Separate **expression** and **statement** node hierarchies
- Explicit representation of program structure
- Designed for interpretation now, compilation later
- Easy to visualize and debug during development

---

### ⚙️ Interpreter (Tree-Walk Execution)
VoltScript now **executes real programs**.

The interpreter supports:

- Arithmetic evaluation with correct precedence
- Variable declaration and assignment
- Block scoping with proper shadowing
- Control flow:
  - `if / else`
  - `while` loops
  - `for` loops
- `print` statement
- Comparison operators (`<`, `>`, `==`, etc.)
- Logical operators with **short-circuit evaluation**
- String concatenation
- Runtime type checking
- Clear runtime error messages:
  - division by zero
  - undefined variables
  - type mismatches

---

### 🗂️ Environment & Scoping
- Lexical scoping using nested environments
- Parent scope lookups
- Variable shadowing
- Proper lifetime handling

---

## 🗃️ Project structure

```text
VoltScript/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── token.h / token.cpp
│   ├── lexer.h / lexer.cpp
│   ├── ast.h / ast.cpp
│   ├── stmt.h
│   ├── parser.h / parser.cpp
│   ├── value.h / value.cpp
│   ├── environment.h / environment.cpp
│   ├── interpreter.h / interpreter.cpp
│   └── main.cpp
├── tests/
│   ├── test_lexer.cpp
│   ├── test_parser.cpp
│   ├── test_evaluator.cpp
│   └── test_interpreter.cpp
├── examples/
│   ├── fibonacci.volt
│   ├── factorial.volt
│   ├── fizzbuzz.volt
│   ├── nested_loops.volt
│   └── scope_demo.volt
└── build/
    └── bin/
        ├── volt
        └── volt_tests
````

---

## 🛠️ Building VoltScript

### Requirements

* C++ compiler with **C++20 support**

  * GCC 10+, Clang 12+, MSVC 2019+
* CMake ≥ 3.20
* Google Test (fetched automatically)

---

### Windows (MSVC)

```powershell
cmake -B build
cmake --build build --config Release
.\build\bin\Release\volt.exe
```

---

### Linux / macOS

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/bin/volt
```

---

## 💻 Using VoltScript

### Interactive REPL

```text
> let x = 10;
> print x;
10
> x = x + 5;
> print x;
15
> if (x > 10) print "big";
big
> for (let i = 0; i < 3; i = i + 1) print i;
0
1
2
```

---

### Run a script file

```bash
./volt examples/fibonacci.volt
```

---

## 📌 Language examples

### Variables & arithmetic

```volt
let x = 10;
let y = 20;
print x + y;     // 30
```

---

### Control flow

```volt
if (age >= 18) {
    print "Adult";
} else {
    print "Minor";
}
```

---

### Loops

```volt
for (let i = 0; i < 5; i = i + 1) {
    print i;
}
```

---

### Scoping

```volt
let x = "global";

{
    let x = "local";
    print x;
}

print x;
```

---

## 🧪 Testing

VoltScript has **134 unit tests**, with **132 passing**.

```bash
ctest --output-on-failure
```

Coverage:

* Lexer: 13 / 13
* Parser: 28 / 28
* Evaluator: 26 / 26
* Interpreter: 65 / 67

**Overall:** 98.5% passing

---

## 🚧 Roadmap

### Completed

* Lexer
* Parser
* AST
* Interpreter
* Variables & scoping
* Control flow
* REPL & file execution
* Comprehensive test suite

### Coming next

* Functions & closures
* Arrays and maps
* Standard library
* `return`, `break`, `continue`
* Bytecode compiler
* Virtual machine
* Garbage collection
* Module system

---

## 🧠 What you’ll learn from this project

* How lexers tokenize code
* Recursive descent parsing
* AST design
* Tree-walk interpretation
* Runtime environments & scoping
* Control flow implementation
* Error handling strategies
* Test-driven development in C++
* Writing clean, maintainable compiler code

---

## 🤝 Contributing

This is an **educational but serious** project.

Contributions are welcome — whether that’s:

* improving diagnostics
* adding language features
* optimizing execution
* expanding documentation

---

## 📄 License

MIT — free to learn from, fork, and build upon.

---

## 🙏 Acknowledgments

Inspired by:

* *Crafting Interpreters* — Robert Nystrom
* The Lox language architecture
* Modern C++ best practices

---

**VoltScript ⚡**
`v0.5.0 — Milestone 5 complete`
*From tokens to real programs*

```

---

### Why this version works
- Clear visual hierarchy
- More breathing space
- Strong human narrative
- Still technically deep
- Looks **professional on GitHub**
- Feels like a real language project, not homework

If you want next:
- a **Milestone 6 design doc**
- a **language spec (formal grammar)**
- a **clean website / landing page README**
- or a **devlog-style CHANGELOG**

Just say the word — this project is legit 🔥
```
