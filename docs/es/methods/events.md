# Métodos events.*

Permiten suscribirse a eventos de la UI de WoW y leerlos o transmitirlos a través del SDK. El sistema de eventos tiene un búfer circular de historial y un pipe de eventos dedicado en `\\.\pipe\WowGameEvent`.

Los eventos se dividen en dos grupos:

| Grupo | Entrega | Ejemplos |
| --- | --- | --- |
| Eventos de la UI de WoW | Se envían solo después de `events.subscribe` | `PLAYER_LOGIN`, `PLAYER_ENTERING_WORLD`, `UNIT_HEALTH`, `ZONE_CHANGED` |
| Eventos del SDK | Se envían automáticamente | `client.screenChanged`, `loading.*`, `friends.*`, `group.invite*`, `summon.*`, `lfg.*`, `trade.*`, `duel.*`, `battlefield.*`, `chat.whisperReceived` |

Todo evento que llega al pipe de eventos también se guarda en el búfer de historial.

## events.subscribe

Se suscribe a uno o varios eventos de WoW.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `events` | array[string] | Sí | Nombres de evento de WoW a los que suscribirse |

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | Siempre `true` |
| `subscribed` | integer | Número de nombres procesados |

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "events.subscribe", "params": {"events": ["PLAYER_LOGIN", "PLAYER_ENTERING_WORLD", "UNIT_HEALTH"]}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "subscribed": 3}}
```

!!! note
    Suscribirse a un evento ya suscrito no tiene efecto adicional.

## events.unsubscribe

Cancela la suscripción a uno o varios eventos de WoW.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `events` | array[string] | Sí | Nombres de evento de WoW cuya suscripción se cancela |

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | Siempre `true` |
| `unsubscribed` | integer | Número de nombres procesados |

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "events.unsubscribe", "params": {"events": ["UNIT_HEALTH"]}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "unsubscribed": 1}}
```

## events.list

Lista los eventos a los que se está suscrito actualmente.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `events` | array[string] | Nombres de los eventos suscritos |

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "events.list"}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"events": ["PLAYER_LOGIN", "PLAYER_ENTERING_WORLD"]}}
```

## events.clear

Elimina todas las suscripciones.

### Parámetros

Ninguno.

### Respuesta

```json
{"ok": true}
```

## events.getHistory

Devuelve una porción del búfer circular de historial.

### Parámetros

| Nombre | Tipo | Obligatorio | Por defecto | Descripción |
| --- | --- | --- | --- | --- |
| `offset` | integer | No | `0` | Posición del búfer desde la que empezar |
| `count` | integer | No | `50` | Número de entradas que se devuelven |

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `total` | integer | Entradas que hay actualmente en el búfer |
| `returned` | integer | Entradas devueltas en esta respuesta |
| `offset` | integer | Desplazamiento usado por la petición |
| `entries` | array | Objetos de entrada de evento |

**Campos de entrada:**

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `event` | string | Nombre del evento |
| `args` | any | Argumentos del evento, cuya forma depende de cada evento |
| `time` | integer | Marca de tiempo en milisegundos |

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "events.getHistory", "params": {"offset": 0, "count": 2}}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "total": 10,
    "returned": 2,
    "offset": 0,
    "entries": [
      {"event": "PLAYER_LOGIN", "args": [], "time": 1712345678000},
      {"event": "client.screenChanged", "args": {"old": 2, "new": 3}, "time": 1712345679000}
    ]
  }
}
```

!!! note
    El búfer es circular y tiene 256 entradas por defecto. Cuando se llena, la entrada más antigua se sobrescribe. Usa `events.setMaxBuffer` para cambiar el tamaño.

## events.clearHistory

Vacía el búfer de historial.

### Parámetros

Ninguno.

### Respuesta

```json
{"ok": true}
```

## events.setMaxBuffer

Define el tamaño máximo del búfer de historial.

### Parámetros

| Nombre | Tipo | Obligatorio | Por defecto | Descripción |
| --- | --- | --- | --- | --- |
| `maxBuffer` | integer | No | `256` | Nuevo tamaño máximo del búfer |

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | Siempre `true` |
| `maxBuffer` | integer | Máximo configurado |

!!! note
    Reducir el tamaño no recorta las entradas existentes de inmediato. Se van sobrescribiendo a medida que llegan eventos nuevos.

## events.isReady

Comprueba si el sistema de hooks de eventos está inicializado.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ready` | boolean | Indica si el sistema de hooks está listo |

## Cargas útiles de los eventos del SDK

| Evento | Descripción | Argumentos |
| --- | --- | --- |
| `client.screenChanged` | La pantalla glue cambió | `{"old": <int>, "new": <int>}`, donde `0` es desconocido, `1` es login, `2` es selección de personajes y `3` es mundo |
| `loading.startup` | Apareció una pantalla de carga | `{"mapId": <int>, "background": <objeto o null>}` |
| `loading.progress` | Actualización del progreso de carga | `{"progress": <float>}` entre `0.0` y `1.0` |
| `loading.finished` | La carga terminó y se entró en el mundo | `{"mapId": <int>}` |
| `loading.cancelled` | La pantalla de carga se cerró sin entrar en el mundo en 3 segundos | `{"mapId": <int>}` |
| `friends.online` | Un amigo se conectó | [Objeto de amigo](friends.md#friendslist) |
| `friends.offline` | Un amigo se desconectó | [Objeto de amigo](friends.md#friendslist) |
| `friends.statusChanged` | Un amigo conectado se puso o dejó de estar ausente o en no molestar | [Objeto de amigo](friends.md#friendslist) |
| `group.inviteReceived` | Alguien te invitó a un grupo | `{"inviterName": <string>}` |
| `group.inviteClosed` | La invitación terminó | `{"inviterName", "reason", "roles"}`; `reason` es `"accepted"`, `"declined"`, `"cancelled"` (quien invitó la retiró o caducó) o `"joined"`; `roles` son los [roles](group.md#roles) enviados al aceptar |
| `summon.requested` | Alguien te invoca | [Objeto de invocación](summon.md#summonget) sin `canTeleport` |
| `summon.closed` | La invocación se respondió o se canceló | Objeto de invocación más `reason`: `"accepted"`, `"declined"` o `"cancelled"` |
| `lfg.proposalShown` | El Buscador de mazmorras encontró grupo | [Objeto de propuesta](lfg.md#lfggetproposal) |
| `lfg.proposalUpdated` | Un miembro respondió a la propuesta | Objeto de propuesta |
| `lfg.proposalFailed` | Alguien rechazó o la propuesta caducó | Objeto de propuesta (normalmente `{"exists": false}`) |
| `lfg.proposalSucceeded` | Todos aceptaron | Objeto de propuesta |
| `trade.requested` | Alguien quiere comerciar contigo | `{"requesterName": <string>}` |
| `trade.requestClosed` | La solicitud terminó | `{"requesterName", "reason"}`; `reason` es `"accepted"`, `"declined"`, `"cancelled"` o `"closed"` |
| `trade.opened` | Se abrió la ventana de comercio | [Objeto de comercio](trade.md#tradeget) |
| `trade.updated` | Cambiaron los objetos, el dinero o las aceptaciones | Objeto de comercio más `change`: `"update"`, `"accept"`, `"playerItem"`, `"targetItem"` o `"money"` |
| `trade.closed` | Se cerró la ventana de comercio | `{"bothAccepted": <bool>}`; `true` indica que los dos jugadores habían aceptado, así que lo más probable es que el comercio se completara |
| `duel.requested` | Alguien te retó a un duelo | [Objeto de duelo](duel.md#duelget) |
| `duel.accepted` | Aceptaste el duelo | Objeto de duelo más `reason: "accepted"` |
| `duel.closed` | La solicitud o el duelo terminaron | Objeto de duelo más `reason`: `"declined"`, `"cancelled"`, `"forfeited"` o `"finished"` |
| `duel.boundsChanged` | Saliste o volviste a entrar en la zona del duelo | `{"inBounds": <bool>}` |
| `battlefield.statusChanged` | Una cola cambió de estado | [Objeto de cola](battlefield.md#battlefieldgetqueues) más `previousStatusId` |
| `battlefield.ready` | Un campo de batalla o una arena está listo para entrar | Objeto de cola |
| `chat.whisperReceived` | Recibiste un susurro | `{"battleNet", "text", "author", "language", "status", "lineId", "guid", "presenceId", "args"}`; `status` es `"AFK"`, `"DND"`, `"GM"` o vacío, `presenceId` solo tiene valor en susurros de Battle.net y `args` contiene los argumentos originales del evento |

`background` describe la imagen de la pantalla de carga según `LoadingScreens.dbc`, o es `null` cuando el mapa no tiene una propia y se usa la imagen genérica `Interface\Glues\loading`:

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `id` | integer | Id de la fila en `LoadingScreens.dbc` |
| `name` | string | Nombre interno de la pantalla de carga |
| `file` | string | Ruta de la imagen `.blp`, por ejemplo `Interface\Glues\LoadingScreens\LoadScreenNorthrend.blp` |
| `hasWideScreen` | boolean | Si existe una variante `Wide`; el cliente la muestra en resoluciones panorámicas |

Los eventos sociales, de comercio, de duelo, de colas y de susurros se construyen a partir de los eventos de WoW correspondientes (`PARTY_INVITE_REQUEST`, `CONFIRM_SUMMON`, `LFG_PROPOSAL_SHOW`, `FRIENDLIST_UPDATE`, `TRADE_*`, `DUEL_*`, `UPDATE_BATTLEFIELD_STATUS`, `CHAT_MSG_WHISPER`…) sin necesidad de suscribirse a ellos, y sus datos se leen de memoria. También se informan las respuestas dadas con los botones del propio juego, porque el SDK sigue las funciones nativas que hay detrás de esos botones. Los eventos de amigos solo comparan entradas ya conocidas en la misma sesión, así que al iniciar sesión no se informa de cada amigo conectado. Cuando el SDK cierra un diálogo del juego después de responder por ti, el evento de WoW que usa para cerrarlo no se informa como un evento nuevo del SDK.

Los eventos de carga se sondean desde memoria una vez por frame en lugar de engancharse desde Lua, y `loading.progress` se emite como máximo una vez cada 100 milisegundos. Una pantalla de carga solo se informa mientras existe la barra de carga del cliente, por lo que la pantalla que se dibuja y se destruye en el mismo frame al salir a la lista de personajes no genera eventos.

## Transmisión de eventos en vivo

Conéctate a `\\.\pipe\WowGameEvent` para recibir eventos como JSON delimitado por saltos de línea. El pipe acepta un cliente a la vez y permanece abierto hasta que el cliente se desconecta. Envía primero el saludo de autenticación; el SDK responde con `auth.ok` o `auth.failed` (consulta [autenticación](../protocol.md#autenticacion)). Con `--no-auth` no hace falta el saludo.

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

try:
    win32file.WriteFile(pipe, (json.dumps({"auth": TOKEN}) + "\n").encode("utf-8"))
    buffer = ""
    while True:
        _, data = win32file.ReadFile(pipe, 65536)
        buffer += data.decode("utf-8", errors="replace")
        while "\n" in buffer:
            line, buffer = buffer.split("\n", 1)
            if line.strip():
                print(json.loads(line))
finally:
    win32file.CloseHandle(pipe)
```

El [cliente de escritorio](../operations/cli.md) muestra la misma secuencia en su pestaña **Events**.
