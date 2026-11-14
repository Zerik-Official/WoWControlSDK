# Protocolo RPC

El SDK expone todos sus métodos mediante JSON-RPC 2.0 sobre dos named pipes de Windows. Esta página describe el transporte, el sobre de petición y respuesta, el modelo de errores y los patrones de cliente que funcionan con él.

## Transporte

| Pipe | Dirección | Modo | Propósito |
| --- | --- | --- | --- |
| `\\.\pipe\WowGameCommand` | Dúplex | Mensaje | Una petición JSON-RPC y una respuesta JSON por conexión |
| `\\.\pipe\WowGameEvent` | Dúplex | Byte | Saludo de autenticación y luego flujo de eventos delimitado por saltos de línea, un cliente a la vez |

El pipe de comandos lee un mensaje de hasta **4096 bytes**, lo despacha y escribe la respuesta. Después vacía el búfer y desconecta al cliente. Esto implica:

- Abrir un handle nuevo para cada petición.
- Escribir el objeto JSON completo en una sola llamada a `WriteFile`.
- Mantener las peticiones por debajo de 4096 bytes, incluidos los fragmentos Lua grandes.
- Leer la respuesta antes de cerrar el handle.

El pipe de eventos es distinto. Es una conexión de larga duración: el cliente se autentica primero, después el SDK escribe un objeto JSON por línea y el cliente sigue leyendo hasta que el pipe se cierra.

## Autenticación

Desde el SDK 2.0 los dos pipes exigen un token de sesión, y solo aceptan conexiones locales del usuario de Windows que ejecuta el juego (los clientes remotos se rechazan y los demás usuarios no tienen acceso).

1. El launcher genera un token aleatorio de 32 a 128 caracteres hexadecimales para cada sesión de juego.
2. Se lo pasa a `WowInjector.exe` por la entrada estándar con `--token-stdin`, así nunca aparece en una línea de comandos.
3. Con el juego todavía congelado, el inyector lo registra con `auth.init`. El SDK acepta `auth.init` una sola vez, y solo del inyector que inició el juego cuando se lanzó de esa forma.
4. Cada comando lleva el token en un campo `auth` de primer nivel. Si falta o no coincide, la respuesta es `-32001 Unauthorized`.
5. Un cliente de eventos envía `{"auth":"<token>"}` seguido de un salto de línea al conectarse. El SDK responde con un evento `auth.ok` y empieza a enviar eventos, o con `auth.failed` y cierra la conexión. El saludo debe llegar en 5 segundos.

`--no-auth` registra `{"disabled": true}`, pensado para herramientas de desarrollo. Sin el inyector, el primer cliente que llame a `auth.init` configura la autenticación.

| Método | Autenticación | Descripción |
| --- | --- | --- |
| `auth.init` | No requerida | Configura la autenticación una vez. Parámetros: `{"token": "<hex>"}` o `{"disabled": true}`. Resultado: `{"ok": true, "state": "enabled"}` o `{"ok": false, "error": "<motivo>"}`. |
| `auth.status` | No requerida | Resultado: `{"state": "pending" \| "enabled" \| "disabled"}`. |

## Petición

Toda petición usa el sobre de JSON-RPC 2.0:

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "unit.get",
  "params": {
    "token": "player"
  }
}
```

| Campo | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `jsonrpc` | string | Sí | Debe ser exactamente `"2.0"`. |
| `id` | integer | Recomendado | Identificador que se devuelve en la respuesta. Usa un número nuevo para cada petición. |
| `method` | string | Sí | Nombre completo del método, como `client.ping` o `lua.evaluate`. |
| `params` | object | No | Parámetros del método. Por defecto, un objeto vacío. |
| `auth` | string | Sí, salvo `auth.*` | Token de sesión, salvo que la autenticación esté desactivada. |

Un `id` numérico se devuelve tal cual. El parser acepta identificadores de texto, pero la respuesta usa `0`, así que utiliza enteros.

## Respuesta

Una llamada correcta devuelve el resultado del handler dentro de `result`:

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "ok": true
  }
}
```

La forma de `result` depende del método. Puede ser un objeto, un arreglo, una cadena, un número, un booleano o `null`.

### Errores de protocolo

El dispatcher devuelve un objeto `error` de JSON-RPC cuando el sobre no es válido o el método no existe:

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "error": {
    "code": -32601,
    "message": "Method not found: unit.nope"
  }
}
```

| Código | Nombre | Cuándo se devuelve |
| --- | --- | --- |
| `-32700` | Parse error | El mensaje no es JSON válido. |
| `-32600` | Invalid request | Falta `jsonrpc` o `method`, o `jsonrpc` no es `"2.0"`. |
| `-32601` | Method not found | El nombre del método no está registrado. |
| `-32602` | Invalid params | Reservado por la definición del protocolo; cada handler valida sus parámetros. |
| `-32603` | Internal error | Reservado por la definición del protocolo. |
| `-32001` | Unauthorized | Falta el token `auth`, no coincide o la autenticación aún no está configurada. |

### Errores de método

La mayoría de los fallos de dominio no son errores de protocolo. El handler devuelve una respuesta JSON-RPC correcta y el error aparece dentro de `result`:

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "error": "not on login screen"
  }
}
```

Ejemplos habituales:

| Método | Condición | `result` |
| --- | --- | --- |
| `client.login` | Pantalla incorrecta o credenciales vacías | `{"error": "not on login screen"}` o `{"error": "username and password required"}` |
| `client.enterWorld` | No se encuentra el personaje | `{"error": "character not found"}` |
| `player.*` | La caché del jugador está vacía | `{"error": "player not available"}` |
| `world.getState` | La caché del mundo está vacía | `{"error": "world not available"}` |
| `window.getInfo` | No se encuentra la ventana | `{"error": "Window not found"}` |
| `lua.evaluate` | Lua lanzó un error | `{"ok": false, "error": "..."}` |

Inspecciona siempre `result` antes de leer sus campos. Que no haya un `error` de nivel superior solo significa que la petición era válida, no que la operación haya funcionado.

## Catálogo de métodos

El SDK registra 67 métodos en diez dominios.

| Dominio | Métodos | Página |
| --- | --- | --- |
| `lua.*` | 5 | [lua.*](methods/lua.md) |
| `unit.*` | 4 | [unit.*](methods/unit.md) |
| `group.*` | 2 | [group.*](methods/group.md) |
| `player.*` | 4 | [player.*](methods/player.md) |
| `world.*` | 1 | [world.*](methods/world.md) |
| `object.*` | 4 | [object.*](methods/object.md) |
| `client.*` | 17 | [client.*](methods/client.md) |
| `events.*` | 8 | [events.*](methods/events.md) |
| `dll.*` | 4 | [dll.*](methods/dll.md) |
| `window.*` | 18 | [window.*](methods/window.md) |

La [introducción a los métodos](methods/index.md) explica las convenciones comunes y enumera todos los nombres.

## Ejemplos de cliente

### Python

El helper abre una conexión nueva para cada petición, lo que coincide con el comportamiento de una petición por conexión:

```python
import json

import win32file


def rpc(method, params=None, request_id=1):
    pipe = win32file.CreateFile(
        r"\\.\pipe\WowGameCommand",
        win32file.GENERIC_READ | win32file.GENERIC_WRITE,
        0,
        None,
        win32file.OPEN_EXISTING,
        0,
        None,
    )
    try:
        request = {
            "jsonrpc": "2.0",
            "id": request_id,
            "method": method,
            "params": params or {},
        }
        win32file.WriteFile(pipe, json.dumps(request).encode("utf-8"))
        _, data = win32file.ReadFile(pipe, 65536)
        return json.loads(data.decode("utf-8").rstrip("\x00"))
    finally:
        win32file.CloseHandle(pipe)


print(rpc("client.getScreen"))
print(rpc("unit.get", {"token": "target"}))
```

### C#

`NamedPipeClientStream` se crea y se libera dentro del helper por el mismo motivo:

```csharp
using System.IO.Pipes;
using System.Text;
using System.Text.Json;

static string Rpc(string method, object? parameters = null, int id = 1)
{
    using var pipe = new NamedPipeClientStream(".", "WowGameCommand", PipeDirection.InOut);
    pipe.Connect(5000);

    var request = JsonSerializer.Serialize(new
    {
        jsonrpc = "2.0",
        id,
        method,
        @params = parameters ?? new { }
    });

    var bytes = Encoding.UTF8.GetBytes(request);
    pipe.Write(bytes, 0, bytes.Length);
    pipe.Flush();

    using var reader = new StreamReader(pipe, Encoding.UTF8);
    return reader.ReadToEnd();
}

Console.WriteLine(Rpc("client.ping"));
```

### Flujo de eventos

El pipe de eventos permanece abierto y entrega un objeto por línea:

```python
import json

import win32file

pipe = win32file.CreateFile(
    r"\\.\pipe\WowGameEvent",
    win32file.GENERIC_READ | win32file.GENERIC_WRITE,
    0,
    None,
    win32file.OPEN_EXISTING,
    0,
    None,
)

win32file.WriteFile(pipe, (json.dumps({"auth": TOKEN}) + "\n").encode("utf-8"))

buffer = ""
while True:
    _, data = win32file.ReadFile(pipe, 65536)
    buffer += data.decode("utf-8", errors="replace")
    while "\n" in buffer:
        line, buffer = buffer.split("\n", 1)
        if line.strip():
            print(json.loads(line))
```

Consulta [events.*](methods/events.md) para los nombres de eventos, sus payloads y el listener del CLI.
