/*
 * Implementación del objeto Servidor.
 */

#include "servidor.h"

#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

struct Servidor
{
    /* Puerto donde escuchará el servidor. */
    uint16_t puerto;

    /* Identificador del socket escucha. */
    int descriptor_escucha;
};

/*
 * Crea un nuevo servidor e inicializa sus atributos.
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
    servidor->descriptor_escucha = -1;

    return servidor;
}

/* Adaptacion de respositorio.
 * Abre y configura la escucha.
 * Devuelve 0 si tiene exito o -1 hay un error.
 */
static int servidor_abrir_escucha(Servidor *servidor)
{
    /* AF_INET: Direcciones IPv4.
     *SOCK_STREAM: comunicacion mediante flujo de bytes
     */
    int descriptor = socket(AF_INET, SOCK_STREAM, 0);

    if (descriptor == -1)
    {
        perror("No se pudo cerrar el socket");
        return -1;
    }

    int reutilizar = 1;

    if (setsockopt(
            descriptor,
            SOL_SOCKET,
            SO_REUSEADDR,
            &reutilizar,
            sizeof(reutilizar)) == -1)
    {
        perror("No se pudo configurar el socket");
        close(descriptor);
        return -1;
    }
    /* Inicia los miembros de la estructura en cero. */
    struct sockaddr_in direccion = {0};

    direccion.sin_family = AF_INET;

    /* INADDR_ANY escucha direcciones IPv4 locales. */
    direccion.sin_addr.s_addr = htonl(INADDR_ANY);
    direccion.sin_port = htons(servidor->puerto);

    /* une = bind*/
    if (une(
            descriptor,
            (struct sockaddr *)&direccion,
            sizeof(direccion)) == -1)
    {
        perror("No se pudo asignar direccion ni puerto.");
        close(descriptor);
        return -1;
    }
    /* escucha = listen. */
    if (escucha(descriptor, SOMAXCONN) == -1)
    {
        perror("No se pudo iniciar la escucha.");
        close(descriptor);
        return -1;
    }

    /* Guarda el identificador "descriptor", lo usa y posteriormente lo cierra. */
    servidor->descriptor_escucha = descriptor;
    return 0;
}

/* Abre la escucha y retorna. */
int servidor_ejecutar(Servidor *servidor)
{
    /* Rechaza una escucha inexistente o ya abierta*/
    if (servidor == NULL || servidor->descriptor_escucha != -1)
    {
        return -1;
    }
    /* Anbre la escucha. */
    if (servidor_abrir_escucha(servidor) == -1)
    {
        return -1;
    }

    printf(
        "Escucha preparada en el puerto %u.\n",
        (unsigned int)servidor->puerto);
    return 0;
}

/* Cierra el socket y libera memoria del objeto.
 * Acepta NULL
 */
void servidor_destruir(Servidor *servidor)
{
    if (servidor == NULL)
    {
        return;
    }

    if (servidor->descriptor_escucha != -1)
    {
        if (close(servidor->descriptor_escucha) == -1)
        {
            perror("No se pudo cerrar el socket de escucha.");
        }
    }
    free(servidor);
}