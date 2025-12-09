# Mini-compilador proyecto final

Este proyecto es un **compilador/intérprete simple** construido con **Flex** y **Bison**, capaz de ejecutar un pequeño lenguaje tipo script que incluye variables, estructuras de control, funciones nativas (`print`, `mid`, etc.) y manejo de cadenas.  
Incluye también un ejemplo para convertir números enteros a números romanos.

---

## Requisitos del sistema

Para compilar y ejecutar este proyecto necesitas:

### **Software obligatorio**
- **GCC** (o cualquier compilador C compatible)
- **Flex** (generador de analizadores léxicos)
- **Bison** (generador de analizadores sintácticos)

### **Sistemas operativos compatibles**
- Linux (Ubuntu, Fedora, Arch…)
- macOS
- Windows mediante:
    - WSL (Windows Subsystem for Linux) — recomendado
    - MSYS2 / Cygwin

### Verificar herramientas
Puedes comprobar que todo está instalado ejecutando:

```bash
gcc --version
flex --version
bison --version
make --version

##  Instrucciones rápidas (versión mini, sin *make*)
###  Compilar manualmente
Asegúrate de tener instalado **gcc**, **flex** y **bison**.

## En la linea de comandos
flex scanner.l
bison -d parser.y
gcc -o mini main.c ast.c parser.tab.c lex.yy.c

####### Ejecutar el compilador con los numeros romanos #########
./mini numeros_romanos.mini

###### Remover los archivos generados #######
./mini numeros_romanos.mini

