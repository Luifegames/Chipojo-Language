# Guía de Aprendizaje del Entorno de Chipojo

Esta guía te ayudará a familiarizarte con la estructura de compilación del intérprete de Chipojo y cómo utilizarlo.

## Estructura del Entorno

La estructura del código base está organizada de la siguiente manera:

* **`Makefile`**: Define las reglas para compilar el código fuente.
* **`include/`**: Contiene todos los archivos de cabecera (`.h`) con las firmas de funciones y las estructuras de datos principales.
* **`src/`**: Contiene la lógica ejecutable del intérprete:
  * `main.c`: Punto de entrada del programa. Administra los argumentos y la inicialización.
  * `io.c`: Lee los archivos `.chp` de forma binaria.
  * `lexer.c`: Analizador léxico (convierte caracteres en tokens como identificadores, números, operadores, etc.).
  * `parser.c`: Analizador sintáctico. Ejecuta la lógica gramatical construyendo las expresiones y sentencias.
  * `variables.c`: Gestión de ámbitos (scopes), tablas de símbolos y asignación/búsqueda de variables.
  * `methods.c` y `native.c`: Funciones nativas de la biblioteca estándar (como `show()`, `abs()`, etc.) y métodos de objetos (como `.push()`, `.size()`).
  * `error.c`: Formateo y reporte de errores léxicos, sintácticos y en tiempo de ejecución.

---

## Ciclo de Compilación y Ejecución

### 1. Compilación
El intérprete se compila con GCC utilizando el Makefile proporcionado.
```bash
make clean && make
```
Esto creará el ejecutable `chipojo` en la raíz de tu proyecto.

### 2. Ejecución de Archivos `.chp`
Para ejecutar un archivo con código Chipojo, pásalo como parámetro al ejecutable:
```bash
./chipojo mi_script.chp
```

### 3. Mostrar Versión
Para ver la versión actual del intérprete e información de autoría:
```bash
./chipojo -v
```

---

## Ejecución de Pruebas
Puedes escribir scripts `.chp` simples y ejecutarlos directamente. Por ejemplo, crea un archivo `hello.chp` con el siguiente contenido:

```chipojo
show("¡Hola, Chipojo!")

nums = [1, 2, 3]
show("Tamaño de la lista:", nums.size())
```

Y ejecútalo usando `./chipojo hello.chp`.
