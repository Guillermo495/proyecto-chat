#ifndef CHAT_SERVIDOR_H
#define CHAT_SERVIDOR_H

#include <stdint.h>

/* La estructura se define de forma privada en servidor.c. */
typedef struct Servidor Servidor;

/* Crea el servidor; devuelve NULL si falla. */
Servidor *servidor_crear(uint16_t puerto);

/* Abre la escucha y maneja multiples conexiones mediante select.
 * Recibe bytes, entrega mensajes completos al controlador y detecta desconexiones.
 * Devuelve -1 si no puede continuar.
 */
int servidor_ejecutar(Servidor *servidor);

/* Cierra el socket y libera memoria. Acepta NULL. */
void servidor_destruir(Servidor *servidor);

#endif