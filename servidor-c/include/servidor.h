#ifndef CHAT_SERVIDOR_H
#define CHAT_SERVIDOR_H

#include <stdint.h>

/* La estructura se define de forma privada en servidor.c. */
typedef struct Servidor Servidor;

/* Crea el servidor; devuelve NULL si falla. */
Servidor *servidor_crear(uint16_t puerto);

/* Abre la escucha y retorna; todavía no acepta conexiones.
 * Devuelve 0 si tiene exito.
 * Devuelve -1 si falla, recibe NULL o la escucha ya esta abierta.
 */
int servidor_ejecutar(Servidor *servidor);

/* Cierra el socket y libera memoria. Acepta NULL. */
void servidor_destruir(Servidor *servidor);

#endif