# Chat TCP: Servidor en C y Cliente en C#

Sistema de chat cliente-servidor desarrollado para el curso de Modelado y Programación.

## Estructura del repositorio

- **`servidor-c/`**: Contiene los directorios fuente en C, los directorios de cabecera y el código externo de cJSON.
- **`cliente-csharp/`**: Contiene el directorio `.csproj` y el código fuente del cliente.
- **`Makefile`**: Directorio de configuración para automatizar la compilación, pruebas y ejecución del sistema.
- **`docs/`**: Contiene la documentación del proyecto, incluyendo el reporte de diseño y desarrollo.
- **`build/`**: Directorio de salida que se genera automáticamente para alojar los ejecutables.

## Uso rápido

1. **Compilar todo:** `make`
2. **Ejecutar servidor:** `make ejecutar-servidor` (Usa el puerto 1234 por defecto)
3. **Ejecutar cliente:** `make ejecutar-cliente` (Conecta a 127.0.0.1:1234)
4. **Limpiar compilación:** `make limpiar`
