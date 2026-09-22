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

/*
 * Agrega un fragmento al dinal del bufer de recepcion
 * Conserva los datos que ya estaban almacenados
 * Devuelve 0 si tiene exito o -1 si ocurre un error.
 */
int cliente_agregar_datos(
    Cliente *cliente,
    const char *datos,
    size_t cantidad);

/* Devuelve el texto almacenado en el búfer. */
const char *cliente_obtener_datos(const Cliente *cliente);

/*
 * Descarta el contenido del bufer para comenzar otro mensaje.
 * Conserva la memoria reservada.
 */
void cliente_limpiar_datos(Cliente *cliente);

#endif
