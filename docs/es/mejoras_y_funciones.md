# Mejoras y Funciones en Chipojo

Este documento detalla las mejoras de robustez, nuevas características y correcciones de errores aplicadas al intérprete de Chipojo.

---

## 1. Soporte para Listas y Diccionarios Vacíos
Anteriormente, declarar una lista vacía `[]` o un diccionario vacío `{}` causaba que el parser fallara inmediatamente arrojando un error de sintaxis ("Syntax Error").
* **Mejora:** Se agregaron comprobaciones de salida inmediata en `define_list()` y `define_dict()` en `src/parser.c`. Si el siguiente token es el cierre correspondiente (`]` o `}`), el intérprete construye correctamente el objeto vacío y continúa la ejecución de manera normal.

## 2. Gestión de Memoria Recursiva (`free_value_internals`)
Se diseñó e implementó un sistema de liberación de memoria recursivo en `src/variables.c` para resolver múltiples fugas de memoria y caídas del sistema (segfaults):
* **Reasignación Segura:** Antes, reasignar una variable a otro tipo diferente (por ejemplo, cambiar de `double` a `string`) corrompía la memoria porque se ejecutaba `free()` sobre el espacio del `union` compartido sin validar el tipo original. Ahora se llama a `free_value_internals` para liberar limpiamente los datos anteriores.
* **Limpieza de Ámbitos (`pop_scope`):** Cuando una función o bloque termina, el intérprete destruye el ámbito. El recolector anterior tenía fugas y errores al liberar diccionarios. El nuevo mecanismo recursivo libera toda la memoria interna de listas, diccionarios (incluyendo claves y valores dinámicos), y los nombres de las variables duplicados en el stack.
* **Transferencia de Propiedad:** En operaciones como `list.pop()` y `list.remove()`, en lugar de realizar clones redundantes y fugar los valores originales, ahora se transfiere la propiedad del elemento directamente al llamador, evitando leaks y copias de memoria innecesarias.

## 3. Mayor Precisión Matemática (`abs` -> `fabs`)
La función matemática interna `abs` de C opera únicamente con enteros (`int`). Al usarla en variables decimales en Chipojo (como `abs(-15.45)`), el intérprete truncaba la parte decimal devolviendo un entero (`15`).
* **Mejora:** Se importó `<math.h>` en `src/native.c` y se modificó el comportamiento para usar `fabs()`, manteniendo la precisión original de coma flotante (`15.45`).

## 4. Robustez del Lexer ante Identificadores Largos
Si se definía una variable o función con un identificador de más de 63 caracteres, el lexer anterior truncaba la lectura abruptamente, y parseaba los caracteres restantes como un token separado, rompiendo la sintaxis.
* **Mejora:** El lexer en `src/lexer.c` ahora consume la totalidad de la palabra/identificador de forma segura y solo guarda los primeros 63 caracteres de forma interna, evitando corromper la cola de tokens.

---

## 5. Sistema de Exportación de Funciones entre Módulos
Se corrigió el sistema de exportación de funciones para que funcionen correctamente entre módulos:
* **Buffer Swapping:** `function_call` intercambia `input` al buffer del módulo cuando `func.buffer != NULL`, permitiendo que las funciones exportadas ejecuten su cuerpo desde el archivo original.
* **Persistencia de Buffers:** Los buffers de módulos se mantienen vivos en `module_buffers[]` para que los índices `start` de funciones sigan siendo válidos.
* **Copia Profunda de Parámetros:** `clone_value` copia profundamente el array `param` (no solo strings individuales), evitando corrupción al hacer `pop_scope`.
* **Buffer como Puntero Prestado:** `buffer` en la estructura `Function` es un puntero prestado (nunca liberado por `free_value_internals`), propiedad de `module_buffers[]`.

---

## 6. Palabras Clave de Declaración (`const`, `var`, `let`, `float`, `string`, `bool`)
Se agregaron nuevas palabras clave para la declaración de variables:

| Palabra | Comportamiento |
|---------|---------------|
| `const` | Declara una constante. Reasignar lanza error. |
| `var`   | Declaración de variable estándar (sugar syntax). |
| `let`   | Declaración de variable estándar (sugar syntax). |
| `float` | Declaración con anotación de tipo float (sugar syntax). |
| `string`| Declaración con anotación de tipo string (sugar syntax). |
| `bool`  | Declaración con anotación de tipo bool (sugar syntax). |

* **Ejemplos:**
  ```chipojo
  const PI = 3.14159
  var x = 10
  let name = "Chipojo"
  float speed = 99.5
  string greeting = "hola"
  bool flag = true
  ```
* **Inmutabilidad:** `const` marca la variable con `is_const = 1`. Cualquier reasignación (`=`, `++`, `--`, `+=`, etc.) llama a `check_not_const()` y lanza error si la variable es constante.
* **Exportación:** `export const` y `export` con typed decls funcionan combinados.

---

## 7. Arrow Functions (`=>`)
Se implementaron arrow functions con sintaxis similar a JavaScript/ECMAScript:

* **Sintaxis:**
  ```chipojo
  // Sin parámetros
  f1 = () => 42

  // Un parámetro
  f2 = (x) => x * 2

  // Múltiples parámetros
  f3 = (a, b) => a + b

  // Cuerpo con bloque y return
  f4 = (x, y) => {
      return x * y
  }
  ```
* **Detección:** `factor()` en el parser usa `scan_is_arrow()` para detectar `=>` antes de evaluar una expresión parentizada.
* **Almacenamiento:** Las arrow functions se almacenan como `VAR_FUNCTION` con el flag `is_block` indicando si el cuerpo es un bloque (`{...}`) o una expresión.
* **Ejecución:** `function_call` verifica `is_block`: si es 1 ejecuta el cuerpo con `block()`, si es 0 con `expression()`.

---

## 8. `export default`
Se agregó soporte para `export default`:

* **Sintaxis:**
  ```chipojo
  export default <expresión>
  ```
  Almacena el valor bajo la clave `"default"` en el scope, marcado como exportado.
* **En módulos:** Cuando un módulo hace `export default 42`, el valor se incluye en el diccionario del módulo bajo la clave `"default"`.
* **Acceso:** `m.default` obtiene la exportación por defecto del módulo `m`.

---

## 9. Punto de Entrada para Directorios
Cuando se pasa un directorio como argumento, Chipojo busca automáticamente `main.chp` dentro de ese directorio:

```bash
./chipojo mipaquete/
# Busca mipaquete/main.chp
```

Comportamiento similar a Java (busca `Main.java` en el directorio).

---

## 10. Exportaciones de Módulos (`export`)
Las funciones y variables ahora deben marcarse explícitamente con `export` para ser visibles desde fuera del módulo:

* **Funciones:** `export func nombre() { ... }`
* **Variables:** `export nombre = valor` o `export const nombre = valor`
* **Módulos package:** Los módulos en `packets_chpm_create/` (usados por `chpm install`) deben usar `export` o sus variables aparecerán como `null`.

---

## 11. Propiedad `.length`
Se depuró y amplió la propiedad `.length`:

* **Strings:** devuelve longitud por caracteres UTF-8, no por bytes. Ejemplo: `"niño".length` devuelve `4`.
* **Listas:** devuelve la cantidad de elementos. Ejemplo: `[1, 2, 3].length` devuelve `3`.
* **Diccionarios:** devuelve la cantidad de claves. Ejemplo: `{"a": 1, "b": 2}.length` devuelve `2`.
* **Literales:** ahora funciona directamente sobre literales y expresiones parentizadas, no solo sobre variables.

```chipojo
show("hola".length)
show([1, 2, 3].length)
show({"a": 1}.length)
```

---

## 12. Manejo de Errores `try/catch`
Se agregó un MVP de manejo de errores:

```chipojo
try {
    throw "boom"
} catch (err) {
    show(err)
}
```

* **`try { ... }`:** ejecuta un bloque protegido.
* **`catch (err) { ... }`:** recibe el mensaje del error en la variable indicada.
* **Alias temporal:** `cach` también se acepta como alias de `catch`.
* **`throw`:** lanza un error manual con el valor indicado.
* **Errores internos:** errores de runtime/sintaxis dentro del `try` saltan al `catch` en vez de cerrar el proceso.

Ejemplo capturando un error real:

```chipojo
try {
    show(no_existe)
} catch (err) {
    show(err.length > 0)
}
```

---

## 11. Detección de Arrow Function (`scan_is_arrow`)
`scan_is_arrow()` escanea el texto crudo del input (sin alterar el estado del parser) para detectar si un `(` debe tratarse como una arrow function o como una expresión parentizada:
* Escanea parámetros potenciales (identificadores separados por comas).
* Busca `)` seguido de `=>`.
* Si el patrón no coincide, la expresión se trata como una agrupación normal.

---

## 12. Manejo de Propiedades con Palabras Clave
Se permitió que palabras clave como `default` se usen como nombres de propiedad:
```chipojo
m.default  // Ahora funciona aunque 'default' sea keyword
```

---

## 13. Sistema de Paquetes y Submódulos (Como Java)
Se implementó un sistema de paquetes con estructura de directorios que permite organizar módulos en múltiples archivos:

### Convención de Paquetes por Directorio
Un paquete es un directorio que contiene un archivo `main.chp` como punto de entrada:
```
paquete/
  main.chp       # ← punto de entrada (import paquete carga esto)
  lib/
    helper.chp   # submódulo
    tools.chp    # submódulo
```

### Import con Notación de Punto (.)
```chipojo
import pkg                   # carga pkg/main.chp
import pkg.lib.helper        # carga pkg/lib/helper.chp, variable = helper
import pkg.lib.helper as h   # carga pkg/lib/helper.chp, variable = h
from pkg.lib.helper import greet  # extrae 'greet' del módulo
```

### Orden de Búsqueda
El intérprete busca módulos en este orden:
1. `./<nombre>.chp` (archivo en el directorio actual)
2. `./<nombre>/main.chp` (paquete en el directorio actual)
3. `chpm_modules/<nombre>.chp` (módulo instalado localmente)
4. `chpm_modules/<nombre>/main.chp` (paquete instalado localmente)
5. `~/chpm_modules/<nombre>.chp` (módulo global)
6. `~/chpm_modules/<nombre>/main.chp` (paquete global)

### chpm create + chpm install
```bash
# Crear un paquete instalable
chpm create mipaquete
# Estructura:
#   mipaquete/package.json
#   mipaquete/index.chp       # entry point
#   mipaquete/lib/             # submódulos

# Dentro del paquete, agregar archivos:
#   mipaquete/lib/util.chp     # submódulo
#   mipaquete/lib/helper.chp   # submódulo

# Instalar paquetes built-in
cd mi-proyecto
chpm install credit            # busca en packets_chpm_create/
chpm install math              # instala módulo built-in

# Instalar todas las dependencias
chpm install                   # lee package.json
```

### Construir Paquetes (Java-style)
Los paquetes pueden tener múltiples archivos que se importan entre sí:
```chipojo
// credit/main.chp — entry point del paquete
export makeCreditPackage = () => {
    show("Credits package created!")
}
```

```chipojo
// credit/lib/util.chp — submódulo interno
export helperFunc = () => {
    return 42
}
```

```chipojo
// En tu proyecto:
import credit                    # carga credit/main.chp
import credit.lib.util           # carga credit/lib/util.chp, variable = util
credit.makeCreditPackage()
util.helperFunc()
```

---

## 14. `default` como Nombre de Variable
La palabra clave `default` puede usarse como nombre de variable en cualquier contexto:

```chipojo
from modulo import default       # importa el export default
show(default)                    # usa 'default' como variable
mod.default                      # acceso a propiedad 'default'
```

Esto funciona porque:
- `from_stmt` acepta `TOKEN_DEFAULT` como nombre de variable importada
- `factor()` reconoce `TOKEN_DEFAULT` como referencia a variable
- `parse_postfix()` acepta `TOKEN_DEFAULT` en acceso de propiedad (`m.default`)

---

## Guía Rápida de Import/Export

### Exportar desde un módulo
```chipojo
// archivo: math.chp
export pi = 3.14159                  // exportar variable
export const NAME = "math"           // exportar constante
export func add(a, b) {              // exportar función
    return a + b
}
export default 42                     // export default
```

### Importar en otro archivo
```chipojo
// Forma 1: importar módulo completo
import math                    // variable 'math' con todo el módulo
show(math.pi)

// Forma 2: import con alias
import math as m
show(m.pi)

// Forma 3: import from (module name como string)
import math from "math"        // igual que import math

// Forma 4: extraer exports específicos
from math import pi, e
show(pi, e)

// Forma 5: export default
from math import default
show(default)

// Forma 6: submodules con punto
import pkg.lib.helper
helper.greeting()
```

---

## 15. Resolución de Rutas desde el Script
Cuando ejecutas un script con ruta absoluta:
```bash
./chipojo /ruta/completa/proyecto/index.chp
```

El intérprete automáticamente hace `chdir()` al directorio del script, por lo que las rutas relativas (`./chpm_modules/`, `./package.json`) se resuelven correctamente. Esto funciona para:
- Archivos individuales: `./chipojo /path/to/script.chp`
- Directorios: `./chipojo /path/to/project/` → busca `main.chp`
- `.` (punto): busca `package.json` → campo `"main"` → `index.chp` por defecto

---

## Archivos Modificados

| Archivo | Cambios |
|---------|---------|
| `include/chipojo.h` | Nuevos tokens: `TOKEN_CONST`, `TOKEN_VAR`, `TOKEN_LET`, `TOKEN_FLOAT_TYPE`, `TOKEN_STRING_TYPE`, `TOKEN_BOOL_TYPE`, `TOKEN_DEFAULT`, `TOKEN_ARROW` |
| `include/variables.h` | Campo `is_const` en `Value`; campo `is_block` en `func` |
| `src/lexer.c` | Reconocimiento de keywords: `const`, `var`, `let`, `float`, `string`, `bool`, `default`; token `=>` |
| `src/parser.c` | `check_not_const()`, `handle_typed_decl()`, `scan_is_arrow()`, `parse_arrow_function()`, `skip_expression_body()`; actualizaciones en `factor()`, `assignation()`, `program()`, `block()`, `function_call()`, `parse_postfix()`, `define_new_function_ex()` |
| `src/variables.c` | `clone_value` (func.is_block), `define_function` (is_block init), `variable_set` (copia is_block), `var_value_get` (copia is_block), `load_module` (export default), `free_value_internals` |
| `src/error.c` | Mappings para todos los nuevos tokens |
| `src/main.c` | Entrada por directorio (`main.chp`), `#include <sys/stat.h>` |
