#ifndef CHAT_CLIENTE_H
#define CHAT_CLIENTE_H

/* Representa una conexión entre el cliente y el servidor. */
typedef struct Cliente Cliente;

/* Crea un cliente a partir de un socket conectado. */

Cliente *cliente_crear(int descriptor);

/* Libera la memoria y cierra el socket. */

void cliente_destruir(Cliente *cliente);

#endif