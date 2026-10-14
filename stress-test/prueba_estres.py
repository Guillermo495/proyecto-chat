#!/usr/bin/env python3
"""64 clientes TCP, 64 publicaciones cada uno y desconexión por protocolo.

Servidor ya iniciado: python3 herramientas/prueba_estres.py --puerto 1234
Servidor aislado: python3 herramientas/prueba_estres.py --servidor build/servidor_chat
Solo usa la biblioteca estándar de Python 3.11 o posterior.
"""
import argparse
import asyncio
import json
from pathlib import Path
import socket
import subprocess
import time

CLIENTES = 64
MENSAJES = 64
NOMBRES = {f"P{i:03}" for i in range(CLIENTES)}


def texto(nombre, numero):
    return f"Prueba de estrés | {nombre} | {numero:03} | áéíóú ñ 🙂"


def exigir(condicion, detalle):
    if not condicion:
        raise RuntimeError(detalle)


class Cliente:
    def __init__(self, nombre, lector, escritor):
        self.nombre = nombre
        self.lector = lector
        self.escritor = escritor
        self.recibidos = set()
        self.salidas = set()
        self.respuestas = asyncio.Queue()
        self.completos = asyncio.Event()
        self.error = None
        self.cerrando = False
        self.tarea = asyncio.create_task(self.recibir())

    async def enviar(self, objeto):
        self.escritor.write((json.dumps(objeto, ensure_ascii=False) + "\n").encode())
        await self.escritor.drain()

    async def recibir(self):
        try:
            while linea := await self.lector.readline():
                objeto = json.loads(linea)
                tipo = objeto.get("type")
                if tipo == "PUBLIC_TEXT_FROM":
                    nombre = objeto.get("username")
                    contenido = objeto.get("text")
                    exigir(nombre in NOMBRES and nombre != self.nombre,
                           f"{self.nombre}: emisor inesperado {nombre}")
                    exigir(contenido in {texto(nombre, i) for i in range(MENSAJES)},
                           f"{self.nombre}: contenido alterado {contenido!r}")
                    clave = (nombre, contenido)
                    exigir(clave not in self.recibidos,
                           f"{self.nombre}: mensaje duplicado {clave}")
                    self.recibidos.add(clave)
                    if len(self.recibidos) == (CLIENTES - 1) * MENSAJES:
                        self.completos.set()
                elif tipo == "DISCONNECTED":
                    nombre = objeto.get("username")
                    exigir(nombre not in self.salidas, "Desconexión duplicada")
                    self.salidas.add(nombre)
                elif tipo in ("RESPONSE", "USER_LIST"):
                    await self.respuestas.put(objeto)
                elif tipo != "NEW_USER":
                    raise RuntimeError(f"Evento inesperado: {objeto}")
            exigir(self.cerrando, f"{self.nombre}: el servidor cerró antes de tiempo")
        except Exception as error:
            self.error = error
            self.completos.set()
            await self.respuestas.put({"error": str(error)})

    def comprobar(self):
        if self.error:
            raise self.error

    async def lista(self):
        await self.enviar({"type": "USERS"})
        respuesta = await self.respuestas.get()
        self.comprobar()
        exigir(respuesta.get("type") == "USER_LIST", f"Lista inválida: {respuesta}")
        return respuesta["users"]

    async def desconectar(self):
        self.cerrando = True
        await self.enviar({"type": "DISCONNECT"})
        await self.tarea  # Espera EOF del servidor, no solo cierra el socket local.
        self.comprobar()
        self.escritor.close()
        await self.escritor.wait_closed()


async def ejecutar(args):
    conexiones = []
    inicio = time.perf_counter()

    async def conectar(nombre):
        lector, escritor = await asyncio.open_connection(args.direccion, args.puerto)
        cliente = Cliente(nombre, lector, escritor)
        conexiones.append(cliente)
        await cliente.enviar({"type": "IDENTIFY", "username": nombre})
        respuesta = await cliente.respuestas.get()
        exigir(respuesta.get("operation") == "IDENTIFY" and
               respuesta.get("result") == "SUCCESS", f"Identificación fallida: {respuesta}")
        return cliente

    try:
        async with asyncio.timeout(args.timeout):
            clientes = await asyncio.gather(*(conectar(n) for n in sorted(NOMBRES)))
            exigir(await clientes[0].lista() == dict.fromkeys(NOMBRES, "ACTIVE"),
                   "La lista inicial no contiene exactamente los 64 clientes activos")
            print("64 clientes identificados simultáneamente.", flush=True)

            async def publicar(cliente):
                for numero in range(MENSAJES):
                    await cliente.enviar({"type": "PUBLIC_TEXT", "text": texto(cliente.nombre, numero)})
                    await asyncio.sleep(0)  # Intercala los 64 emisores.

            comienzo_envios = time.perf_counter()
            await asyncio.gather(*(publicar(c) for c in clientes))
            await asyncio.gather(*(c.completos.wait() for c in clientes))
            for cliente in clientes:
                cliente.comprobar()
                # Barrera de lectura: también detecta entregas extras ya encoladas.
                exigir(set(await cliente.lista()) == NOMBRES, "Un usuario desapareció durante los envíos")
                exigir(len(cliente.recibidos) == 4032, "Cantidad de entregas incorrecta")
            tiempo_mensajes = time.perf_counter() - comienzo_envios
            print("4,096 publicaciones; 258,048 entregas verificadas, sin pérdidas ni duplicados.", flush=True)

            # El último de los 64 comprueba las salidas de los otros 63.
            testigo = clientes[-1]
            await asyncio.gather(*(c.desconectar() for c in clientes[:-1]))
            exigir(await testigo.lista() == {testigo.nombre: "ACTIVE"},
                   "Quedaron usuarios desconectados en la lista")
            exigir(testigo.salidas == NOMBRES - {testigo.nombre},
                   "Faltan notificaciones de desconexión")
            await testigo.desconectar()
            print("64 desconexiones confirmadas por EOF; 63 avisos comprobados por el último cliente.", flush=True)

            # USERS exige identificación: una conexión posterior debe ver solo a sí misma.
            # Reutiliza además un nombre previo para detectar registros que no se liberaron.
            verificador = await conectar(clientes[0].nombre)
            exigir(await verificador.lista() == {verificador.nombre: "ACTIVE"},
                   "La lista conserva usuarios de la prueba anterior")
            await verificador.desconectar()
            print("Sin usuarios anteriores en la lista; nombre reutilizable y servidor operativo.", flush=True)
            return {"resultado": "APROBADO", "clientes_simultaneos": 64,
                    "mensajes_por_cliente": 64, "publicaciones": 4096,
                    "entregas_verificadas": 258048, "desconexiones": 64,
                    "usuarios_anteriores_restantes": 0,
                    "segundos_mensajes": round(tiempo_mensajes, 3),
                    "segundos_total": round(time.perf_counter() - inicio, 3)}
    finally:
        for cliente in conexiones:
            cliente.escritor.close()
            if not cliente.tarea.done():
                cliente.tarea.cancel()
        await asyncio.gather(*(c.tarea for c in conexiones), return_exceptions=True)
        await asyncio.gather(*(c.escritor.wait_closed() for c in conexiones), return_exceptions=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--direccion", default="127.0.0.1")
    parser.add_argument("--puerto", type=int, default=1234)
    parser.add_argument("--timeout", type=float, default=90)
    parser.add_argument("--servidor", help="Ejecutable local; se inicia en un puerto libre y se detiene al terminar")
    parser.add_argument("--resultado", default="resultado_estres.json")
    args = parser.parse_args()
    proceso = None
    registro = None
    try:
        if args.servidor:
            args.direccion = "127.0.0.1"
            with socket.socket() as reserva:
                reserva.bind((args.direccion, 0))
                args.puerto = reserva.getsockname()[1]
            registro = open("servidor_estres.log", "w")
            proceso = subprocess.Popen([str(Path(args.servidor).resolve()), str(args.puerto)],
                                       stdout=registro, stderr=subprocess.STDOUT)
            limite = time.monotonic() + 5
            while True:
                exigir(proceso.poll() is None, "El servidor terminó al iniciar")
                try:
                    with socket.create_connection((args.direccion, args.puerto), timeout=0.1):
                        break
                except OSError:
                    exigir(time.monotonic() < limite, "El servidor no abrió la escucha")
                    time.sleep(0.02)
        resultado = asyncio.run(ejecutar(args))
        if proceso:
            exigir(proceso.poll() is None, "El servidor terminó durante la prueba")
        Path(args.resultado).write_text(json.dumps(resultado, indent=2, ensure_ascii=False) + "\n")
        print(json.dumps(resultado, indent=2, ensure_ascii=False))
    except Exception as error:
        resultado = {"resultado": "FALLO", "error": str(error) or type(error).__name__}
        Path(args.resultado).write_text(json.dumps(resultado, indent=2, ensure_ascii=False) + "\n")
        print(resultado)
        raise SystemExit(1)
    finally:
        if proceso and proceso.poll() is None:
            proceso.terminate()
            try:
                proceso.wait(timeout=3)
            except subprocess.TimeoutExpired:
                proceso.kill()
                proceso.wait()
        if registro:
            registro.close()


if __name__ == "__main__":
    main()
