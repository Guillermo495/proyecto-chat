/* Ejecuta las operaciones de un mensaje completo del chat. */
#include "controlador.h"
#include "protocolo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>

/* Encola una respuesta y libera el texto recibido, incluso si falla. */
static int controlador_encolar_respuesta(Cliente *cliente, char *respuesta)
{
    if (respuesta == NULL)
    {
        return -1;
    }

    int resultado = cliente_encolar_mensaje(cliente, respuesta);
    free(respuesta);
    return resultado;
}

/*
 * Guarda la respuesta de error y programa el cierre.
 * Devuelve -1 si no pudo guardar la respuesta.
 */
static int controlador_rechazar_mensaje(Cliente *cliente, const char *motivo)
{
    if (controlador_encolar_respuesta(cliente,
                                      protocolo_crear_respuesta("INVALID", motivo, NULL)) == -1)
    {
        return -1;
    }

    cliente_programar_cierre(cliente);
    return 0;
}

/* Busca el nombre entre los clientes ya identificados. */
static int controlador_nombre_ocupado(Cliente *clientes[], const char *nombre)
{
    for (int descriptor = 0; descriptor < FD_SETSIZE; descriptor++)
    {
        const char *registrado = cliente_obtener_nombre(clientes[descriptor]);
        if (registrado != NULL && strcmp(registrado, nombre) == 0)
        {
            return 1;
        }
    }
    return 0;
}

/* Devuelve la lista de usuarios identificados. */
static int controlador_responder_usuarios(Cliente *solicitante, Cliente *clientes[])
{
    const char *nombres[FD_SETSIZE];
    size_t cantidad = 0;

    for (int descriptor = 0; descriptor < FD_SETSIZE; descriptor++)
    {
        const char *nombre = cliente_obtener_nombre(clientes[descriptor]);
        if (nombre != NULL)
        {
            nombres[cantidad++] = nombre;
        }
    }

    return controlador_encolar_respuesta(solicitante,
                                         protocolo_crear_lista_usuarios(nombres, cantidad));
}

/*
 * Encola un evento para los demás clientes identificados.
 * Libera el mensaje al terminar.
 */
static int controlador_difundir_evento(
    Cliente *emisor,
    char *mensaje,
    Cliente *clientes[])
{
    if (mensaje == NULL)
    {
        return -1;
    }

    for (int descriptor = 0;
         descriptor < FD_SETSIZE;
         descriptor++)
    {
        Cliente *destinatario = clientes[descriptor];

        if (destinatario == NULL ||
            destinatario == emisor ||
            cliente_obtener_nombre(destinatario) == NULL ||
            cliente_tiene_cierre_pendiente(destinatario))
        {
            continue;
        }

        if (cliente_encolar_mensaje(destinatario, mensaje) == -1)
        {
            fprintf(
                stderr,
                "No se pudo encolar el mensaje para el cliente %d.\n",
                descriptor);
        }
    }

    free(mensaje);
    return 0;
}

/* Consulta los campos ya validados y ejecuta la operación correspondiente. */
static int controlador_ejecutar(
    Cliente *cliente, const MensajeProtocolo *mensaje, Cliente *clientes[])
{
    const char *tipo = protocolo_obtener_texto(mensaje, "type");
    if (strcmp(tipo, "IDENTIFY") == 0)
    {
        if (cliente_obtener_nombre(cliente) != NULL)
        {
            return controlador_rechazar_mensaje(cliente, "INVALID");
        }

        const char *nombre = protocolo_obtener_texto(mensaje, "username");
        const char *resultado = "USER_ALREADY_EXISTS";
        if (!controlador_nombre_ocupado(clientes, nombre))
        {
            if (cliente_identificar(cliente, nombre) == -1)
            {
                return -1;
            }
            resultado = "SUCCESS";
        }

        /* Construye la respuesta para la solicitud de identificacion. */
        return controlador_encolar_respuesta(cliente,
                                             protocolo_crear_respuesta("IDENTIFY", resultado, nombre));
    }

    if (cliente_obtener_nombre(cliente) == NULL)
    {
        return controlador_rechazar_mensaje(cliente, "NOT_IDENTIFIED");
    }

    if (strcmp(tipo, "USERS") == 0)
    {
        return controlador_responder_usuarios(cliente, clientes);
    }

    if (strcmp(tipo, "PUBLIC_TEXT") == 0)
    {
        return controlador_difundir_evento(
            cliente,
            protocolo_crear_texto(
                "PUBLIC_TEXT_FROM",
                cliente_obtener_nombre(cliente),
                protocolo_obtener_texto(mensaje, "text")),
            clientes);
    }

    if (strcmp(tipo, "STATUS") == 0)
    {
        const char *estado =
            protocolo_obtener_texto(mensaje, "status");

        /* No anuncia el estado si sigue siendo el mismo. */
        if (strcmp(cliente_obtener_estado(cliente), estado) == 0)
        {
            return 0;
        }

        if (cliente_cambiar_estado(cliente, estado) == -1)
        {
            return controlador_rechazar_mensaje(cliente, "INVALID");
        }

        return controlador_difundir_evento(
            cliente,
            protocolo_crear_evento(
                "NEW_STATUS",
                cliente_obtener_nombre(cliente),
                "status",
                cliente_obtener_estado(cliente)),
            clientes);
    }

    /* Las demás operaciones siguen pendientes, como en la versión anterior. */
    return 0;
}

int controlador_procesar_mensaje(
    Cliente *cliente, const char *mensaje, Cliente *clientes[])
{
    if (cliente == NULL || mensaje == NULL || clientes == NULL)
    {
        return -1;
    }

    MensajeProtocolo *interpretado = protocolo_interpretar_mensaje(mensaje);
    if (interpretado == NULL)
    {
        return controlador_rechazar_mensaje(cliente, "INVALID");
    }

    int resultado = controlador_ejecutar(cliente, interpretado, clientes);
    protocolo_liberar_mensaje(interpretado);
    return resultado;
}
