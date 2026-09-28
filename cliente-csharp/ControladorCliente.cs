/* Coordina la entrada de consola y los mensajes del servidor. */
internal static class ControladorCliente
{
    public static async Task EjecutarAsync(ConexionCliente conexion)
    {
        using CancellationTokenSource cancelacion =
            new CancellationTokenSource();

        Console.WriteLine(
        "Escribe texto para enviar un mensaje público.\n" +
        "/usuarios: consultar usuarios conectados.\n" +
        "/privado usuario mensaje: enviar un mensaje privado.\n" +
        "/estado disponible|ausente|ocupado Cambiar de estado.\n" +
         "/salir: desconectarse.");

        Task recepcion = RecibirMensajesAsync(
            conexion, cancelacion.Token);

        try
        {
            while (true)
            {
                /* La espera del teclado no detiene la recepción. */
                Task<string?> entrada =
                    Task.Run(() => Console.ReadLine());

                await Task.WhenAny(entrada, recepcion);

                if (recepcion.IsCompleted)
                {
                    /* Propaga también los posibles errores de recepción. */
                    await recepcion;
                    return;
                }

                string? texto = await entrada;

                if (texto == null || texto == "/salir")
                {
                    await conexion.EnviarAsync(
                        ProtocoloCliente.CrearDesconexion());
                    return;
                }

                if (string.IsNullOrWhiteSpace(texto))
                {
                    continue;
                }

                string? solicitud = PrepararSolicitud(texto);

                if (solicitud != null)
                {
                    await conexion.EnviarAsync(solicitud);
                }
            }
        }
        finally
        {
            /* Termina la lectura pendiente antes de cerrar la conexión. */
            cancelacion.Cancel();

            try
            {
                await recepcion;
            }
            catch (OperationCanceledException)
                when (cancelacion.IsCancellationRequested)
            {
                /* La cancelación al salir es intencional. */
            }
        }
    }

    /* Recibe y muestra mensajes mientras la conexión permanezca abierta. */
    private static async Task RecibirMensajesAsync(
        ConexionCliente conexion,
        CancellationToken cancelacion)
    {
        while (true)
        {
            string? mensaje = await conexion.RecibirAsync(cancelacion);

            if (mensaje == null)
            {
                Console.WriteLine("El servidor cerró la conexión.");
                return;
            }

            Console.WriteLine(
                ProtocoloCliente.DescribirMensaje(mensaje));
        }
    }

    /* Interpreta la entrada y construye la solicitud correspondiente. */
    private static string? PrepararSolicitud(string texto)
    {
        if (!texto.StartsWith("/"))
        {
            return ProtocoloCliente.CrearTextoPublico(texto);
        }

        /* Conserva el mensaje privado completo en la tercera parte. */
        string[] partes = texto.Split(
            ' ', 3, StringSplitOptions.RemoveEmptyEntries);

        switch (partes[0])
        {
            case "/usuarios":
                if (partes.Length != 1)
                {
                    Console.WriteLine("Uso: /usuarios");
                    return null;
                }

                return ProtocoloCliente.CrearSolicitudUsuarios();

            case "/privado":
                if (partes.Length != 3 ||
                    string.IsNullOrWhiteSpace(partes[2]))
                {
                    Console.WriteLine("Uso: /privado usuario mensaje");
                    return null;
                }

                return ProtocoloCliente.CrearTextoPrivado(
                    partes[1], partes[2]);

            case "/estado":
                {
                    if (partes.Length != 2)
                    {
                        Console.WriteLine(
                            "Uso: /estado Disponible|Ausente|Ocupado");
                        return null;
                    }

                    /* Traduce el estado escrito al valor utilizado por el protocolo. */
                    string? estado = partes[1].ToLowerInvariant() switch
                    {
                        "disponible" => "ACTIVE",
                        "ausente" => "AWAY",
                        "ocupado" => "BUSY",
                        _ => null
                    };

                    if (estado == null)
                    {
                        Console.WriteLine(
                            "Estado no disponible. Usa disponible, ausente u ocupado.");
                        return null;
                    }

                    return ProtocoloCliente.CrearCambioEstado(estado);
                }

            default:
                Console.WriteLine(
                    "Comando desconocido. Usa /usuarios, /privado, /estado o /salir.");
                return null;
        }
    }

}

