#include "protocolo.h"
#include "cJSON.h"

#include <stdio.h>
#include <string.h>

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
        return protocolo_campo_es_texto(objeto, "username");
    }

    if (strcmp(tipo, "STATUS") == 0)
    {
        if (!protocolo_campo_es_texto(objeto, "status"))
        {
            return 0;
        }

        const cJSON *estado = cJSON_GetObjectItemCaseSensitive(
            objeto,
            "status");

        return strcmp(estado->valuestring, "ACTIVE") == 0 ||
               strcmp(estado->valuestring, "AWAY") == 0 ||
               strcmp(estado->valuestring, "BUSY") == 0;
    }

    if (strcmp(tipo, "USERS") == 0 ||
        strcmp(tipo, "DISCONNECT") == 0)
    {
        return 1;
    }

    if (strcmp(tipo, "TEXT") == 0)
    {
        return protocolo_campo_es_texto(objeto, "username") &&
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
        return protocolo_campo_es_texto(objeto, "roomname");
    }

    if (strcmp(tipo, "ROOM_TEXT") == 0)
    {
        return protocolo_campo_es_texto(objeto, "roomname") &&
               protocolo_campo_es_texto(objeto, "text");
    }

    if (strcmp(tipo, "INVITE") == 0)
    {
        if (!protocolo_campo_es_texto(objeto, "roomname"))
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
                nombre->valuestring == NULL)
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
int protocolo_inspeccionar_mensaje(const char *mensaje)
{
    if (mensaje == NULL)
    {
        return -1;
    }

    /* Rechaza texto sobrante después del JSON. */
    cJSON *objeto = cJSON_ParseWithOpts(mensaje, NULL, 1);

    if (objeto == NULL)
    {
        fprintf(stderr, "No se pudo interpretar el mensaje JSON.\n");
        return -1;
    }

    /* El protocolo requiere un objeto, no un arreglo u otro valor. */
    if (!cJSON_IsObject(objeto))
    {
        fprintf(stderr, "El mensaje debe ser un objeto JSON.\n");
        cJSON_Delete(objeto);
        return -1;
    }

    const cJSON *tipo = cJSON_GetObjectItemCaseSensitive(
        objeto,
        "type");

    if (!cJSON_IsString(tipo) || tipo->valuestring == NULL)
    {
        fprintf(stderr, "El campo \"type\" debe contener texto.\n");
        cJSON_Delete(objeto);
        return -1;
    }

    if (!protocolo_validar_campos(objeto, tipo->valuestring))
    {
        fprintf(
            stderr,
            "Operación desconocida o campos incorrectos.\n");

        cJSON_Delete(objeto);
        return -1;
    }

    printf(
        "Campos comprobados para: %s\n",
        tipo->valuestring);

    fflush(stdout);

    cJSON_Delete(objeto);
    return 0;

    printf("Campos comprobados para: %s\n",
           tipo->valuestring);

    fflush(stdout);

    /* Libera el objeto y todos los campos que contiene. */
    cJSON_Delete(objeto);

    return 0;
}