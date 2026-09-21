#include "cliente.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

/* Espacio inicial del bufer. */
#define CAPACIDAD_INICIAL 4096

#define TAMANO_MAXIMO_MENSAJE ((size_t)1048576)

/* Manejo del caracter nulo '\0'. */
#define CAPACIDAD_MAXIMA (TAMANO_MAXIMO_MENSAJE + 1)

struct Cliente
{
    int descriptor;
    char *datos;
    size_t utilizados;
    size_t capacidad;
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
    cliente->utilizados = 0;
    cliente->capacidad = CAPACIDAD_INICIAL;

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
    free(cliente);
}