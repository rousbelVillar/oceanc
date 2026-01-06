# Ocean Compiler Project

A **simple compiler/interpreter** built with **Flex** and **Bison**. It executes a small script‑like language featuring variables, control structures, native functions (`print`, `mid`, etc.), and string handling.

The project also includes an example program that converts **integers to Roman numerals**.

---

## Features

* Lexical analysis with **Flex**
* Syntax parsing with **Bison**
* Simple AST-based execution
* Variables and expressions
* Control structures (conditionals / loops)
* Built‑in functions
* String manipulation
* Example script: **Integer → Roman numeral conversion**

---

## System Requirements

### Required Software

* **GCC** (or any compatible C compiler)
* **Flex** — lexical analyzer generator
* **Bison** — parser generator
* **Make** (optional, but recommended)

### Supported Operating Systems

* Linux (Ubuntu, Fedora, Arch, etc.)
* macOS
* Windows via:

  * **WSL (Windows Subsystem for Linux)** — recommended
  * **MSYS2** or **Cygwin**

---

## Verify Installation

Ensure all required tools are installed:

```bash
gcc --version
flex --version
bison --version
make --version
```

---

## Project Structure

```text
.
├── scanner.l        # Flex lexer definition
├── parser.y         # Bison grammar
├── ast.c / ast.h    # Abstract Syntax Tree implementation
├── main.c           # Program entry point
├── numeros_romanos.mini  # Example script
└── README.md
```

---

## Quick Start (Without Make)

### 1. Generate the Lexer and Parser

```bash
flex scanner.l
bison -d parser.y
```

### 2. Compile

```bash
gcc -o mini main.c ast.c parser.tab.c lex.yy.c
```

### 3. Run the Compiler / Interpreter

Run the Roman numerals example:

```bash
./mini numeros_romanos.mini
```

---

## Cleaning Generated Files

To remove generated and compiled files:

```bash
rm -f mini lex.yy.c parser.tab.c parser.tab.h
```

---

## Notes

* This project is intended for **educational purposes**.
* The language syntax is minimal and designed to demonstrate how a compiler/interpreter works internally.
* Easily extendable with new grammar rules or native functions.

---

## License

This project is provided for academic and learning use. Add a license file if you plan to distribute or modify it publicly.
