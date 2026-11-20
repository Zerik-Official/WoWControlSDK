# Métodos player.*

Leen el estado cacheado del jugador. Estos métodos leen la caché por frame y nunca envían una petición al hilo del juego, así que responden de inmediato incluso con el cliente muy cargado.

La caché se rellena cuando el jugador entra en el mundo. Antes de eso, todos los métodos devuelven `{"error": "player not available"}`.

## player.getHealth

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `health` | integer | Salud actual |
| `maxHealth` | integer | Salud máxima |

## player.getPosition

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `position` | object | Posición en el mundo `{x, y, z}` |
| `rotation` | float | Dirección de orientación en radianes |

## player.getTarget

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `guid` | array[2] | GUID del objetivo actual `[high, low]` |
| `valid` | boolean | Indica si el GUID del objetivo no es cero |

!!! note
    Cuando no hay ningún objetivo seleccionado, la respuesta es `{"guid": [0, 0], "valid": false}`.

## player.getState

Devuelve la instantánea completa cacheada del jugador.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `guid` | array[2] | GUID del jugador `[high, low]` |
| `health` | integer | Salud actual |
| `maxHealth` | integer | Salud máxima |
| `power` | integer | Maná, ira, energía o poder rúnico actual |
| `maxPower` | integer | Poder máximo |
| `level` | integer | Nivel del jugador |
| `race` | integer | ID de raza |
| `class` | integer | ID de clase |
| `xp` | integer | Experiencia actual |
| `maxXp` | integer | Experiencia necesaria para el siguiente nivel |
| `position` | object | Posición en el mundo `{x, y, z}` |
| `rotation` | float | Dirección de orientación en radianes |
| `targetGuid` | array[2] | GUID del objetivo `[high, low]` |
| `inCombat` | boolean | Indica si el jugador está en combate |
| `dead` | boolean | Indica si el jugador está muerto |
| `ghost` | boolean | Indica si el jugador es un fantasma |
| `mounted` | boolean | Indica si el jugador va montado |
| `flying` | boolean | Indica si el jugador está volando |
| `swimming` | boolean | Indica si el jugador está nadando |
| `afk` | boolean | Indica si el jugador está ausente |
| `dnd` | boolean | Indica si el jugador está marcado como no molestar |
| `underwater` | boolean | Indica si el jugador está bajo el agua |
| `casting` | boolean | Indica si el jugador está lanzando un hechizo |
| `valid` | boolean | Indica si la entrada de caché está rellena |

### Ejemplo

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "player.getState"}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "guid": [1234, 5678],
    "health": 25000,
    "maxHealth": 25000,
    "power": 100,
    "maxPower": 100,
    "level": 80,
    "race": 1,
    "class": 2,
    "xp": 5000,
    "maxXp": 10000,
    "position": {"x": 1234.5, "y": 2345.6, "z": 300.1},
    "rotation": 5.23,
    "targetGuid": [4321, 8765],
    "inCombat": false,
    "dead": false,
    "ghost": false,
    "mounted": false,
    "flying": false,
    "swimming": false,
    "afk": false,
    "dnd": false,
    "underwater": false,
    "casting": false,
    "valid": true
  }
}
```
