/*
 * Implementación del objeto Servidor.
 */

#include "servidor.h"
#include "cliente.h"
#include "protocolo.h"

#include <stdio.h>
#include <stdlib.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>

#include <string.h>

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

/* Adaptacion de selectserver.c de Brian Beej's.
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
        perror("No se pudo configurar el socket.");
        close(descriptor);
        return -1;
    }
    /* Inicia los miembros de la estructura en cero. */
    struct sockaddr_in direccion = {0};

    direccion.sin_family = AF_INET;

    /* INADDR_ANY escucha direcciones IPv4 locales. */
    direccion.sin_addr.s_addr = htonl(INADDR_ANY);
    direccion.sin_port = htons(servidor->puerto);

    if (bind(
            descriptor,
            (struct sockaddr *)&direccion,
            sizeof(direccion)) == -1)
    {
        perror("No se pudo asignar direccion ni el puerto.");
        close(descriptor);
        return -1;
    }

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

/* Adaptación de handle_new_connection(). */
static int servidor_aceptar_cliente(
    int descriptor_escucha,
    fd_set *conexiones,
    int *descriptor_maximo,
    Cliente *clientes[])

{
    struct sockaddr_in direccion_cliente = {0};
    socklen_t longitud = sizeof(direccion_cliente);

    int descriptor_cliente = accept(
        descriptor_escucha,
        (struct sockaddr *)&direccion_cliente,
        &longitud);

    if (descriptor_cliente == -1)
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

    /* FD_SET admite solo descriptores menos que FD_SESTIZE.
     * comprueba el limite para agregar descriptor.
     */
    if (descriptor_cliente >= FD_SETSIZE)
    {
        fprintf(stderr, "El descriptor supera limite de select.\n");
        close(descriptor_cliente);
        return 0;
    }

    if (configurar_no_bloqueante(descriptor_cliente) == -1)
    {
        fprintf(stderr, "No se pudo configurar la conexion.\n");
        close(descriptor_cliente);
        return 0;
    }

    /* Crea el cliente relacionado a la conexion aceptada. */
    Cliente *cliente = cliente_crear(descriptor_cliente);

    if (cliente == NULL)
    {
        fprintf(stderr, "No se pudo crear el Cliente.\n");
        close(descriptor_cliente);
        return 0;
    }

    /* Guarda el cliente usando el descriptor como indice. */
    clientes[descriptor_cliente] = cliente;

    FD_SET(descriptor_cliente, conexiones);

    if (descriptor_cliente > *descriptor_maximo)
    {
        *descriptor_maximo = descriptor_cliente;
    }

    printf("Conexion aceptada: %d. \n", descriptor_cliente);
    fflush(stdout);

    return 0;
}

static int servidor_procesar_datos(
    Cliente *cliente,
    const char *datos,
    size_t cantidad)
{
    if (cliente == NULL)
    {
        return -1;
    }

    size_t posicion = 0;

    while (posicion < cantidad)
    {
        const char *inicio = datos + posicion;
        size_t pendientes = cantidad - posicion;

        /* Busca el final del siguiente mensaje en esta lectura. */
        const char *salto = memchr(inicio, '\n', pendientes);

        size_t longitud_fragmento = pendientes;

        if (salto != NULL)
        {
            longitud_fragmento = (size_t)(salto - inicio);
        }

        /* El protocolo no permite bytes nulos en los mensajes. */
        if (memchr(inicio, '\0', longitud_fragmento) != NULL)
        {
            fprintf(stderr, "El mensaje contiene un byte nulo.\n");
            return -1;
        }

        if (cliente_agregar_datos(
                cliente,
                inicio,
                longitud_fragmento) == -1)
        {
            fprintf(stderr,
                    "No se pudoalmacenar el fragmento del mensaje.\n");
            return -1;
        }

        posicion += longitud_fragmento;

        /* Conserva el fragmento hasta que llegue el salto de linea. */
        if (salto == NULL)
        {
            return 0;
        }
        const char *mensaje = cliente_obtener_datos(cliente);

        /* Muestra solo los mensajes que tienen datos. */
        if (mensaje[0] != '\0')
        {
            if (protocolo_inspeccionar_mensaje(mensaje) == -1)
            {
                return -1;
            }
        }
        cliente_limpiar_datos(cliente);

        /* Avanza sobre el salto de linea encontrado. */
        posicion++;
    }
    return 0;
}

/* Adaptacion de handle_client_data.
 *Recibe bytes y detecta que el cliente se desconecto.
 */
static void servidor_recibir_datos(
    int descriptor_cliente,
    fd_set *conexiones,
    Cliente *clientes[])
{
    char datos[4096];

    ssize_t recibidos = recv(
        descriptor_cliente,
        datos,
        sizeof(datos),
        0);

    if (recibidos > 0)
    {
        if (servidor_procesar_datos(
                clientes[descriptor_cliente],
                datos,
                (size_t)recibidos) == -1)
        {
            FD_CLR(descriptor_cliente, conexiones);

            cliente_destruir(clientes[descriptor_cliente]);
            clientes[descriptor_cliente] = NULL;

            printf(
                "Cliente %d desconectado por mensaje inválido.\n",
                descriptor_cliente);
            fflush(stdout);
        }
        return;
    }

    if (recibidos == -1)
    {
        if (errno == EINTR ||
            errno == EAGAIN ||
            errno == EWOULDBLOCK)
        {
            return;
        }
        perror("No se pudiero recibir datos.");
    }
    else
    {
        printf("Conexión finalizada %d\n", descriptor_cliente); /* recv() recibe 0 cuando el cliente
         cierra su envio*/
        fflush(stdout);
    }

    FD_CLR(descriptor_cliente, conexiones);
    cliente_destruir(clientes[descriptor_cliente]);
    clientes[descriptor_cliente] = NULL;
}

/* Adaptacion selectserver.c de Brian Beej's.
 * Vigila la escucha y las conexiones de los clientes en un solo hilo.
 */
int servidor_ejecutar(Servidor *servidor)
{
    /* Rechaza una escucha inexistente o ya abierta. */
    if (servidor == NULL || servidor->descriptor_escucha != -1)
    {
        return -1;
    }

    /* Abre la escucha. */
    if (servidor_abrir_escucha(servidor) == -1)
    {
        return -1;
    }

    int descriptor_escucha = servidor->descriptor_escucha;

    if (descriptor_escucha >= FD_SETSIZE)
    {
        fprintf(stderr, "La escucha supera el límite de select.\n");
        return -1;
    }

    if (configurar_no_bloqueante(descriptor_escucha) == -1)
    {
        perror("No se pudo configurar la escucha.");
        return -1;
    }

    /* Objetos cliente ubicados por el descriptor de su conexion. */
    Cliente *clientes[FD_SETSIZE] = {0};

    /* Conjunto permanente de descriptores que vigilamos. */
    fd_set conexiones;
    FD_ZERO(&conexiones);
    FD_SET(descriptor_escucha, &conexiones);

    int descriptor_maximo = descriptor_escucha;
    int continuar = 1;

    printf(
        "Escucha preparada en el puerto %u.\n",
        (unsigned int)servidor->puerto);
    fflush(stdout);

    /* Cambio de for(;;) a while(continuar).
     * Ademas de recibir datos, permite enviar los mensajes pendientes
     * cuando el socket esta listo para escribir.
     */
    while (continuar)
    {
        fd_set preparados = conexiones;
        fd_set escritura;

        FD_ZERO(&escritura);

        /* Vigila la escritura solo cuando hay bytes pendientes. */
        for (int descriptor = 0;
             descriptor <= descriptor_maximo;
             descriptor++)
        {
            if (clientes[descriptor] != NULL &&
                cliente_tiene_salida_pendiente(clientes[descriptor]))
            {
                FD_SET(descriptor, &escritura);
            }
        }

        int resultado = select(
            descriptor_maximo + 1,
            &preparados,
            &escritura,
            NULL,
            NULL);

        if (resultado == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            perror("No se pudo esperar actividad.");
            break;
        }
        for (int descriptor = 0;
             descriptor <= descriptor_maximo;
             descriptor++)
        {
            /* La esuccha solo se atiende para aceptar conexiones. */
            if (descriptor == descriptor_escucha)
            {
                if (FD_ISSET(descriptor, &preparados))
                {
                    if (servidor_aceptar_cliente(
                            descriptor_escucha,
                            &conexiones,
                            &descriptor_maximo,
                            clientes) == -1)
                    {
                        continuar = 0;
                        break;
                    }
                }
                continue;
            }

            if (clientes[descriptor] == NULL)
            {
                continue;
            }

            if (FD_ISSET(descriptor, &preparados))
            {
                servidor_recibir_datos(
                    descriptor,
                    &conexiones,
                    clientes);
            }

            /* La recepcion pudo cerrar y destruir al cliente. */
            if (clientes[descriptor] == NULL)
            {
                continue;
            }

            if (FD_ISSET(descriptor, &escritura))
            {
                if (cliente_enviar_pendientes(
                        clientes[descriptor]) == -1)
                {
                    FD_CLR(descriptor, &conexiones);
                    cliente_destruir(clientes[descriptor]);
                    clientes[descriptor] = NULL;

                    fprintf(
                        stderr,
                        "Cliente %d desconectado por error de envío.\n",
                        descriptor);
                }
            }
        }

        /*if (descriptor == descriptor_escucha)
        {
            if (servidor_aceptar_cliente(
                    descriptor_escucha,
                    &conexiones,
                    &descriptor_maximo,
                    clientes) == -1)
            {
                continuar = 0;
                break;
            }
        }
        else
        {
            servidor_recibir_datos(descriptor, &conexiones, clientes);
        }
        */

        /* Reduce el recorrido si cerramos los descriptores mayores. */
        while (descriptor_maximo > descriptor_escucha &&
               !FD_ISSET(descriptor_maximo, &conexiones))
        {
            descriptor_maximo--;
        }
    }

    /* Cierra los clientes si se abandona el ciclo por error. */
    for (int descriptor = 0;
         descriptor <= descriptor_maximo;
         descriptor++)
    {
        if (descriptor != descriptor_escucha &&
            FD_ISSET(descriptor, &conexiones))
        {
            cliente_destruir(clientes[descriptor]);
            clientes[descriptor] = NULL;
        }
    }
    /* main llama a servidor_destruir para cerrar la escucha. */
    return -1;
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
