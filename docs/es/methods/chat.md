# Métodos chat.*

Envían mensajes de chat en cualquier canal y responden a susurros. Los mensajes se envían con el mismo paquete `CMSG_MESSAGECHAT` que construye el juego para `SendChatMessage`, y los susurros de Battle.net pasan por las funciones `BNSendWhisper` del propio cliente. No se ejecuta Lua.

Los susurros recibidos se envían como el evento [`chat.whisperReceived`](events.md#cargas-utiles-de-los-eventos-del-sdk).

## chat.send

Envía un mensaje en un canal.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `channel` | string | Sí | `SAY`, `YELL`, `EMOTE`, `PARTY`, `RAID`, `RAID_WARNING`, `BATTLEGROUND`, `GUILD`, `OFFICER`, `WHISPER`, `CHANNEL` o `BN_WHISPER`. No distingue mayúsculas |
| `target` | string | En `WHISPER`, `CHANNEL` y `BN_WHISPER` | Nombre del personaje en `WHISPER`; número (`"1"`) o nombre (`"General - Dalaran"`) del canal en `CHANNEL`; ID de presencia de Battle.net en `BN_WHISPER` |
| `text` | string | Sí | Mensaje, hasta 255 bytes una vez codificado |
| `language` | integer | No | ID de `Languages.dbc`. Por defecto, el idioma por defecto del personaje, como en el juego |

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | Siempre `true` |
| `channel` | string | Canal usado, en mayúsculas |
| `target` | string | Destino usado; en `CHANNEL`, el nombre del canal resuelto |

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "chat.send", "params": {"channel": "WHISPER", "target": "Jaina", "text": "¡Voy para allá!"}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "channel": "WHISPER", "target": "Jaina"}}
```

### Errores

| Condición | Respuesta |
| --- | --- |
| Falta `channel` | `{"error": "channel is required"}` |
| El canal no está en la lista | `{"error": "unknown channel"}` |
| `WHISPER`, `CHANNEL` o `BN_WHISPER` sin `target` | `{"error": "target is required for this channel"}` |
| `text` está vacío | `{"error": "text is empty"}` |
| `text` ocupa más de 255 bytes | `{"error": "text is longer than 255 bytes"}` |
| `PARTY` o `RAID_WARNING` fuera de un grupo | `{"error": "not in a group"}` |
| `RAID` o `BATTLEGROUND` fuera de una banda | `{"error": "not in a raid"}` |
| No estás unido al número de canal | `{"error": "channel not joined"}` |
| `SAY`, `YELL` o `EMOTE` mientras el juego no deja hablar | `{"error": "cannot speak now"}` |
| Battle.net no está conectado (siempre en servidores privados) | `{"error": "battle.net is not available"}` |
| El amigo de Battle.net está desconectado | `{"error": "battle.net friend is not online"}` |
| El jugador no está en el mundo | `{"error": "not in world"}` |

!!! note
    Como en el juego, los mensajes `PARTY`, `RAID` y `BATTLEGROUND` del líder del grupo se envían con su variante de líder. Un `|` en el texto se envía literal, así que no se pueden inyectar enlaces ni códigos de color. El servidor sigue aplicando sus propias reglas: jugadores silenciados, ignorados y límites de spam.

## chat.reply

Responde a un susurro, como pulsar **R** en el juego. Sin parámetros responde al último susurro recibido y elige `WHISPER` o `BN_WHISPER` según ese susurro.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `text` | string | Sí | Mensaje |
| `lineId` | integer | No | Responder al susurro con este `lineId` en lugar del último |
| `guid` | string | No | Responder al último susurro de este GUID de remitente |

### Respuesta

La misma que [`chat.send`](#chatsend).

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "chat.reply", "params": {"text": "Dame 5 minutos"}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "channel": "WHISPER", "target": "Jaina"}}
```

### Errores

Los errores de [`chat.send`](#chatsend), y además:

| Condición | Respuesta |
| --- | --- |
| No se ha recibido ningún susurro, o ninguno coincide con `lineId` o `guid` | `{"error": "no whisper to reply to"}` |

!!! note
    El SDK guarda su propia lista de los últimos 20 susurros en lugar de usar el destino de respuesta del juego, así que la respuesta siempre va al susurro que elegiste, aunque el marco de chat haya cambiado su destino.

## chat.getRecentWhispers

Devuelve los susurros a los que puede responder `chat.reply`, del más antiguo al más reciente.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `whispers` | array | Hasta 20 objetos de susurro |

**Campos de cada susurro:**

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `author` | string | Nombre del remitente |
| `battleNet` | boolean | Susurro de Battle.net |
| `presenceId` | integer | ID de presencia de Battle.net, `0` en susurros normales |
| `lineId` | integer | ID de la línea de chat |
| `guid` | string | GUID del remitente tal como lo envía el juego |
| `receivedAtMs` | integer | Momento de `GetTickCount64()` en que llegó |

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "chat.getRecentWhispers"}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "whispers": [
      {"author": "Jaina", "battleNet": false, "presenceId": 0, "lineId": 412, "guid": "0x0000000000001234", "receivedAtMs": 81234567}
    ]
  }
}
```
