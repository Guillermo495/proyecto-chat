# Compilación desde la raíz: make. Pruebas: make pruebas.
CC = cc
CPPFLAGS += -Iservidor-c/include
CFLAGS += -std=c11 -Wall -Wextra -Wpedantic

DIRECTORIO_SALIDA = build

SERVIDOR = $(DIRECTORIO_SALIDA)/servidor_chat

PROYECTO_CLIENTE = cliente-csharp/ClienteChat.csproj

PRUEBA = $(DIRECTORIO_SALIDA)/prueba_servidor

FUENTES_SERVIDOR = servidor-c/src/main.c servidor-c/src/servidor.c servidor-c/src/cliente.c

FUENTES_PRUEBA = servidor-c/pruebas/prueba_servidor.c servidor-c/src/servidor.c servidor-c/src/cliente.c

CABECERAS = servidor-c/include/servidor.h servidor-c/include/cliente.h

CPPFLAGS += -Iservidor-c/externos/cjson

FUENTES_SERVIDOR += servidor-c/externos/cjson/cJSON.c

FUENTES_PRUEBA += servidor-c/externos/cjson/cJSON.c

CABECERAS += servidor-c/externos/cjson/cJSON.h

FUENTES_SERVIDOR += servidor-c/src/protocolo.c servidor-c/src/controlador.c

FUENTES_PRUEBA += servidor-c/src/protocolo.c servidor-c/src/controlador.c

CABECERAS += servidor-c/include/protocolo.h servidor-c/include/controlador.h

FUENTES_SERVIDOR += servidor-c/src/salas.c

FUENTES_PRUEBA += servidor-c/src/salas.c

CABECERAS += servidor-c/include/salas.h

PUERTO ?= 1234

DIRECCION ?= 127.0.0.1

.PHONY: todo servidor cliente ejecutar-servidor ejecutar-cliente pruebas limpiar

todo: servidor cliente

servidor: $(SERVIDOR)

$(SERVIDOR): $(FUENTES_SERVIDOR) $(CABECERAS) Makefile | $(DIRECTORIO_SALIDA)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(FUENTES_SERVIDOR) $(LDLIBS) -o $@

cliente:
	dotnet build $(PROYECTO_CLIENTE) --configuration Release

$(PRUEBA): $(FUENTES_PRUEBA) $(CABECERAS) Makefile | $(DIRECTORIO_SALIDA)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(FUENTES_PRUEBA) $(LDLIBS) -o $@

$(DIRECTORIO_SALIDA):
	mkdir -p $@

ejecutar-servidor: servidor
	./$(SERVIDOR) $(PUERTO)

ejecutar-cliente: cliente
	dotnet run --project $(PROYECTO_CLIENTE) --configuration Release --no-build -- $(DIRECCION) $(PUERTO)

pruebas: $(PRUEBA)
	./$(PRUEBA)

limpiar:
	rm -rf $(DIRECTORIO_SALIDA) cliente-csharp/bin cliente-csharp/obj