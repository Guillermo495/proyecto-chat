/*
 * Implementación del objeto Servidor.
 */

#include "servidor.h"

#include <stdio.h>
#include <stdlib.h>

struct Servidor
{
    /* Puerto donde escuchará el servidor. */
    uint16_t puerto;
};

/*
 * Crea un nuevo servidor.
 *
 * Reserva memoria para un objeto Servidor e inicializa su puerto.
 */
Servidor *servidor_crear(uint16_t puerto)
{
    /*
     * malloc reserva memoria suficiente para guardar un Servidor.
     * sizeof(*servidor) obtiene el tamaño de la estructura sin repetir su tipo.
     */
    Servidor *servidor = malloc(sizeof(*servidor));

    /*
     * malloc devuelve NULL cuando no puede reservar la memoria
     */
    if (servidor == NULL)
    {
        return NULL;
    }

    /* Inicializa el puerto del nuevo servidor y devolvemos su direccion. */
    servidor->puerto = puerto;

    return servidor;
}

/*
 * Ejecuta la versión inicial: imprime un mensaje sin abrir conexiones.
 *
 * Valor de retorno
 *     0 si la ejecucion termina correctamente
 *    -1 si el puntero recibido es NULL
 */
int servidor_ejecutar(Servidor *servidor)
{
    /* No es posible ejecutar un servidor inexistente. */
    if (servidor == NULL)
    {
        return -1;
    }

    /*
     * uint16_t es un entero sin signo. Se convierte a unsigned int
     * para imprimirlo de manera compatible con %u.
     */
    printf(
        "Servidor iniciado en el puerto %u.\n",
        (unsigned int)servidor->puerto);

    return 0;
}

/*
 * Destruye un servidor.
 * Libera la memoria reservada por servidor_crear().
 */
void servidor_destruir(Servidor *servidor)
{
    free(servidor);
}