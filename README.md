<div align="center">
<img src="assets/banner.png" alt="Logo Chipojo Language" width="450" />
</div>

**Chipojo** es un lenguaje de programación interpretado, pequeño y escrito en C, diseñado para aprender y experimentar.

> [!NOTE]
> Las condiciones en `if`, `elif`, `while` y `for` **no requieren paréntesis obligatorios**. Ambas formas son válidas:
>
> ```chipojo
> if (x > 5) { ... }  // estilo C
> if x > 5  { ... }   // también funciona
> ```

## Características

- **Variables** – enteros, flotantes, booleanos, cadenas, listas, diccionarios
- **Asignación** – `=`, compuesta: `+=`, `-=`, `*=`, `/=`, incremento/decremento: `++`, `--`
- **Operadores lógicos** – `and`, `or`, `not`
- **Aritmética** – `+`, `-`, `*`, `/` y paréntesis
- **Comparaciones** – `==`, `!=`, `<`, `>`, `<=`, `>=` (devuelven `1` o `0`)
- **Mostrar** – `show(...)` con concatenación `+`

> [!TIP]
> **Nuevo:** `switch`, `class`, `void`, `def`, arrow functions (`=>`), imports nombrados y más.

- **Condicionales** – `if`, `elif`, `else` con `{ }`
- **Switch** – `switch` / `case` / `default`
- **Bucles** – `while`, `for`
- **Funciones** – `func` o `def`, parámetros, retorno, recursión, arrow functions (`=>`)
- **Clases** – `class` con miembros `public` / `private` y métodos `void`
- **Listas** – `[v1, v2, v3]` con métodos: `.push()`, `.pop()`, `.size()`, `.get()`, `.insert()`, `.remove()`, `.contains()`, `.find()`, `.reverse()`, `.clear()`, `.is_empty()`
- **Diccionarios** – `{key: value}` con `.get()`, `.set()`, `.has()`
- **Propiedades** – `.length` en cadenas, listas y diccionarios
- **Errores** – `try { ... } catch (err) { ... }` y `throw`
- **Módulos** – `import` / `export`, imports nombrados: `import { hello } from "modulo"`

> [!IMPORTANT]
> Los nombres de los módulos se buscan en: `./nombre.chp` → `./nombre/main.chp` → `./chpm_modules/nombre.chp` → `./chpm_modules/nombre/main.chp`

- **Extensión** – `.chp`
- **Versión** – `chipojo -v` muestra el ASCII art

## Compilar

### Linux / macOS

```bash
git clone <repo>
cd Chipojo
make
```

El ejecutable `chipojo` se genera en la raíz del proyecto.

> [!TIP]
> Para usar `chipojo` desde cualquier terminal, agrega la carpeta al `PATH`:
>
> ```bash
> echo 'export PATH=$PATH:'$(pwd) >> ~/.bashrc
> source ~/.bashrc
> ```

### Windows

Opción A — **MinGW / MSYS2:**

```bash
cd Chipojo
gcc -Ichipojo-interpreter/include chipojo-interpreter/src/*.c -o chipojo.exe
```

Opción B — **WSL (Windows Subsystem for Linux):**

```bash
# Instala WSL con Ubuntu, luego:
cd Chipojo
make
./chipojo script.chp
```

> [!NOTE]
> El `Makefile` usa `gcc` y está pensado para Linux/WSL. En Windows nativo usa MinGW o compila con WSL.

## Usar

```bash
./chipojo script.chp
./chipojo -v          # versión
```

## Ejemplo

```chipojo
nombre = "Chipojo"
tamano = 10

if (tamano >= 10) {
    show("Bienvenido a " + nombre + "!")
} else {
    show("muy pequeño")
}

show("2 + 2 = " + (2 + 2))
```

```text
Bienvenido a Chipojo!
2 + 2 = 4
```

## Switch

```chipojo
switch (valor) {
    case 1 { show("uno") }
    case 2 { show("dos") }
    default { show("otro") }
}
```

## Clases

```chipojo
class Persona {
    public nombre
    public edad
    private id

    void init(nombre, edad) {
        this.nombre = nombre
        this.edad = edad
    }

    func saludar() {
        show("Hola, soy " + this.nombre)
    }
}

p = Persona("Ana", 25)
p.saludar()
```

## Arrow Functions

```chipojo
doble = x => x * 2
suma = (a, b) => a + b
```

## Listas

```chipojo
nums = [1, 2, 3]
nums.push(4)
show(nums.size())     // 4
show(nums.contains(2)) // 1
nums.reverse()
show(nums)            // [4, 3, 2, 1]
```

## Diccionarios

```chipojo
persona = {"nombre": "Ana", "edad": 30}
show(persona.nombre)    // "Ana"
persona.edad = 31
show(persona.has("edad")) // 1
```

## Errores

```chipojo
try {
    throw "error!"
} catch (err) {
    show("Capturado: " + err)
}
```

## Extensión VS Code

La extensión **Chipojo Language** (`vscode-extension/`) aporta:

- Resaltado de sintaxis (keywords, strings, números, operadores, métodos)
- Snippets de código (`if`, `while`, `for`, `switch`, `class`, `func`, `try`/`catch`, `import`, arrow functions, etc.)
- Tema de iconos Sybo (archivos `.chp` con icono personalizado)
- Configuración de lenguaje (bracket matching, auto-cierre, comentarios)

Instálalo desde el `.vsix` generado en `vscode-extension/` o desde el marketplace de VS Code.

## Módulos y Closures

Chipojo soporta módulos con `import`/`export`. Las funciones exportadas **capturan el ámbito del módulo** (closures), permitiendo acceder a variables privadas del módulo:

```chipojo
// modulo.chp
privado = 42

export func obtener() {
    return privado  // captura 'privado' del módulo
}
```

```chipojo
// main.chp
import modulo
show(modulo.obtener())  // 42
```

> [!TIP]
> Las funciones flecha (`=>`) también capturan closures cuando se exportan desde un módulo.

## Tests

```bash
make test
```

Ejecuta todos los scripts `.chp` en `test/` para verificar el funcionamiento.

## Probar

Los scripts en `test/` son ejemplos prácticos para que veas cómo funciona el lenguaje y puedas experimentar.

## Estructura del proyecto

```
Chipojo/
├── chipojo-interpreter/  # Intérprete del lenguaje (C)
│   ├── src/              # Código fuente (.c)
│   ├── include/          # Cabeceras (.h)
│   └── lib/              # Librería estándar (.chp)
├── vscode-extension/     # Extensión VS Code
├── assets/               # Recursos (iconos)
├── test/                 # Scripts de prueba
├── docs/                 # Documentación
│   └── es/               # Docs en español
├── Makefile              # Compilar: make
└── LICENSE               # MIT
```

## Licencia

MIT – libre de usar y modificar.
