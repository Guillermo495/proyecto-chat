#ifndef CHAT_PROTOCOLO_H
#define CHAT_PROTOCOLO_H

#include <stddef.h>

/* El contenido del mensaje se consulta mediante las funciones del protocolo. */
typedef struct cJSON MensajeProtocolo;

/* Interpreta y valida el JSON una sola vez. Devuelve NULL si falla. */
MensajeProtocolo *protocolo_interpretar_mensaje(const char *mensaje);

/*
 * Devuelve un campo de texto o NULL si no existe o tiene otro tipo.
 * El texto pertenece al mensaje: no se libera ni se usa después de liberarlo.
 */
const char *protocolo_obtener_texto(
    const MensajeProtocolo *mensaje,
    const char *campo);

/* Libera el objeto y todos los campos que contiene. */
void protocolo_liberar_mensaje(MensajeProtocolo *mensaje);

/*
 * Construye una respuesta JSON.
 * Devuelve una cadena que el llamador debe liberar con free().
 */
char *protocolo_crear_respuesta(
    const char *operacion,
    const char *resultado,
    const char *extra);

/* Construye un evento. La cadena devuelta se libera con free(). */
char *protocolo_crear_evento(
    const char *tipo,
    const char *nombre,
    const char *campo,
    const char *valor);

/* Construye un evento de texto. La cadena devuelta se libera con free(). */
char *protocolo_crear_texto(
    const char *tipo,
    const char *nombre,
    const char *texto);

/* Construye USER_LIST con estado ACTIVE, como en la versión anterior. */
/* La cadena devuelta se libera con free(). */
char *protocolo_crear_lista_usuarios(
    const char *const nombres[], size_t cantidad);

#endif
