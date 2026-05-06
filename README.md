# 🦎 Lizard Language

**Lizard** is a small, C‑based interpreted language designed for learning and experimentation. It supports variables, arithmetic, comparisons, conditionals, and string concatenation in `print` statements.

## Features

- **Variables** – integers, float, boolean and strings  
- **Assignment** – `=`  
- **Logical Operator** – `and, or, not`  
- **Compound Assignment** – `= += -= *= /=`  
- **Arithmetic** – `+ - * / ++ --` and parentheses  
- **Comparisons** – `== != < > <= >=` (return `1` or `0`)  
- **Print** – `print(...)` with `+` concatenation (strings, ints, variables)  
- **Conditionals** – `if`, `elif`, `else` with `{ }` blocks  
- **Loops** – `while`  
- **File extension** – `.lzd`  
- **Version** – `Lizard -v` shows ASCII art

## Building

Clone the repository and compile with `make`:

```bash
git clone https://github.com/Luifegames/Lizard-Language.git
cd lizard
make
```
The executable lizard (or Lizard.exe on Windows) will be created.

## Usage
Run an Lizaes script:

```bash
./lizard script.lzd
```
Show version:

```bash
./lizard -v
```

## Example
Create a file hello.lzd:

```text
name = "Lizard"
size = 10

if (size >= 10) {
    print("Welcome to " + name + "!")
} else {
    print("too small")
}

print("2 + 2 = " + (2 + 2))
print("5 > 3 is " + (5 > 3))
```
Run it:

```bash
./lizard hello.lzd
```

Output:

```text
Welcome to Lizard!
2 + 2 = 4
5 > 3 is 1
```

## UTF‑8 Support
Lizard reads .lzd files in binary mode and preserves UTF‑8 byte sequences inside string literals. To display accented characters (á, é, í, ó, ú, ñ) correctly:

## Project Structure
```text
Lizard/
├── Makefile
├── include/          
│   ├── error.h
│   ├── lexer.h
│   ├── lizard.h
│   ├── parser.h
│   ├── variables.h
│   └── utils.h
└── src/
    ├── error.c
    ├── lexer.c
    ├── main.c
    ├── lexer.c
    ├── parser.c
    ├── variables.c
    └── utils.c
```
## License
MIT – free to use and modify.


## Installation Lizard Language for VS Code

1. Download the latest `.vsix` file from the [Releases](https://github.com/Luifegames/lizard-language/releases) page of this repository.

2. Open **VS Code**.

3. Go to the Extensions view:
   - Press `Ctrl+Shift+X`
   - Or click the Extensions icon in the Activity Bar.

4. Click on the `...` (More Actions) menu and select **"Install from VSIX..."**.

5. Choose the `.vsix` file you downloaded.

6. Reload VS Code when prompted.

## Usage

Once installed, the extension automatically activates when you open a `.lzd` file.

- Keywords (`if`, `else`, `while`, `print`, `and`, `or`, `not`) will have syntax highlighting.
- Strings, numbers, comments, and operators are also highlighted.

### Example

Open a file named `hello.lzd`:

```lizard
name = "Lizard"
size = 10

if (size >= 10) {
    print("Welcome to " + name + "!")
} else {
    print("too small")
}

print("2 + 2 = " + (2 + 2))
print("5 > 3 is " + (5 > 3))
```