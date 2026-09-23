#include "cliente.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>

/* Espacio inicial del bufer. */
#define CAPACIDAD_INICIAL 4096

#define TAMANO_MAXIMO_MENSAJE ((size_t)1048576)

/* Manejo del caracter nulo '\0'. */
#define CAPACIDAD_MAXIMA (TAMANO_MAXIMO_MENSAJE + 1)

/* Limite de memoria para las respuestas pendientes de un cliente. */
#define MAXIMA_SALIDA_PENDIENTE ((size_t)8 * 1024 * 1024)

struct Cliente
{
    int descriptor;
    char *datos;
    size_t bytes_utilizados;
    size_t capacidad;
    char *salida;
    size_t bytes_salida;
    size_t bytes_enviados;
};

/* Crea un cliente e inicializa su búfer de recepción. */
Cliente *cliente_crear(int descriptor)
{
    if (descriptor < 0)
    {
        return NULL;
    }

    /* Reserva memoria para los atributos del cliente. */
    Cliente *cliente = malloc(sizeof(*cliente));

    if (cliente == NULL)
    {
        return NULL;
    }

    /* Reserva memoria para el búfer de recepción. */
    cliente->datos = malloc(CAPACIDAD_INICIAL);

    if (cliente->datos == NULL)
    {
        free(cliente);
        return NULL;
    }

    cliente->descriptor = descriptor;
    cliente->bytes_utilizados = 0;
    cliente->capacidad = CAPACIDAD_INICIAL;
    cliente->datos[0] = '\0';

    cliente->salida = NULL;
    cliente->bytes_salida = 0;
    cliente->bytes_enviados = 0;

    return cliente;
}

/*
 * Adaptación de add_to_pfds(), del ejemplo pollserver.c
 * de la guía de Beej.
 *
 * Beej amplía un arreglo de conexiones; aquí se aplica la misma
 * estrategia para ampliar el búfer de bytes de un cliente.
 *
 * Se agrega un límite de capacidad y se comprueba si realloc falla.
 */
int cliente_reservar(Cliente *cliente, size_t bytes_necesarios)
{
    /* Rechaza clientes inexistentes y tamanos mayores al permitido. */
    if (cliente == NULL || bytes_necesarios > CAPACIDAD_MAXIMA)
    {
        return -1;
    }

    /* No hace falta reservar mas memoria si el espacio actual alcanza. */
    if (bytes_necesarios <= cliente->capacidad)
    {
        return 0;
    }
    size_t nueva_capacidad = cliente->capacidad;

    /* Duplica la capacidad hasta obtener suficiente espacio,
     * No supera el tamano permitido.
     */
    while (nueva_capacidad < bytes_necesarios)
    {
        if (nueva_capacidad > CAPACIDAD_MAXIMA / 2)
        {
            nueva_capacidad = CAPACIDAD_MAXIMA;
        }
        else
        {
            nueva_capacidad *= 2; // ups...
        }
    }

    /*
     * Guarda el resultado en un puntero temporal para no perder
     * la memoria anterior si realloc falla.
     */
    char *nuevos_datos = realloc(
        cliente->datos,
        nueva_capacidad);

    if (nuevos_datos == NULL)
    {
        return -1;
    }

    /* Actualiza el búfer y registra su nueva capacidad. */
    cliente->datos = nuevos_datos;
    cliente->capacidad = nueva_capacidad;

    return 0;
}

/*
 * Agrega los bytes recibidos al final del búfer del cliente.
 * Amplía el espacio si hace falta, sin superar el tamaño máximo
 * permitido, y coloca '\0' después de los datos almacenados.
 *
 * Devuelve 0 si tiene éxito o -1 si ocurre un error.
 */
int cliente_agregar_datos(
    Cliente *cliente,
    const char *datos,
    size_t cantidad)
{
    if (cliente == NULL)
    {
        return -1;
    }

    /* Un fragmento vacio no cambia el contenido el bufer. */
    if (cantidad == 0)
    {
        return 0;
    }

    if (datos == 0)
    {
        return -1;
    }

    /* Comprueba el límite antes de sumar los tamaños. */
    if (cantidad > TAMANO_MAXIMO_MENSAJE - cliente->bytes_utilizados)
    {
        return -1;
    }
    size_t total = cliente->bytes_utilizados + cantidad;

    /* Deja espacio para los datos y el carácter nulo '\0'. */
    if (cliente_reservar(cliente, total + 1) == -1)
    {
        return -1;
    }

    /* Copia el fragmenteo despues de los bytes ya almacenados. */
    memcpy(
        cliente->datos + cliente->bytes_utilizados,
        datos,
        cantidad);

    /* Registra el nuevo tamaño y coloca el carácter nulo '\0'. */
    cliente->bytes_utilizados = total;
    cliente->datos[total] = '\0';

    return 0;
}

/*  */
const char *cliente_obtener_datos(const Cliente *cliente)
{
    if (cliente == NULL)
    {
        return NULL;
    }
    return cliente->datos;
}

void cliente_limpiar_datos(Cliente *cliente)
{
    if (cliente == NULL)
    {
        return;
    }

    cliente->bytes_utilizados = 0;
    cliente->datos[0] = '\0';
}

/* Cierra la conexión y libera las dos reservas de memoria. */
void cliente_destruir(Cliente *cliente)
{
    if (cliente == NULL)
    {
        return;
    }

    if (close(cliente->descriptor) == -1)
    {
        perror("No se pudo cerrar el socket del cliente");
    }

    free(cliente->datos);
    free(cliente->salida);
    free(cliente);
}

/*
 * Conserva los bytes que faltan por enviar y agrega otro mensaje.
 * El terminador '\0' no se incluye en la salida.
 */
int cliente_encolar_mensaje(Cliente *cliente, const char *mensaje)
{
    if (cliente == NULL || mensaje == NULL)
    {
        return -1;
    }

    size_t longitud = strlen(mensaje);

    if (longitud > TAMANO_MAXIMO_MENSAJE)
    {
        return -1;
    }

    size_t pendientes =
        cliente->bytes_salida - cliente->bytes_enviados;

    /* Incluye un byte para el salto de línea del protocolo. */
    size_t adicionales = longitud + 1;

    if (adicionales > MAXIMA_SALIDA_PENDIENTE - pendientes)
    {
        return -1;
    }

    size_t total = pendientes + adicionales;

    /*
     * Prepara una nueva reserva antes de modificar la salida actual.
     * Si falla, los datos anteriores permanecen intactos.
     */
    char *nueva_salida = malloc(total);

    if (nueva_salida == NULL)
    {
        return -1;
    }

    /* Conserva únicamente la parte que todavía no se ha enviado. */
    if (pendientes > 0)
    {
        memcpy(
            nueva_salida,
            cliente->salida + cliente->bytes_enviados,
            pendientes);
    }

    memcpy(
        nueva_salida + pendientes,
        mensaje,
        longitud);

    nueva_salida[total - 1] = '\n';

    free(cliente->salida);

    cliente->salida = nueva_salida;
    cliente->bytes_salida = total;
    cliente->bytes_enviados = 0;

    return 0;
}
