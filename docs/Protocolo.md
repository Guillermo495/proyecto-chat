# Protocolo para el chat

## Mensajes que recibe el servidor

# `IDENTIFY`

Identificará al usuario con el servidor, con la propiedad `username` para el
nombre del usuario. Por ejemplo:

```
{ "type": "IDENTIFY",
  "username": "Kimberly" }
```

Si la identificación se realiza exitosamente, el servidor responderá con un
mensaje `RESPONSE` con la operación `IDENTIFY`, el resultado `SUCCESS` y el
nombre de usuario como extra, por ejemplo:

```
{ "type": "RESPONSE",
  "operation": "IDENTIFY",
  "result": "SUCCESS",
  "extra": "Kimberly" }
```

Además se enviará a los demás clientes el mensaje `NEW_USER` con el nombre del
nuevo usuario, por ejemplo:

```
{ "type": "NEW_USER",
  "username": "Kimberly" }
```

Si el nombre de usuario ya está siendo usado el servidor responderá con un
resultado `USER_ALREADY_EXISTS`, por ejemplo:

```
{ "type": "RESPONSE",
  "operation": "IDENTIFY",
  "result": "USER_ALREADY_EXISTS",
  "extra": "Kimberly" }
```

# `STATUS`

Cambia el estado de un usuario:

```
{ "type": "STATUS",
  "status": "AWAY" }
```

Si el estado es uno de los tres estados válidos (`ACTIVE`, `AWAY` o `BUSY`) y es
distinto al que el usuario tiene al momento, el servidor manda el mensaje
`NEW_STATUS` a los demás clientes conectados:

```
{ "type": "NEW_STATUS",
  "username": "Kimberly",
  "status": "AWAY" }
```

# `USERS`

Solicitará la lista de usuarios en el chat:

```
{ "type": "USERS" }
```

El servidor responderá al cliente con mensaje `USER_LIST` con la propiedad
`users`, que será un objeto con los nombres de usuario y sus estados:

```
{ "type": "USER_LIST",
  "users": { "Kimberly": "ACTIVE",
             "Luis": "BUSY",
             "Fernando": "AWAY",
             "Antonio": "ACTIVE" } }
```

# `TEXT`

Mandará un texto privado a un usuario:

```
{ "type": "TEXT",
  "username": "Luis",
  "text": "Hola Luis, ¿cómo estás?" }
```

Si el usuario destinatario existe el servidor no responderá nada y enviará el
mensaje `TEXT_FROM` al usuario:

```
{ "type": "TEXT_FROM",
  "username": "Kimberly",
  "text": "Hola Luis, ¿cómo estás?" }
```

Si el usuario destinatario no existe, el servidor responderá:

```
{ "type": "RESPONSE",
  "operation": "TEXT",
  "result": "NO_SUCH_USER",
  "extra": "Luis" }
```

# `PUBLIC_TEXT`

Mandará un texto público a todos los usuarios conectados:

```
{ "type": "PUBLIC_TEXT",
  "text": "¡Hola a todos!" }
```

El servidor no responderá nada y se enviará el mensaje `PUBLIC_TEXT_FROM` a los
demás usuarios en el chat:

```
{ "type": "PUBLIC_TEXT_FROM",
  "username": "Kimberly",
  "text": "¡Hola todos!" }
```

# `NEW_ROOM`

Creará una nueva sala en el chat:

```
{ "type": "NEW_ROOM",
  "roomname": "Sala 1" }
```

Si la sala se crea exitosamente el servidor responderá:

```
{ "type": "RESPONSE",
  "operation": "NEW_ROOM",
  "result": "SUCCESS",
  "extra": "Sala 1" }
```

Además, el usuario que crea la sala será el primero y único en la misma
inmediatamente después.

Si el nombre de la sala ya existe, el servidor responderá:

```
{ "type": "RESPONSE",
  "operation": "NEW_ROOM",
  "result": "ROOM_ALREADY_EXISTS",
  "extra": "Sala 1" }
```

# `INVITE`

Invitará a uno o múltiples usuarios a una sala; únicamente usuarios en una sala
pueden invitar a otros usuarios a esa sala:

```
{ "type": "INVITE",
  "roomname": "Sala 1",
  "usernames": [ "Luis", "Antonio", "Fernando" ] }
```

La sala y todos los usuarios deben existir, en cuyo caso el servidor no
responderá nada y enviará el mensaje `INVITATION` a cada usuario en la lista:

```
{ "type": "INVITATION",
  "username" "Kimberly",
  "roomname": "Sala 1" }
```

Si la sala no existe, el servidor responderá:

```
{ "type": "RESPONSE",
  "operation": "INVITE",
  "result": "NO_SUCH_ROOM",
  "extra": "Sala 1" }
```

Si uno o más de los usuarios no existe, al detectar el primero el servidor
responderá:

```
{ "type": "RESPONSE",
  "operation": "INVITE",
  "result": "NO_SUCH_USER",
  "extra": "Fernando" }
```

Si un usuario ya está en la sala o ya se le había invitado, ese usuario se
ignora y no se le enviará el mensaje `INVITATION`.

# `JOIN_ROOM`

El usuario se unirá a una sala; el usuario debió previamente ser invitado a la
misma para poder unirse:

```
{ "type": "JOIN_ROOM",
  "roomname": "Sala 1" }
```

Si la sala existe y el usuario fue invitado previamente a la misma, el servidor
responderá:

```
{ "type": "RESPONSE",
  "operation": "JOIN_ROOM",
  "result": "SUCCESS",
  "extra": "Sala 1" }
```

Además el usuario se unirá a la sala y el servidor enviará el mensaje
`JOINED_ROOM` a todos los usuarios en la sala:

```
{ "type": "JOINED_ROOM",
  "roomname": "Sala 1",
  "username": "Fernando" }
```

Si la sala no existe el servidor responderá:

```
{ "type": "RESPONSE",
  "operation": "JOIN_ROOM",
  "result": "NO_SUCH_ROOM",
  "extra": "Sala 1" }
```

Si el usuario no fue invitado previamente al cuarto, el servidor responderá:

```
{ "type": "RESPONSE",
  "operation": "JOIN_ROOM",
  "result": "NOT_INVITED",
  "extra": "Sala 1" }
```

# `ROOM_USERS`

```
{ "type": "ROOM_USERS",
  "roomname": "Sala 1" }
```

Si la sala existe y el usuario se ha unido a la misma, el servidor responderá
con un diccionario con los usuarios y su estado:

```
{ "type": "ROOM_USER_LIST",
  "roomname": "Sala 1",
  "users": { "Kimberly": "ACTIVE",
             "Luis": "AWAY",
             "Antonio": "BUSY",
             "Fernando": "ACTIVE" } }
```

Si la sala no existe el servidor responderá:

```
{ "type": "RESPONSE",
  "operation": "ROOM_USERS",
  "result": "NO_SUCH_ROOM",
  "extra": "Sala 1" }
```

Si la sala existe pero el usuario no ha sido invitado, o ha sido invitado pero
no se ha unido, el servidor responderá:

```
{ "type": "RESPONSE",
  "operation": "ROOM_USERS",
  "result": "NOT_JOINED",
  "extra": "Sala 1" }
```

# `ROOM_TEXT`

Manda un text a un cuarto.

```
{ "type": "ROOM_TEXT",
  "roomname": "Sala 1",
  "text": "¡Hola sala 1!" }
```

Si la sala existe y el usuario se ha unido a la misma, el servidor no responderá
nada y enviará el mensaje `ROOM_TEXT_FROM` a todos los demás usuarios en el
cuarto:

```
{ "type": "ROOM_TEXT_FROM",
  "roomname": "Sala 1",
  "username": "Kimberly",
  "text": "¡Hola sala 1!" }
```

Si la sala no existe el servidor responderá:

```
{ "type": "RESPONSE",
  "operation": "ROOM_TEXT",
  "result": "NO_SUCH_ROOM",
  "extra": "Sala 1" }
```

Si la sala existe pero el usuario no ha sido invitado, o ha sido invitado pero
no se ha unido, el servidor responderá:

```
{ "type": "RESPONSE",
  "operation": "ROOM_TEXT",
  "result": "NOT_JOINED",
  "extra": "Sala 1" }
```

# `LEAVE_ROOM`

El usuario abandona un cuarto:

```
{ "type": "LEAVE_ROOM",
  "roomname": "Sala 1" }
```

Si la sala existe y el usuario se ha unido a la misma, el servidor no responderá
nada y enviará el mensaje `LEFT_ROOM` a los demás usuarios en la sala:

```
{ "type": "LEFT_ROOM",
  "roomname": "Sala 1",
  "username": "Fernando" }
```

Si la sala no existe el servidor responderá:

```
{ "type": "RESPONSE",
  "operation": "LEAVE_ROOM",
  "result": "NO_SUCH_ROOM",
  "extra": "Sala 1" }
```

Si la sala existe pero el usuario no ha sido invitado, o ha sido invitado pero
no se ha unido, el servidor responderá:

```
{ "type": "RESPONSE",
  "operation": "LEAVE_ROOM",
  "result": "NOT_JOINED",
  "extra": "Sala 1" }
```

# `DISCONNECT`

Desconecta al usuario del chat, incluyendo abandonar todos los cuartos donde se
haya unido.

```
{ "type": "DISCONNECT" }
```

El servidor no responderá nada y enviará el mensaje `DISCONNECTED` a todos los
usuarios conectados:

```
{ "type": "DISCONNECTED",
  "username": "Luis" }
```

Además, si el usuario se había unido a cuartos, enviará el mensaje `LEFT_ROOM` a
cada cuarto:

```
{ "type": "LEFT_ROOM",
  "roomname": "Sala 1",
  "username": "Fernando" }
```

## Mensajes que recibe el cliente

# `NEW_USER`

Un nuevo usario se conectó e identificó:

```
{ "type": "NEW_USER",
  "username": "Luis" }
```

# `NEW_STATUS`

Un usuario cambió su estado:

```
{ "type": "NEW_STATUS",
  "username": "Kimberly",
  "status": "AWAY" }
```

# `USER_LIST`

En respuesta a `USERS`

```
{ "type": "USER_LIST",
  "users": { "Kimberly": "ACTIVE",
             "Luis": "BUSY",
             "Fernando": "AWAY",
             "Antonio": "ACTIVE" } }
```

# `TEXT_FROM`

Recibe un texto privado:

```
{ "type": "TEXT_FROM",
  "username": "Luis",
  "text": "Hola Kim, bien ¿y tú?" }
```

# `PUBLIC_TEXT_FROM`

Recibe un texto público:

```
{ "type": "PUBLIC_TEXT_FROM",
  "username": "Kimberly",
  "text": "¡Hola todos!" }
```

# `JOINED_ROOM`

Un nuevo usuario se unió a un cuarto:

```
{ "type": "JOINED_ROOM",
  "roomname": "Sala 1",
  "username": "Fernando" }
```

# `ROOM_USER_LIST`

En respuesta a `ROOM_USERS`

```
{ "type": "ROOM_USER_LIST",
  "roomname": "Sala 1",
  "users": { "Kimberly": "ACTIVE",
             "Luis": "AWAY",
             "Antonio": "BUSY",
             "Fernando": "ACTIVE" } }
```

# `ROOM_TEXT_FROM`

Recibe un texto en un cuarto:

```
{ "type": "ROOM_TEXT_FROM",
  "roomname": "Sala 1",
  "username": "Kimberly",
  "text": "¡Bienvenidos a mi sala!" }
```

# `LEFT_ROOM`

Un usuario abandonó un cuarto:

```
{ "type": "LEFT_ROOM",
  "roomname": "Sala 1",
  "username": "Fernando" }
```

# `DISCONNECTED`

Un usuario se desconectó:

```
{ "type": "DISCONNECTED",
  "username": "Luis" }
```

## Notas

- Todo mensaje que un cliente o el servidor envíe debe terminar con el carácter
  de salto de línea: `\n`. Esto es para facilitar el procesamiento de mensajes,
  en particular en el servidor que puede recibir múltiples mensajes en una sola
  lectura del enchufe correspondiente y entonces el salto de línea funciona como
  delimitador.

- Los mensajes presentados son ejemplos; obviamente los nombres de usario, de
  cuartos y textos particulares serán distintos.

- Los nombres de usuario deben tener un límite de 8 caracteres y los nombres de
  cuartos un límite de 16 caracteres.

- Cuando todos los usuarios de una sala lo hayan abandonado, la sala
  desaparece y otro usuario debe crearla de nuevo antes de poder volver a usarse
  ese nombre.

- Un usuario siempre se conecta con el estado `ACTIVE`.

- Si un usuario no se ha identificado no puede hacer nada hasta que se
  identifique; todo mensaje distinto de `IDENTIFY` se responderárá con lo
  siguiente:

  ```
  { "type": "RESPONSE",
  "operation": "INVALID",
  "result": "NOT_IDENTIFIED" }
  ```

  Después de responder, el servidor procederá a desconectar al cliente.

- Si un mensaje es incompleto (por ejemplo, un `TEXT` que le falte la llave
  `"username"`); o falla con valores esperados (como un estado distinto de
  `ACTIVE`, `AWAY` y `BUSY`); o no se puede reconocer (en particular si no es un
  diccionario JSON con la llave `"type"`); el servidor responderá:

  ```
  { "type": "RESPONSE",
  "operation": "INVALID",
  "result": "INVALID" }
  ```

  y se desconectará al cliente.

Por legibilidad se han mostrado todos los mensajes con saltos de línea y
espacios incluidos; sin embargo, como se dijo arriba, el protocolo utilizará
salto de línea como separación entre mensajes; en otras palabras, se debe
utilizar un salto de línea para dejar claro que un mensaje ya terminó de
recibirse. Un mensaje entonces debe verse de la siguiente manera:

```
{"type":"RESPONSE","operation":"IDENTIFY","result":"SUCCESS","extra":"Kimberly"}
```

Múltiples saltos de líneas seguidos (o en otras palabras, mensajes que consistan
en una cadena vacía) deben _IGNORARSE_, tanto por el cliente como por el
servidor.

Lenguajes como C y C++ utilizan el carácter vacío (`\0`) como terminador de
cadenas; el carácter vacío no debe ser enviado como parte de ningún mensaje: lo
que delimita los mensajes es el carácter de salto de línea.

Se espera que todas las cadenas transmitidas sean UTF-8 válido en todos los
casos.
