# 🦎 Lizard Language
<div align="center">
<img src="https://github.com/Luifegames/Lizard-Language/blob/74f78dcaae82aeed552b0b00ab53031f4293c24a/icon.svg" alt="Logo Lizard Language" width="250" />
</div>
**Lizard** is a small, C‑based interpreted language designed for learning and experimentation. It supports variables, arithmetic, comparisons, conditionals,dictionaries and functions.

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
**Dictionaries** – `{key:value}
- **Functions** – user‑defined with func, parameters, return, and recursion
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

```Lizard
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
│   ├── io.h
│   ├── lexer.h
│   ├── lizard.h
│   ├── parser.h
│   ├── variables.h
│   └── utils.h
└── src/
    ├── error.c
    ├── io.c
    ├── lexer.c
    ├── main.c
    ├── lexer.c
    ├── parser.c
    ├── variables.c
    └── utils.c
```


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

## Example

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

## Dictionaries
Lizard supports dictionaries (hash maps) that store key‑value pairs. Keys must be strings, values can be any type (numbers, strings, nested dictionaries, etc.).

### Creating a dictionary

```lizard
empty = {}
person = {"name": "Alice", "age": 30, "active": true}
```

### Accessing and modifying values
Use dot notation to read or assign values:

```lizard
print(person.name)   // "Alice"
person.age = 31      // update existing key
person.city = "Paris" // add new key-value pair
```

```lizard
if person.has("age") {
    print("Age: " + person.age)
}
```
### Printing a dictionary

print(dict) displays the dictionary in a compact format:

```lizard
print(person)   // {name:Alice,age:31,active:true,city:Paris}
```

### Notes
Keys are strings; using a non‑string key will cause an error.
Accessing a non‑existent key returns null.

## Functions

Lizard allows you to define reusable functions with parameters and return values. Functions are declared using the keyword func, followed by the name, parameters in parentheses, and the body enclosed in braces { }. A function can return a value using return.

Declaration
```lizard
func name(parameter1, parameter2, ...) {
    // function body
    return expression
}
```
Calling a function
```lizard
variable = name(argument1, argument2, ...)
```
Recursive Example: Fibonacci
```lizard
func fib(n) {
    if n <= 1 {
        return n
    } else {
        return fib(n-1) + fib(n-2)
    }
}

i = 0
while i < 10 {
    print(fib(i))
    i = i + 1
}
```
This program prints the first 10 Fibonacci numbers: 0, 1, 1, 2, 3, 5, 8, 13, 21, 34.

## License
MIT – free to use and modify.
