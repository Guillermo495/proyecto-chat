#include "protocolo.h"
#include "cJSON.h"
#include "cliente.h"

#include <stdio.h>
#include <string.h>

#include <stdlib.h>

#define MAXIMO_CARACTERES_USUARIO 8
#define MAXIMO_CARACTERES_SALA 16

/*
 * Comprueba que un campo exista y contenga texto.
 * Devuelve 1 si cumple esas condiciones o 0 si no las cumple.
 */
static int protocolo_campo_es_texto(
    const cJSON *objeto,
    const char *campo)
{
    const cJSON *valor = cJSON_GetObjectItemCaseSensitive(
        objeto,
        campo);

    return cJSON_IsString(valor) && valor->valuestring != NULL;
}

/*
 * Comprueba que el nombre no esté vacío, tenga UTF-8 válido
 * y no supere la cantidad de caracteres permitida.
 */
static int protocolo_nombre_valido(
    const char *nombre,
    size_t maximo)
{
    if (nombre == NULL || nombre[0] == '\0')
    {
        return 0;
    }

    const unsigned char *actual = (const unsigned char *)nombre;
    size_t caracteres = 0;

    while (*actual != '\0')
    {
        size_t bytes;

        if (*actual <= 0x7F)
        {
            bytes = 1;
        }
        else if (*actual >= 0xC2 && *actual <= 0xDF)
        {
            bytes = 2;
        }
        else if (*actual >= 0xE0 && *actual <= 0xEF)
        {
            bytes = 3;
        }
        else if (*actual >= 0xF0 && *actual <= 0xF4)
        {
            bytes = 4;
        }
        else
        {
            return 0;
        }

        /* Comprueba los bytes que completan el carácter. */
        for (size_t i = 1; i < bytes; i++)
        {
            if (actual[i] < 0x80 || actual[i] > 0xBF)
            {
                return 0;
            }
        }

        /* Rechaza secuencias que no representan valores Unicode válidos. */
        if ((*actual == 0xE0 && actual[1] < 0xA0) ||
            (*actual == 0xED && actual[1] > 0x9F) ||
            (*actual == 0xF0 && actual[1] < 0x90) ||
            (*actual == 0xF4 && actual[1] > 0x8F))
        {
            return 0;
        }

        caracteres++;

        if (caracteres > maximo)
        {
            return 0;
        }

        actual += bytes;
    }

    return 1;
}

/*
 * Comprueba los campos requeridos para cada operación.
 * Todavía no verifica usuarios, salas ni permisos.
 *
 * Devuelve 1 si los campos cumplen las comprobaciones
 * o 0 si el tipo es desconocido o algún campo es incorrecto.
 */
static int protocolo_validar_campos(
    const cJSON *objeto,
    const char *tipo)
{
    if (strcmp(tipo, "IDENTIFY") == 0)
    {
        return protocolo_nombre_valido(
            protocolo_obtener_texto(objeto, "username"),
            MAXIMO_CARACTERES_USUARIO);
    }

    if (strcmp(tipo, "STATUS") == 0)
    {
        const char *estado =
            protocolo_obtener_texto(objeto, "status");

        return cliente_estado_desde_texto(estado) != NULL;
    }

    if (strcmp(tipo, "USERS") == 0 ||
        strcmp(tipo, "DISCONNECT") == 0)
    {
        return 1;
    }

    if (strcmp(tipo, "TEXT") == 0)
    {
        return protocolo_nombre_valido(
                   protocolo_obtener_texto(objeto, "username"),
                   MAXIMO_CARACTERES_USUARIO) &&
               protocolo_campo_es_texto(objeto, "text");
    }

    if (strcmp(tipo, "PUBLIC_TEXT") == 0)
    {
        return protocolo_campo_es_texto(objeto, "text");
    }

    if (strcmp(tipo, "NEW_ROOM") == 0 ||
        strcmp(tipo, "JOIN_ROOM") == 0 ||
        strcmp(tipo, "ROOM_USERS") == 0 ||
        strcmp(tipo, "LEAVE_ROOM") == 0)
    {
        return protocolo_nombre_valido(
            protocolo_obtener_texto(objeto, "roomname"),
            MAXIMO_CARACTERES_SALA);
    }

    if (strcmp(tipo, "ROOM_TEXT") == 0)
    {
        return protocolo_nombre_valido(
                   protocolo_obtener_texto(objeto, "roomname"),
                   MAXIMO_CARACTERES_SALA) &&
               protocolo_campo_es_texto(objeto, "text");
    }

    if (strcmp(tipo, "INVITE") == 0)
    {
        if (!protocolo_nombre_valido(protocolo_obtener_texto(objeto, "roomname"),
                                     MAXIMO_CARACTERES_SALA))
        {
            return 0;
        }

        const cJSON *nombres = cJSON_GetObjectItemCaseSensitive(
            objeto,
            "usernames");

        if (!cJSON_IsArray(nombres))
        {
            return 0;
        }

        /* Comprueba cada nombre incluido en la invitación. */
        const cJSON *nombre = NULL;

        cJSON_ArrayForEach(nombre, nombres)
        {
            if (!cJSON_IsString(nombre) ||
                !protocolo_nombre_valido(
                    nombre->valuestring,
                    MAXIMO_CARACTERES_USUARIO))
            {
                return 0;
            }
        }

        return 1;
    }

    /* Ninguna operación del protocolo coincide con el tipo recibido. */
    return 0;
}

/*
 * Basado en el ejemplo de lectura de campos del README de cJSON.
 * Comprueba el campo "type" en lugar del campo "name" del ejemplo.
 */
MensajeProtocolo *protocolo_interpretar_mensaje(const char *mensaje)
{
    if (mensaje == NULL)
    {
        return NULL;
    }

    /* Rechaza texto sobrante después del JSON. */
    cJSON *objeto = cJSON_ParseWithOpts(mensaje, NULL, 1);

    if (objeto == NULL)
    {
        fprintf(stderr, "No se pudo interpretar el mensaje JSON.\n");
        return NULL;
    }

    /* El protocolo requiere un objeto, no un arreglo u otro valor. */
    if (!cJSON_IsObject(objeto))
    {
        fprintf(stderr, "El mensaje debe ser un objeto JSON.\n");
        cJSON_Delete(objeto);
        return NULL;
    }

    const cJSON *tipo = cJSON_GetObjectItemCaseSensitive(
        objeto,
        "type");

    if (!cJSON_IsString(tipo) || tipo->valuestring == NULL)
    {
        fprintf(stderr, "El campo \"type\" debe contener texto.\n");
        cJSON_Delete(objeto);
        return NULL;
    }

    if (!protocolo_validar_campos(objeto, tipo->valuestring))
    {
        fprintf(
            stderr,
            "Operación desconocida o campos incorrectos.\n");

        cJSON_Delete(objeto);
        return NULL;
    }

    /* IDENTIFY requiere un nombre que no esté vacío. */
    /*if (strcmp(tipo->valuestring, "IDENTIFY") == 0 &&
        protocolo_obtener_texto(objeto, "username")[0] == '\0')
    {
        cJSON_Delete(objeto);
        return NULL;
    }*/

    printf("Campos comprobados para: %s\n", tipo->valuestring);
    fflush(stdout);
    return objeto;
}

const char *protocolo_obtener_texto(
    const MensajeProtocolo *mensaje, const char *campo)
{
    const cJSON *valor = cJSON_GetObjectItemCaseSensitive(mensaje, campo);
    return cJSON_IsString(valor) ? valor->valuestring : NULL;
}

/* Libera el objeto y todos los campos que contiene. */
void protocolo_liberar_mensaje(MensajeProtocolo *mensaje)
{
    cJSON_Delete(mensaje);
}

/* Convierte el objeto a texto y libera el objeto, incluso si falla. */
static char *protocolo_serializar(cJSON *objeto)
{
    char *texto = cJSON_PrintUnformatted(objeto);
    cJSON_Delete(objeto);
    return texto;
}

char *protocolo_crear_respuesta(
    const char *operacion,
    const char *resultado,
    const char *extra)
{
    if (operacion == NULL || resultado == NULL)
    {
        return NULL;
    }

    cJSON *objeto = cJSON_CreateObject();

    if (objeto == NULL)
    {
        return NULL;
    }

    if (cJSON_AddStringToObject(objeto, "type", "RESPONSE") == NULL ||
        cJSON_AddStringToObject(objeto, "operation", operacion) == NULL ||
        cJSON_AddStringToObject(objeto, "result", resultado) == NULL ||
        (extra != NULL &&
         cJSON_AddStringToObject(objeto, "extra", extra) == NULL))
    {
        cJSON_Delete(objeto);
        return NULL;
    }

    return protocolo_serializar(objeto);
}

/*
 * Construye un evento con el nombre del usuario y el campo indicado.
 * La cadena devuelta se libera con free().
 */
char *protocolo_crear_evento(
    const char *tipo,
    const char *nombre,
    const char *campo,
    const char *valor)
{
    if (tipo == NULL || nombre == NULL)
    {
        return NULL;
    }

    if ((campo == NULL) != (valor == NULL))
    {
        return NULL;
    }

    cJSON *objeto = cJSON_CreateObject();

    if (objeto == NULL)
    {
        return NULL;
    }

    if (cJSON_AddStringToObject(objeto, "type", tipo) == NULL ||
        cJSON_AddStringToObject(objeto, "username", nombre) == NULL ||
        (campo != NULL &&
         cJSON_AddStringToObject(objeto, campo, valor) == NULL))
    {
        cJSON_Delete(objeto);
        return NULL;
    }

    return protocolo_serializar(objeto);
}

/* Construye un evento cuyo contenido es un mensaje de texto. */
char *protocolo_crear_texto(
    const char *tipo,
    const char *nombre,
    const char *texto)
{
    return protocolo_crear_evento(tipo, nombre, "text", texto);
}

/*
 * Construye USER_LIST con el nombre y estado de cada usuario.
 * La cadena devuelta se libera con free().
 */
char *protocolo_crear_lista_usuarios(
    const UsuarioProtocolo usuarios[],
    size_t cantidad)
{
    if (usuarios == NULL && cantidad != 0)
    {
        return NULL;
    }

    cJSON *respuesta = cJSON_CreateObject();

    if (respuesta == NULL)
    {
        return NULL;
    }

    cJSON *usuarios_json =
        cJSON_AddObjectToObject(respuesta, "users");

    if (usuarios_json == NULL ||
        cJSON_AddStringToObject(
            respuesta, "type", "USER_LIST") == NULL)
    {
        cJSON_Delete(respuesta);
        return NULL;
    }

    for (size_t i = 0; i < cantidad; i++)
    {
        if (usuarios[i].nombre == NULL ||
            usuarios[i].estado == NULL ||
            cJSON_AddStringToObject(
                usuarios_json,
                usuarios[i].nombre,
                usuarios[i].estado) == NULL)
        {
            cJSON_Delete(respuesta);
            return NULL;
        }
    }

    return protocolo_serializar(respuesta);
}
