# Métodos events.*

Suscríbete a eventos de WoW y recíbelos en tiempo real a través del named pipe. Soporta eventos estándar de WoW UI (`PLAYER_LOGIN`, `PLAYER_ENTERING_WORLD`, `COMBAT_LOG_EVENT_UNFILTERED`, etc.) así como eventos personalizados del SDK.

Los eventos se capturan en un búfer circular y también pueden transmitirse por el event pipe (conexión separada del named pipe).

---

## `events.subscribe`

Suscríbete a uno o más eventos de WoW.

### Parámetros

| Nombre | Tipo | Requerido | Descripción |
|---|---|---|---|
| `events` | array[string] | sí | lista de nombres de eventos de WoW a los que suscribirse |

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `ok` | boolean | siempre `true` |
| `subscribed` | integer | cantidad de eventos suscritos exitosamente |

### Ejemplo

```json
{"id": 1, "method": "events.subscribe", "params": {"events": ["PLAYER_LOGIN", "PLAYER_ENTERING_WORLD", "UNIT_HEALTH"]}}
```

```json
{"id": 1, "result": {"ok": true, "subscribed": 3}}
```

> [!NOTE]
> Suscribirse a un evento ya suscrito es idempotente.

---

## `events.unsubscribe`

Cancela la suscripción de uno o más eventos de WoW.

### Parámetros

| Nombre | Tipo | Requerido | Descripción |
|---|---|---|---|
| `events` | array[string] | sí | lista de nombres de eventos de los que desuscribirse |

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `ok` | boolean | siempre `true` |
| `unsubscribed` | integer | cantidad de eventos desuscritos |

### Ejemplo

```json
{"id": 1, "method": "events.unsubscribe", "params": {"events": ["UNIT_HEALTH"]}}
```

```json
{"id": 1, "result": {"ok": true, "unsubscribed": 1}}
```

---

## `events.list`

Lista los eventos actualmente suscritos.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `events` | array[string] | nombres de eventos actualmente suscritos |

### Ejemplo

```json
{"id": 1, "method": "events.list"}
```

```json
{"id": 1, "result": {"events": ["PLAYER_LOGIN", "PLAYER_ENTERING_WORLD"]}}
```

---

## `events.clear`

Limpia todas las suscripciones a eventos.

### Parámetros

Ninguno.

### Respuesta

```json
{ "ok": true }
```

---

## `events.getHistory`

Recupera una porción de eventos almacenados en el búfer circular.

### Parámetros

| Nombre | Tipo | Requerido | Por defecto | Descripción |
|---|---|---|---|---|
| `offset` | integer | no | 0 | posición en el búfer desde donde empezar |
| `count` | integer | no | 50 | número de entradas a devolver |

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `total` | integer | total de entradas actualmente en el búfer |
| `returned` | integer | número de entradas realmente devueltas |
| `offset` | integer | el offset usado |
| `entries` | array | objetos de entrada de evento |

**Campos de entrada:**

| Campo | Tipo | Descripción |
|---|---|---|
| `event` | string | nombre del evento de WoW |
| `args` | json | argumentos del evento (la estructura depende del evento de WoW específico) |
| `time` | integer | timestamp en milisegundos |

### Ejemplo

```json
{"id": 1, "method": "events.getHistory", "params": {"offset": 0, "count": 2}}
```

```json
{
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

> [!NOTE]
> El búfer es circular. Cuando está lleno (256 entradas por defecto), las entradas viejas se sobrescriben. Usa `events.setMaxBuffer` para configurar el tamaño.

---

## `events.clearHistory`

Limpia todo el búfer de eventos.

### Parámetros

Ninguno.

### Respuesta

```json
{ "ok": true }
```

---

## `events.setMaxBuffer`

Establece el tamaño máximo del búfer circular del historial de eventos.

### Parámetros

| Nombre | Tipo | Requerido | Por defecto | Descripción |
|---|---|---|---|---|
| `maxBuffer` | integer | no | 256 | nuevo tamaño máximo del búfer |

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `ok` | boolean | siempre `true` |
| `maxBuffer` | integer | el nuevo tamaño máximo del búfer |

> [!NOTE]
> Reducir el tamaño del búfer no elimina inmediatamente las entradas existentes. Se sobrescribirán a medida que lleguen nuevos eventos.

---

## `events.isReady`

Verifica si el sistema de hooks de eventos está inicializado y listo.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `ready` | boolean | |

---

### Tipos de Eventos Disponibles

El SDK proporciona dos categorías de eventos:

**Eventos UI de WoW** — eventos estándar del juego disparados por `FrameScript_SignalEvent`. Suscríbete a cualquier nombre de evento de WoW. Ejemplos:
- `PLAYER_LOGIN`
- `PLAYER_ENTERING_WORLD`
- `PLAYER_LEAVE_COMBAT`
- `PLAYER_DEAD`
- `COMBAT_LOG_EVENT_UNFILTERED`
- `UNIT_HEALTH`
- `UNIT_POWER_UPDATE`
- `UNIT_NAME_UPDATE`
- `UNIT_AURA`
- `ZONE_CHANGED`

**Eventos SDK Personalizados** — empujados al event pipe automáticamente sin suscripción:

| Evento | Descripción | Args |
|---|---|---|
| `client.screenChanged` | cambió la pantalla del juego | `{ "old": <int>, "new": <int> }` donde `0=UNKNOWN`, `1=LOGIN`, `2=CHARSELECT`, `3=WORLD` |
| `loading.startup` | apareció la pantalla de carga | `{}` |
| `loading.progress` | actualización de progreso de carga | `{ "progress": <float> }` |
| `loading.finished` | carga completada, mundo entrado | `{}` |

Solo estos cuatro eventos se empujan directamente al event pipe. Los eventos UI de WoW (`PLAYER_LOGIN`, `UNIT_HEALTH`, etc.) requieren suscripción explícita via `events.subscribe` antes de aparecer.

> [!TIP]
> Los eventos de carga se sondean desde memoria, no se hookean desde Lua. `loading.progress` se emite como máximo una vez cada 100ms. El valor de progreso es un float entre 0.0 y 1.0.

---

### Transmisión en Vivo de Eventos (CLI Python)

El CLI de prueba (`tests/wowsdk_cli.py`) puede conectarse a un pipe de eventos secundario en `\\.\pipe\WowGameEvent` para recibir eventos en tiempo real. Este está separado del pipe RPC principal.

Para usarlo, ejecuta el CLI y selecciona la opción **Listen live** del menú de Eventos. Este:
1. Se conecta a `\\.\pipe\WowGameEvent`
2. Transmite todos los eventos entrantes a stdout como JSON
3. Se ejecuta hasta ser interrumpido (Ctrl+C)

Útil para monitorear eventos mientras pruebas otras funcionalidades, o para construir integraciones reactivas en tiempo real.

```python
import win32pipe, win32file, json

pipe = win32file.CreateFile(
    r"\\.\pipe\WowGameEvent",
    win32file.GENERIC_READ,
    0, None, win32file.OPEN_EXISTING, 0, None
)
while True:
    _, data = win32file.ReadFile(pipe, 65536)
    print(json.loads(data.decode()))
```
