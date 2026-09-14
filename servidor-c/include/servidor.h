#ifndef CHAT_SERVIDOR_H
#define CHAT_SERVIDOR_H

#include <stdint.h>

/* La estructura se define de forma privada en servidor.c. */
typedef struct Servidor Servidor;

/* Crea el servidor; devuelve NULL si falla. */
Servidor *servidor_crear(uint16_t puerto);

/* Por ahora imprime un mensaje; todavía no abre conexiones.
 * Devuelve 0 al terminar correctamente y -1 si recibe NULL.
 */
int servidor_ejecutar(Servidor *servidor);

/* Libera los recursos del servidor */
void servidor_destruir(Servidor *servidor);

#endif