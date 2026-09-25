#ifndef CHAT_CONTROLADOR_H
#define CHAT_CONTROLADOR_H

#include "cliente.h"

/*
 * Ejecuta un mensaje completo para el cliente que lo envió.
 * Devuelve -1 si no puede continuar, o 0 si lo atendió o programó el cierre.
 * clientes contiene FD_SETSIZE posiciones, una por descriptor.
 */
int controlador_procesar_mensaje(
    Cliente *cliente, const char *mensaje, Cliente *clientes[]);

#endif
