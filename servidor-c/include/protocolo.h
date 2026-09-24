#ifndef CHAT_PROTOCOLO_H
#define CHAT_PROTOCOLO_H

/*
 * Comprueba el mensaje. Si es IDENTIFY, entrega una copia de username.
 */
int protocolo_inspeccionar_mensaje(
    const char *mensaje,
    char **nombre_identificacion);

/*
 * Construye una respuesta JSON.
 * Devuelve una cadena que el llamador debe liberar con free().
 */
char *protocolo_crear_respuesta(
    const char *operacion,
    const char *resultado,
    const char *extra);

#endif