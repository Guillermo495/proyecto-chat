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

/* Guarda un mensaje para enviarlo y agrega el salto de linea.
 * Recibe una cadena sin '\n'.
 */
int cliente_encolar_mensaje(Cliente *cliente, const char *mensaje);

/* Indica si aun quedan bytes por enviar. */
int cliente_tiene_salida_pendiente(const Cliente *cliente);

/* Intenta enviar los bytes pendientes sin bloquear. */
int cliente_enviar_pendientes(Cliente *cliente);

/* Marca la conexión para cerrarla después de enviar lo pendiente. */
void cliente_programar_cierre(Cliente *cliente);

/* Indica si la conexión está esperando su cierre. */
int cliente_tiene_cierre_pendiente(const Cliente *cliente);

/* Devuelve el nombre registrado o NULL si falta identificar al cliente. */
const char *cliente_obtener_nombre(const Cliente *cliente);

/* Guarda el nombre. Devuelve -1 si ya existe o falla la reserva. */
int cliente_identificar(Cliente *cliente, const char *nombre);

#endif
