using System;
using System.Net.Sockets;
using System.Text.Json;

/* Comprueba la direccion y el puerto recibidos. */
if (args.Length != 2)
{
    Console.Error.WriteLine(
        "Uso: ClienteChat <direccion> <puerto>");
    return 1;
}

if (!int.TryParse(args[1], out int puerto) || puerto < 1 || puerto > 65535)
{
    Console.Error.WriteLine("Puerto inválido: debe ser un entero entre 1 y 65535.");
    return 1;
}

try
{
    using ConexionCliente conexion = new ConexionCliente();

    await conexion.ConectarAsync(args[0], puerto);

    Console.WriteLine($"Conectado a {args[0]}:{puerto}.");
    while (true)
    {
        Console.Write("Nombre de usuario: ");
        string? nombre = Console.ReadLine();

        /* Termina si se cierra la entrada de la consola. */
        if (nombre == null)
        {
            return 0;
        }

        if (string.IsNullOrWhiteSpace(nombre))
        {
            Console.WriteLine("Debe escribir un nombre.");
            continue;
        }

        string solicitud = ProtocoloCliente.CrearIdentificacion(nombre);
        await conexion.EnviarAsync(solicitud);

        string? respuesta = await conexion.RecibirAsync();

        if (respuesta == null)
        {
            Console.Error.WriteLine(
                "El servidor cerró la conexión sin responder.");
            return 1;
        }

        string resultado =
            ProtocoloCliente.ObtenerResultadoIdentificacion(respuesta);

        if (resultado == "SUCCESS")
        {
            Console.WriteLine($"Identificado como {nombre}.");
            break;
        }

        if (resultado == "USER_ALREADY_EXISTS")
        {
            Console.WriteLine(
                "Ese nombre ya está ocupado. Prueba con otro.");
            continue;
        }

        Console.Error.WriteLine(
            $"El servidor rechazó la identificación: {resultado}");
        return 1;
    }

    await ControladorCliente.EjecutarAsync(conexion);
}
catch (SocketException error)
{
    Console.Error.WriteLine(
        $"Error de conexión: {error.Message}");
    return 1;
}
catch (IOException error)
{
    Console.Error.WriteLine(
        $"Error al enviar o recibir datos: {error.Message}");
    return 1;
}
catch (JsonException error)
{
    Console.Error.WriteLine(
        $"Respuesta inválida del servidor: {error.Message}");
    return 1;
}
return 0;
