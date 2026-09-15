#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "servidor.h"

/*
 * Inicia el servidor usando el puerto indicado como argumento.
 *
 * Ejemplo:
 *   ./servidor_chat 1234
 */
int main(int cantidad_argumentos, char *argumentos[])
{
    /* El programa necesita exactamente un argumento, el puerto*/
    if (cantidad_argumentos != 2)

    /* fprintf: imprime con formato y permite elegir el flujo del destino, stderr o FILE. */
    {
        fprintf(stderr, "Uso: %s <puerto>\n", argumentos[0]);
        return EXIT_FAILURE;
    }

    /* strtol: convierte el argumento a un entero decimal. */
    char *fin; /* Usuario ingresa texto como argumento del puerto. */
    errno = 0; /* Indicador de errores se reinicia. */
    long puerto = strtol(argumentos[1], &fin, 10);

    /* Verifica que todo el argumento represente un puerto válido. */
    if (errno == ERANGE ||      /* Detecta si el numero es muy grande. */
        fin == argumentos[1] || /* Sin digitos convertidos. */
        *fin != '\0' ||         /* Detecta caracteres basura. */
        puerto < 1 ||           /* Un entero sin signo va desde 0 hasta 2^16-1 = 65535. */
        puerto > 65535)
    {
        fprintf(stderr,
                "Puerto inválido: debe ser un entero entre 1 y 65535.\n"); /* MEnsaje si el puerto ingresado no es valido. */
        return EXIT_FAILURE;
    }

    /* Crea e inicializa el objeto servidor. */
    Servidor *servidor = servidor_crear((uint16_t)puerto);

    if (servidor == NULL)
    {
        fprintf(stderr, "No se pudo crear el servidor.\n");
        return EXIT_FAILURE;
    }

    /* Ejecuta la versión actual del servidor y comprueba su resultado. */
    int resultado = servidor_ejecutar(servidor);

    if (resultado != 0)
    {
        fprintf(stderr, "El servidor terminó con un error.\n");
    }

    /* Libera los recursos incluso si la ejecución falló. */
    servidor_destruir(servidor);

    return resultado == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}