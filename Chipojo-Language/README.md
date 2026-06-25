# 🦎 Chipojo Language
<div align="center">
<img src="icon.svg" alt="Logo Chipojo Language" width="250" />
</div>
**Chipojo** is a small, C‑based interpreted language designed for learning and experimentation.

## Features

- **Variables** – integers, floats, booleans, strings, lists, dictionaries  
- **Assignment** – `=`, compound: `+=`, `-=`, `*=`, `/=`, increment/decrement: `++`, `--`  
- **Logical Operators** – `and`, `or`, `not`  
- **Arithmetic** – `+`, `-`, `*`, `/` and parentheses  
- **Comparisons** – `==`, `!=`, `<`, `>`, `<=`, `>=` (return `1` or `0`)  
- **Print** – `show(...)` with `+` concatenation  
- **Conditionals** – `if`, `elif`, `else` with `{ }` blocks (parentheses opcionales en la condición)  
- **Switch** – `switch` / `case` / `default` control flow  
- **Loops** – `while`, `for`, and the alias `mientras` (parentheses opcionales en la condición)  
- **Functions** – `func` or `def`, parameters, return values, recursion, arrow functions (`=>`)  
- **Classes** – `class` with `public` / `private` members and `void` methods  
- **Lists** – `[v1, v2, v3]` with dynamic methods (`.push()`, `.pop()`, `.size()`, etc.)  
- **Dictionaries** – `{key: value}` with `.get()`, `.set()`, `.has()`  
- **Properties** – `.length` for strings, lists and dictionaries  
- **Error Handling** – `try { ... } catch (err) { ... }` (alias `cach`) plus `throw`  
- **Modules / Packages** – `import` / `export`, scoped packages (`@creator/module`), named imports: `import { hello } from "module"`  
- **File extension** – `.chp`  
- **Version** – `chipojo -v` shows ASCII art

> [!NOTE]
> Las condiciones en `if`, `elif`, `while` y `for` **no requieren paréntesis obligatorios**. Ambas formas son válidas:
> ```chipojo
> if (x > 5) { ... }  // estilo C
> if x > 5 { ... }    // también funciona
> ```

## Building

Clone the repository and compile with `make`:

```bash
git clone https://github.com/Luifegames/Chipojo-Language.git
cd chipojo
make
```
The executable chipojo (or Chipojo.exe on Windows) will be created.

## Usage
Run an Lizaes script:

```bash
./chipojo script.chp
```
Show version:

```bash
./chipojo -v
```

> [!IMPORTANT]
> Los paquetes con alcance (`@creator/module`) permiten imports cortos: `import module from "module"`.  
> También puedes importar funciones específicas: `import { hello } from "module"`.

## Packages

Chipojo supports local packages installed in `chpm_modules/`, including scoped packages:

```text
chpm_modules/@creator/module/main.chp
```

Import a package by its package name. The `@creator` folder is for chpm ownership/registry layout:

```chipojo
import module from "module"

show(module.hello("Chipojo"))
```

Named exports still work when the package declares `export func`:

```chipojo
from "module" import { hello }
```

Create a package scaffold with:

```bash
chpm create @creator/module
```

`chpm create module` asks for the author username and creates `@username/module`. Users send that project to a chpm admin; only admins publish it to the official chpm repository/database.

## Example
Create a file hello.chp:

```Chipojo
name = "Chipojo"
size = 10

if (size >= 10) {
    show("Welcome to " + name + "!")
} else {
    show("too small")
}

show("2 + 2 = " + (2 + 2))
show("5 > 3 is " + (5 > 3))
```
Run it:

```bash
./chipojo hello.chp
```

Output:

```text
Welcome to Chipojo!
2 + 2 = 4
5 > 3 is 1
```

## UTF‑8 Support
Chipojo reads .chp files in binary mode and preserves UTF‑8 byte sequences inside string literals. To display accented characters (á, é, í, ó, ú, ñ) correctly:

## Project Structure
```text
Chipojo/
├── Makefile
├── include/          
│   ├── error.h
│   ├── io.h
│   ├── lexer.h
│   ├── chipojo.h
│   ├── methods.h
│   ├── natives.h
│   ├── parser.h
│   ├── variables.h
│   └── utils.h
└── src/
    ├── error.c
    ├── io.c
    ├── lexer.c
    ├── main.c
    ├── methods.h
    ├── natives.h
    ├── lexer.c
    ├── parser.c
    ├── variables.c
    └── utils.c
```


## Installation Chipojo Language for VS Code

1. Download the latest `.vsix` file from the [Plugin](https://github.com/Luifegames/chipojo-language-vscode/releases/tag/v0.0.1) page of this repository.

2. Open **VS Code**.

3. Go to the Extensions view:
   - Press `Ctrl+Shift+X`
   - Or click the Extensions icon in the Activity Bar.

4. Click on the `...` (More Actions) menu and select **"Install from VSIX..."**.

5. Choose the `.vsix` file you downloaded.

6. Reload VS Code when prompted.

## Usage

Once installed, the extension automatically activates when you open a `.chp` file.

- Keywords (`if`, `else`, `while`, `show`, `and`, `or`, `not`) will have syntax highlighting.
- Strings, numbers, comments, and operators are also highlighted.

## Example

Open a file named `hello.chp`:

```chipojo
name = "Chipojo"
size = 10

if (size >= 10) {
    show("Welcome to " + name + "!")
} else {
    show("too small")
}

show("2 + 2 = " + (2 + 2))
show("5 > 3 is " + (5 > 3))
```

## Error Handling

Chipojo supports a minimal `try/catch` flow:

```chipojo
try {
    throw "boom"
} catch (err) {
    show(err)
}
```

`catch` also accepts the temporary alias `cach`.

## `.length`

Use `.length` on strings, lists and dictionaries:

```chipojo
show("niño".length)       // 4
show([1, 2, 3].length)    // 3
show({"a": 1}.length)     // 1
```

## Dictionaries
Chipojo supports dictionaries (hash maps) that store key‑value pairs. Keys must be strings, values can be any type (numbers, strings, nested dictionaries, etc.).

### Creating a dictionary

```chipojo
empty = {}
person = {"name": "Alice", "age": 30, "active": true}
```

### Accessing and modifying values
Use dot notation to read or assign values:

```chipojo
show(person.name)   // "Alice"
person.age = 31      // update existing key
person.city = "Paris" // add new key-value pair
```

```chipojo
if person.has("age") {
    show("Age: " + person.age)
}
```
### Printing a dictionary

show(dict) displays the dictionary in a compact format:

```chipojo
show(person)   // {name:Alice,age:31,active:true,city:Paris}
```

### Notes
Keys are strings; using a non‑string key will cause an error.
Accessing a non‑existent key returns null.

## Lists

Chipojo supports dynamic lists with built-in methods.

### Creating Lists
```chipojo
numbers = [1, 2, 3]

names = ["Luis", "Ana", "Carlos"]

mixed = [1, "hello", null]
```

### Access Elements
```chipojo
numbers = [10, 20, 30]

show(numbers[0]) // 10
show(numbers[1]) // 20
Nested Lists
matrix = [
    [1, 2],
    [3, 4]
]

show(matrix[1][0]) // 3
```

### List Methods
- **size()**

Returns the amount of elements in the list.
```chipojo
numbers = [1, 2, 3]

show(numbers.size()) // 3
```

- **get(index)**

Returns the element at the specified index.
```chipojo
numbers = [10, 20, 30]

show(numbers.get(2)) // 30
```
- **push(value)**

Adds a new element to the end of the list.
```chipojo
numbers = [1, 2]

numbers.push(3)

show(numbers) // [1, 2, 3]
```

- **pop()**

Removes and returns the last element.
```chipojo
numbers = [1, 2, 3]

value = numbers.pop()

show(value) // 3

show(numbers) // [1, 2]
```

- **is_empty()**

Returns true if the list is empty.
```chipojo
numbers = []

show(numbers.is_empty()) // true
```

- **insert(index,value)**

Insert a value inside the list.
```chipojo
nums = [1,2,3]

nums.insert(1, 99)

show(nums) // [1,99,2,3]
```
- **remove(index)**
Remove a value of the list
```chipojo
nums = [10,20,30]

nums.remove(1)

show(nums) //[10,30]
```
- **contains(value)**

Returns true if the value exists in the list.
```chipojo
nums = [1, 2, 3]

show(nums.contains(2)) // 1
```
- **find(value)**

Returns the index of the value inside the list.
```chipojo
nums = [10, 20, 30]

show(nums.find(20)) // 1
```
- **reverse()**

Reverses the list in place.
```chipojo
nums = [1, 2, 3]

nums.reverse()

show(nums) // [3, 2, 1]
```

## Functions

Chipojo allows you to define reusable functions with parameters and return values. Functions are declared using the keyword func, followed by the name, parameters in parentheses, and the body enclosed in braces { }. A function can return a value using return.

Declaration
```chipojo
func name(parameter1, parameter2, ...) {
    // function body
    return expression
}
```
Calling a function
```chipojo
variable = name(argument1, argument2, ...)
```
Recursive Example: Fibonacci
```chipojo
func fib(n) {
    if n <= 1 {
        return n
    } else {
        return fib(n-1) + fib(n-2)
    }
}

i = 0
while i < 10 {
    show(fib(i))
    i = i + 1
}
```
This program prints the first 10 Fibonacci numbers: 0, 1, 1, 2, 3, 5, 8, 13, 21, 34.

> [!TIP]
> **Nuevo en v0.7.0 (dev):** `switch`, `class`, `void`, `def`, arrow functions, alcances con `@creator`, e imports nombrados. ¡Mira las secciones nuevas abajo!

## Switch

Chipojo supports `switch` / `case` / `default` for multi-branch control flow:

```chipojo
switch (value) {
    case 1 {
        show("one")
    }
    case 2 {
        show("two")
    }
    default {
        show("other")
    }
}
```

## Classes

Define classes with `class`, declare members as `public` or `private`, and methods as `void` or with `func`/`def`:

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

## `def` alias

`def` can be used interchangeably with `func`:

```chipojo
def add(a, b) {
    return a + b
}
```

## Arrow Functions

Compact function syntax with `=>`:

```chipojo
double = x => x * 2
add = (a, b) => a + b
```

## VS Code Extension

Install the **Chipojo Language** extension (v0.3.0+) for:

- Syntax highlighting (keywords, strings, numbers, operators, methods)
- Code snippets (`if`, `while`, `for`, `switch`, `class`, `func`, `try`/`catch`, `import`, `arrow function`, etc.)
- File icon for `.chp` files (Sybo icon theme)
- Language configuration (bracket matching, auto-closing pairs, comment toggling)

Install via the VSIX file in `chipojo-vscode/` or download from the releases page.

> [!WARNING]
> Las funciones exportadas actualmente **no capturan variables privadas del módulo** (closures/module environments aún no implementados). Los paquetes generados usan metadatos literales hasta que se implementen.

## License
MIT – free to use and modify.
