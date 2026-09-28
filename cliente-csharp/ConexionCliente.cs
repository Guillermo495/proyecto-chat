using System.Net.Sockets;
using System.Text;

/* Administra la conexión y el intercambio de mensajes por TCP. */
internal sealed class ConexionCliente : IDisposable
{
    private readonly TcpClient cliente = new TcpClient();
    private StreamReader? lector;

    /* Conecta e inicializa la lectura de mensajes en UTF-8. */
    public async Task ConectarAsync(string servidor, int puerto)
    {
        await cliente.ConnectAsync(servidor, puerto);

        lector = new StreamReader(
            cliente.GetStream(),
            Encoding.UTF8,
            detectEncodingFromByteOrderMarks: false,
            bufferSize: 4096,
            leaveOpen: true);
    }

    /* Envía un mensaje seguido del delimitador del protocolo. */
    public async Task EnviarAsync(string mensaje)
    {
        byte[] datos = Encoding.UTF8.GetBytes(mensaje + "\n");

        await cliente.GetStream().WriteAsync(datos);
    }

    /* Lee un mensaje y permite cancelar la espera al salir. */
    public async Task<string?> RecibirAsync(
        CancellationToken cancelacion = default)
    {
        if (lector == null)
        {
            throw new InvalidOperationException(
                "Primero debe establecerse la conexión.");
        }

        return await lector.ReadLineAsync(cancelacion);
    }

    /* Libera el lector y cierra la conexión. */
    public void Dispose()
    {
        lector?.Dispose();
        cliente.Dispose();
    }
}