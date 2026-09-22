#ifndef CHAT_PROTOCOLO_H
#define CHAT_PROTOCOLO_H

/*
 * Interpreta un mensaje JSON y muestra su campo "type".
 *
 * Devuelve 0 si el mensaje es un objeto con "type" de texto,
 * o -1 si no cumple esas condiciones.
 */
int protocolo_inspeccionar_mensaje(const char *mensaje);

#endif