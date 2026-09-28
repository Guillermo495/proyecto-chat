using System.Text;
using System.Text.Json;


/* Construye los mensajes JSON del cliente. */
internal static class ProtocoloCliente
{
    /* Construye la solicitud de identificación. */
    public static string CrearIdentificacion(string nombre)
    {
        return JsonSerializer.Serialize(new { type = "IDENTIFY", username = nombre });
    }

    /* Construye un mensaje para todos los usuarios. */
    public static string CrearTextoPublico(string texto)
    {
        return JsonSerializer.Serialize(new
        {
            type = "PUBLIC_TEXT",
            text = texto
        });
    }

    /* Construye la solicitud de desconexión. */
    public static string CrearDesconexion()
    {
        return JsonSerializer.Serialize(new
        {
            type = "DISCONNECT"
        });
    }

    /* Convierte un mensaje del servidor en texto para mostrar. */
    public static string DescribirMensaje(string mensaje)
    {
        using JsonDocument documento = JsonDocument.Parse(mensaje);
        JsonElement objeto = documento.RootElement;

        string tipo = ObtenerTexto(objeto, "type");

        switch (tipo)
        {
            case "PUBLIC_TEXT_FROM":
                return $"{ObtenerTexto(objeto, "username")}: " +
                       ObtenerTexto(objeto, "text");

            case "TEXT_FROM":
                return $"[Privado de {ObtenerTexto(objeto, "username")}] " +
                       ObtenerTexto(objeto, "text");

            case "NEW_USER":
                return $"{ObtenerTexto(objeto, "username")} se conectó.";

            case "DISCONNECTED":
                return $"{ObtenerTexto(objeto, "username")} se desconectó.";

            case "NEW_STATUS":
                return $"{ObtenerTexto(objeto, "username")} cambió su estado a " +
                    DescribirEstado(ObtenerTexto(objeto, "status")) + ".";

            case "RESPONSE":
                return $"{ObtenerTexto(objeto, "operation")}: " +
                       ObtenerTexto(objeto, "result");
            case "USER_LIST":
                return DescribirUsuarios(objeto);

            default:
                return $"Mensaje del servidor: {mensaje}";
        }
    }

    /* Obtiene un campo de texto y comprueba que tenga el tipo correcto. */
    private static string ObtenerTexto(JsonElement objeto, string campo)
    {
        if (objeto.ValueKind != JsonValueKind.Object ||
            !objeto.TryGetProperty(campo, out JsonElement valor) ||
            valor.ValueKind != JsonValueKind.String)
        {
            throw new JsonException(
                $"El campo \"{campo}\" debe contener texto.");
        }

        return valor.GetString()!;
    }

    /* Comprueba la respuesta de IDENTIFY y devuelve su resultado. */
    public static string ObtenerResultadoIdentificacion(string mensaje)
    {
        using JsonDocument documento = JsonDocument.Parse(mensaje);
        JsonElement objeto = documento.RootElement;

        if (ObtenerTexto(objeto, "type") != "RESPONSE" ||
            ObtenerTexto(objeto, "operation") != "IDENTIFY")
        {
            throw new JsonException(
                "Se esperaba una respuesta de identificación.");
        }

        return ObtenerTexto(objeto, "result");
    }

    /* Solicita la lista de usuarios conectados. */
    public static string CrearSolicitudUsuarios()
    {
        return JsonSerializer.Serialize(new
        {
            type = "USERS"
        });
    }

    /* Construye un mensaje dirigido a un usuario. */
    public static string CrearTextoPrivado(string nombre, string texto)
    {
        return JsonSerializer.Serialize(new
        {
            type = "TEXT",
            username = nombre,
            text = texto
        });
    }

    /* Construye una solicitud de cambio de estado. */
    public static string CrearCambioEstado(string estado)
    {
        return JsonSerializer.Serialize(new
        {
            type = "STATUS",
            status = estado
        });
    }

    /* Convierte los nombres y estados recibidos en una lista legible. */
    private static string DescribirUsuarios(JsonElement objeto)
    {
        if (!objeto.TryGetProperty("users", out JsonElement usuarios) ||
            usuarios.ValueKind != JsonValueKind.Object)
        {
            throw new JsonException(
                "El campo \"users\" debe ser un objeto.");
        }

        StringBuilder texto = new StringBuilder("Usuarios conectados:");

        foreach (JsonProperty usuario in usuarios.EnumerateObject())
        {
            if (usuario.Value.ValueKind != JsonValueKind.String)
            {
                throw new JsonException(
                    "El estado de cada usuario debe contener texto.");
            }

            texto.AppendLine();
            texto.Append($"{usuario.Name}: {DescribirEstado(usuario.Value.GetString()!)}");
        }

        return texto.ToString();
    }

    /* Traduce los estados recibidos para mostrarlos al usuario. */
    private static string DescribirEstado(string estado)
    {
        return estado switch
        {
            "ACTIVE" => "Disponible",
            "AWAY" => "Ausente",
            "BUSY" => "Ocupado",
            _ => "Estado desconocido"
        };
    }

}