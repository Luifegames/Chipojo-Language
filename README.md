# 🦎 Chipojo

**Chipojo** is a small, C-based interpreted programming language designed for learning and experimentation. This repository contains the full toolchain: interpreter, package manager, and VS Code extension.

> [!NOTE]
> Las condiciones en `if`, `elif`, `while` y `for` aceptan **ambas formas**: con y sin paréntesis.
> ```chipojo
> if (x > 5) { show("ok") }
> if x > 5  { show("ok") }  // también válido
> ```

## Components

| Component | Description | Location |
|-----------|-------------|----------|
| **Interpreter** | Single-pass tree-walk interpreter written in C | `Chipojo-Language/` |
| **Package Manager** | `chpm` – npm-like CLI for Chipojo packages | `chpm/` |
| **VS Code Extension** | Syntax highlighting, snippets, icon theme | `chipojo-vscode/` |

## Quick Start

```bash
# Build the interpreter
cd Chipojo-Language && make

# Run a script
./chipojo script.chp
```

> [!TIP]
> **Rama dev** — Contiene las últimas características: `switch`, `class`, `void`, `def`, arrow functions, imports nombrados y más.

## Language Overview

- Dynamic types: numbers, strings, booleans, lists, dictionaries, functions, null
- Conditionals: `if` / `elif` / `else` and `switch` / `case` / `default`
- Loops: `while`, `for`, `mientras`
- Functions: `func` / `def`, parameters, return, recursion, arrow functions (`=>`)
- Classes: `class`, `public` / `private`, `void` methods
- Error handling: `try` / `catch` / `throw`
- Module system: `import` / `export`, scoped packages (`@creator/module`), named imports
- Built-in methods: `.push()`, `.pop()`, `.size()`, `.length`, `.has()`, `.set()`, etc.

## Installation

### VS Code Extension

Install `chipojo-language` v0.3.0+ from `chipojo-vscode/` for syntax highlighting, snippets, and icon theme support.

### chpm (Package Manager)

```bash
cd chpm && make
chpm init        # Create a new project
chpm install math  # Install a package
```

## Project Structure

```
Chipojo/
├── Chipojo-Language/     # Interpreter (C source)
│   ├── src/              # Lexer, parser, evaluator, methods, variables
│   ├── include/          # Header files
│   ├── lib/              # Standard library (.chp modules)
│   └── Makefile
├── chpm/                 # Package manager (C source)
│   ├── src/main.c        # All chpm commands
│   └── packets_chpm_create/  # Built-in package repository
├── chipojo-vscode/       # VS Code extension
│   ├── syntaxes/         # TextMate grammar
│   ├── snippets/         # Code snippets
│   └── themes/           # Sybo icon theme
├── examples/             # Example .chp programs
├── test/                 # Test suites
└── docs/                 # Documentation
```

## License

MIT – free to use and modify.
