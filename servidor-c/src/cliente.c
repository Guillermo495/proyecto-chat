#include "cliente.h"

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

#define CAPACIDAD_INICIAL 4096
struct Cliente
{
    int descriptor;

    int *datos;

    size_t utilizados;

    size_t capacidad;
};
/* Espacio inicial el búfer, no es el tamaño máximo del cliente. */
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