#ifndef CHAT_CLIENTE_H
#define CHAT_CLIENTE_H

#include <stddef.h>

/* Representa una conexión entre el cliente y el servidor. */
typedef struct Cliente Cliente;

/* Crea un cliente a partir de un socket conectado. */

Cliente *cliente_crear(int descriptor);

/* Libera la memoria y cierra el socket. */

void cliente_destruir(Cliente *cliente);

/*
 * Reserva espacio suficiente para guardar la cantidad de bytes solicitada.
 * Los datos que ya están almacenados se conservan.
 *
 * Devuelve 0 si la reserva tiene éxito o -1 si ocurre un error.
 */
int cliente_reservar(Cliente *cliente, size_t bytes_necesarios);

#endif
