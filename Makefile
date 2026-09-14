# Compilación desde la raíz: make. Pruebas: make pruebas.
CC = cc
CPPFLAGS += -Iservidor-c/include
CFLAGS += -std=c11 -Wall -Wextra -Wpedantic

DIRECTORIO_SALIDA = build
SERVIDOR = $(DIRECTORIO_SALIDA)/servidor_chat
PRUEBA = $(DIRECTORIO_SALIDA)/prueba_servidor
FUENTES_SERVIDOR = servidor-c/src/main.c servidor-c/src/servidor.c
FUENTES_PRUEBA = servidor-c/pruebas/prueba_servidor.c servidor-c/src/servidor.c
CABECERAS = servidor-c/include/servidor.h

.PHONY: todo servidor pruebas limpiar

todo: servidor

servidor: $(SERVIDOR)

$(SERVIDOR): $(FUENTES_SERVIDOR) $(CABECERAS) Makefile | $(DIRECTORIO_SALIDA)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(FUENTES_SERVIDOR) $(LDLIBS) -o $@

$(PRUEBA): $(FUENTES_PRUEBA) $(CABECERAS) Makefile | $(DIRECTORIO_SALIDA)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(FUENTES_PRUEBA) $(LDLIBS) -o $@

$(DIRECTORIO_SALIDA):
	mkdir -p $@

pruebas: $(PRUEBA)
	./$(PRUEBA)

limpiar:
	rm -rf $(DIRECTORIO_SALIDA)