/*
 * Implementación del objeto Servidor.
 */

#include "servidor.h"

#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>

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

    /* une. */
    if (bind(
            descriptor,
            (struct sockaddr *)&direccion,
            sizeof(direccion)) == -1)
    {
        perror("No se pudo asignar direccion ni puerto.");
        close(descriptor);
        return -1;
    }
    /* escucha. */
    if (listen(descriptor, SOMAXCONN) == -1)
    {
        perror("No se pudo iniciar la escucha.");
        close(descriptor);
        return -1;
    }

    /* Guarda el identificador "descriptor", lo usa y posteriormente lo cierra. */
    servidor->descriptor_escucha = descriptor;
    return 0;
}
/* Configuracion de socket no bloqueante.
 * Regresa 0 si jala -1 si truena.
 */
static int configurar_no_bloqueante(int descriptor)
{
    int opciones = fcntl(descriptor, F_GETFL, 0);

    if (opciones == -1)
    {
        return -1;
    }

    return fcntl(descriptor, F_SETFL, opciones | O_NONBLOCK);
}

/* handle_new_connection, adaptado al proyecto, espero...*/
static int servidor_aceptar_cliente(
    int descriptor_escucha,
    fd_set *conexiones,
    int *descriptor_maximo)

{
    struct sockaddr_in direccion_cliente = {0};
    socklen_t longitud = sizeof(direccion_cliente);

    int descriptor_cliente = accept(
        descriptor_escucha,
        (struct sockaddr *)&direccion_cliente,
        &longitud);

    if (descriptor_cliente = -1)
    {
        if (errno == EINTR ||
            errno == EAGAIN ||
            errno == ECONNABORTED)
        {
            return 0;
        }
        perror("No se pudo aceptar la conexion");
        return -1;
    }

    /* FD_SET admite solo descriptores menos que  FD_SESTIZE.
     * comprueba el limite para agregar descriptor.
     */
    if (descriptor_cliente >= FD_SETSIZE)
    {
        fprintf(stderr, "El descriptor supera limite de select.\n");
        return 0;
    }

    if (configurar_no_bloqueante(descriptor_cliente) == -1)
    {
        fprintf(stderr, "No se pudo configurar la conexion.\n");
        close(descriptor_cliente);
        return 0;
    }
    FD_SET(descriptor_cliente, conexiones);

    if (descriptor_cliente > *descriptor_maximo)
    {
        *descriptor_maximo = descriptor_cliente;
    }

    pprintf("Conexion aceptad: %d. \n", descriptor_cliente);
    fflush(stdout);

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

    /* Abre la escucha. */
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