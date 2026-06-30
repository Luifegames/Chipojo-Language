<div align="center">
<img src="assets/icon.svg" alt="Logo Chipojo Language" width="450" />
</div>

**Chipojo** is a small interpreted programming language written in C, designed for learning and experimentation.

> [!NOTE]
> Conditions in `if`, `elif`, `while` and `for` **do not require mandatory parentheses**. Both forms are valid:
>
> ```chipojo
> if (x > 5) { ... }  // C style
> if x > 5  { ... }   // also works
> ```

## Features

- **Variables** – integers, floats, booleans, strings, lists, dictionaries
- **Assignment** – `=`, compound: `+=`, `-=`, `*=`, `/=`, increment/decrement: `++`, `--`
- **Logical operators** – `and`, `or`, `not`
- **Arithmetic** – `+`, `-`, `*`, `/` and parentheses
- **Comparisons** – `==`, `!=`, `<`, `>`, `<=`, `>=` (return `1` or `0`)
- **Output** – `show(...)` with concatenation `+`

> [!TIP]
> **New:** `switch`, `class`, `void`, `def`, arrow functions (`=>`), named imports, error handling, classes, function exports, and more.

- **Conditionals** – `if`, `elif`, `else` with `{ }`
- **Switch** – `switch` / `case` / `default`
- **Loops** – `while`, `for`
- **Functions** – `func` or `def`, parameters, return, recursion, arrow functions (`=>`)
- **Classes** – `class` with `public` / `private` members and `void` methods
- **Lists** – `[v1, v2, v3]` with methods: `.push()`, `.pop()`, `.size()`, `.get()`, `.insert()`, `.remove()`, `.contains()`, `.find()`, `.reverse()`, `.clear()`, `.is_empty()`
- **Dictionaries** – `{key: value}` with `.get()`, `.set()`, `.has()`
- **Properties** – `.length` on strings, lists and dictionaries
- **Errors** – `try { ... } catch (err) { ... }` and `throw`
- **Modules** – `import` / `export`, named imports: `import { hello } from "module"`

> [!IMPORTANT]
> Module names are looked up in: `./name.chp` → `./name/main.chp` → `./chpm_modules/name.chp` → `./chpm_modules/name/main.chp`

- **Extension** – `.chp`
- **Version** – `chipojo -v` shows ASCII art

## Compiling

### Linux / macOS

```bash
git clone https://github.com/Luifegames/Chipojo-Language
cd Chipojo
make
```

The `chipojo` executable is generated in the project root.

> [!TIP]
> To use `chipojo` from any terminal, add the folder to your `PATH`:
>
> ```bash
> echo 'export PATH=$PATH:'$(pwd) >> ~/.bashrc
> source ~/.bashrc
> ```

### Windows

Option A — **MinGW / MSYS2:**

```bash
cd Chipojo
gcc -Ichipojo-interpreter/include chipojo-interpreter/src/*.c -o chipojo.exe
```

Option B — **WSL (Windows Subsystem for Linux):**

```bash
# Install WSL with Ubuntu, then:
cd Chipojo
make
./chipojo script.chp
```

> [!NOTE]
> The `Makefile` uses `gcc` and is intended for Linux/WSL. On native Windows, use MinGW or compile with WSL.

## Usage

```bash
./chipojo script.chp
./chipojo -v          # version
```

## Example

```chipojo
name = "Chipojo"
size = 10

if (size >= 10) {
    show("Welcome to " + name + "!")
} else {
    show("too small")
}

show("2 + 2 = " + (2 + 2))
```

```text
Welcome to Chipojo!
2 + 2 = 4
```

## Switch

```chipojo
switch (value) {
    case 1: { show("one") }
    case 2: { show("two") }
    default: { show("other") }
}
```

## Classes

```chipojo
class Person {
    public name
    public age
    private id

    void init(name, age) {
        this.name = name
        this.age = age
    }

    func greet() {
        show("Hello, I'm " + this.name)
    }
}

p = Person("Ana", 25)
p.greet()
```

## Arrow Functions

```chipojo
double = x => x * 2
sum = (a, b) => a + b
```

## Lists

```chipojo
nums = [1, 2, 3]
nums.push(4)
show(nums.size())     // 4
show(nums.contains(2)) // 1
nums.reverse()
show(nums)            // [4, 3, 2, 1]
```

## Dictionaries

```chipojo
person = {"name": "Ana", "age": 30}
show(person.name)    // "Ana"
person.age = 31
show(person.has("age")) // 1
```

## Errors

```chipojo
try {
    throw "error!"
} catch (err) {
    show("Caught: " + err)
}
```

## VS Code Extension

The **Chipojo Language** extension (`vscode-extension/`) provides:

- Syntax highlighting (keywords, strings, numbers, operators, methods)
- Code snippets (`if`, `while`, `for`, `switch`, `class`, `func`, `try`/`catch`, `import`, arrow functions, etc.)
- Sybo icon theme (`.chp` files with a custom icon)
- Language configuration (bracket matching, auto‑closing, comments)

### Installation

```bash
# From VS Code:
# 1. Extensions (Ctrl+Shift+X) → ... → Install from VSIX...
# 2. Select vscode-extension/chipojo-language-0.3.0.vsix

# Or from terminal:
code --install-extension vscode-extension/chipojo-language-0.3.0.vsix
```

> [!TIP]
> The `.vsix` is already packaged in `vscode-extension/`. Just import it and you're done.

## Modules and Closures

Chipojo supports modules with `import`/`export`. Exported functions **capture the module scope** (closures), allowing access to private module variables:

```chipojo
// module.chp
private = 42

export func get() {
    return private  // captures 'private' from the module
}
```

```chipojo
// main.chp
import module
show(module.get())  // 42
```

> [!TIP]
> Arrow functions (`=>`) also capture closures when exported from a module.

## Tests

```bash
make test
```

Runs all `.chp` scripts in `test/` to verify functionality.

## Try it out

The scripts in `test/` are practical examples so you can see how the language works and experiment.

## Project structure

```
Chipojo/
├── chipojo-interpreter/  # Language interpreter (C)
│   ├── src/              # Source code (.c)
│   ├── include/          # Headers (.h)
│   └── lib/              # Standard library (.chp)
├── vscode-extension/     # VS Code extension
├── assets/               # Resources (icons)
├── test/                 # Test scripts
├── docs/                 # Documentation
│   └── es/               # Spanish docs
├── Makefile              # Compile: make
└── LICENSE               # MIT
```

## License

MIT – free to use and modify.
```
