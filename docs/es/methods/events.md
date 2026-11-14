# Métodos events.*

Permiten suscribirse a eventos de la UI de WoW y leerlos o transmitirlos a través del SDK. El sistema de eventos tiene un búfer circular de historial y un pipe de eventos dedicado en `\\.\pipe\WowGameEvent`.

Los eventos se dividen en dos grupos:

| Grupo | Entrega | Ejemplos |
| --- | --- | --- |
| Eventos de la UI de WoW | Se envían solo después de `events.subscribe` | `PLAYER_LOGIN`, `PLAYER_ENTERING_WORLD`, `UNIT_HEALTH`, `ZONE_CHANGED` |
| Eventos del SDK | Se envían automáticamente | `client.screenChanged`, `loading.startup`, `loading.progress`, `loading.finished`, `loading.cancelled` |

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

`background` describe la imagen de la pantalla de carga según `LoadingScreens.dbc`, o es `null` cuando el mapa no tiene una propia y se usa la imagen genérica `Interface\Glues\loading`:

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `id` | integer | Id de la fila en `LoadingScreens.dbc` |
| `name` | string | Nombre interno de la pantalla de carga |
| `file` | string | Ruta de la imagen `.blp`, por ejemplo `Interface\Glues\LoadingScreens\LoadScreenNorthrend.blp` |
| `hasWideScreen` | boolean | Si existe una variante `Wide`; el cliente la muestra en resoluciones panorámicas |

Los eventos de carga se sondean desde memoria una vez por frame en lugar de engancharse desde Lua, y `loading.progress` se emite como máximo una vez cada 100 milisegundos. Una pantalla de carga solo se informa mientras existe la barra de carga del cliente, por lo que la pantalla que se dibuja y se destruye en el mismo frame al salir a la lista de personajes no genera eventos.

## Transmisión de eventos en vivo

Conéctate a `\\.\pipe\WowGameEvent` para recibir eventos como JSON delimitado por saltos de línea. El pipe acepta un cliente a la vez y permanece abierto hasta que el cliente se desconecta.

```python
import json

import win32file

pipe = win32file.CreateFile(
    r"\\.\pipe\WowGameEvent",
    win32file.GENERIC_READ,
    0,
    None,
    win32file.OPEN_EXISTING,
    0,
    None,
)

try:
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

La [herramienta CLI](../operations/cli.md) expone la misma secuencia mediante **Events → Listen live**.
