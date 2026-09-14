#include "servidor.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Comprueba que se pueda crear un servidor con el puerto indicado.
 *
 * Devuelve true si la prueba pasa y false si falla.
 */
static bool prueba_crear_con_puerto(uint16_t puerto)
{
    /* Ejecuta la operación que queremos probar. */
    Servidor *servidor = servidor_crear(puerto);

    /* Comprueba el resultado esperado. */
    if (servidor == NULL)
    {
        fprintf(
            stderr,
            "FALLO: no se pudo crear el servidor con puerto %u.\n",
            (unsigned int)puerto);

        return false;
    }

    /* Cada prueba libera los objetos que crea. */
    servidor_destruir(servidor);

    return true;
}

/*
 * Comprueba que dos creaciones produzcan objetos distintos.
 *
 * Ambos objetos deben existir al mismo tiempo para comparar
 * sus direcciones antes de liberar su memoria.
 */
static bool prueba_crear_servidores_distintos(void)
{
    Servidor *primero = servidor_crear(5000);
    Servidor *segundo = servidor_crear(5001);

    if (primero == NULL || segundo == NULL)
    {
        fprintf(stderr, "FALLO: no se pudieron crear ambos servidores.\n");

        /* Nuestra función acepta NULL. */
        servidor_destruir(primero);
        servidor_destruir(segundo);

        return false;
    }

    if (primero == segundo)
    {
        fprintf(stderr, "FALLO: ambas creaciones devolvieron el mismo objeto.\n");

        /* Si son el mismo objeto, solamente lo liberamos una vez. */
        servidor_destruir(primero);

        return false;
    }

    servidor_destruir(primero);
    servidor_destruir(segundo);

    return true;
}

/*
 * Ejecuta todas las comprobaciones.
 *
 * Un resultado de fallo permite que Make detecte el problema.
 */
int main(void)
{
    int fallos = 0;

    if (!prueba_crear_con_puerto(5000))
    {
        fallos++;
    }

    if (!prueba_crear_con_puerto(1))
    {
        fallos++;
    }

    if (!prueba_crear_con_puerto(65535))
    {
        fallos++;
    }

    if (!prueba_crear_servidores_distintos())
    {
        fallos++;
    }

    if (fallos > 0)
    {
        fprintf(stderr, "Comprobaciones fallidas: %d de 4.\n", fallos);
        return EXIT_FAILURE;
    }

    printf("Las 4 comprobaciones de creacion pasaron.\n");

    return EXIT_SUCCESS;
}