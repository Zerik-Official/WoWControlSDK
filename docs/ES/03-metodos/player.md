# Métodos player.*

Estado del jugador en caché — leído de un caché por frame, sin ida y vuelta al hilo del juego. Todos los métodos devuelven `{ "error": "player not available" }` si la entrada de caché aún no está poblada (ej. antes de entrar al mundo).

---

## `player.getHealth`

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `health` | integer | salud actual |
| `maxHealth` | integer | salud máxima |

---

## `player.getPosition`

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `position` | object | `{ x: float, y: float, z: float }` |
| `rotation` | float | dirección de mirada en radianes |

---

## `player.getTarget`

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `guid` | array[2] | `[high, low]` GUID de la unidad |
| `valid` | boolean | si el GUID del objetivo es no-cero |

> [!NOTE]
> Si no hay objetivo seleccionado, devuelve `{ "guid": [0, 0], "valid": false }`.

---

## `player.getState`

Instantánea completa del estado del jugador en caché.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `guid` | array[2] | `[high, low]` |
| `health` | integer | |
| `maxHealth` | integer | |
| `power` | integer | maná/rabia/energía actual |
| `maxPower` | integer | |
| `level` | integer | |
| `race` | integer | ID de raza |
| `class` | integer | ID de clase |
| `xp` | integer | experiencia actual |
| `maxXp` | integer | experiencia para siguiente nivel |
| `position` | object | `{ x: float, y: float, z: float }` |
| `rotation` | float | |
| `targetGuid` | array[2] | `[high, low]` |
| `inCombat` | boolean | |
| `dead` | boolean | |
| `ghost` | boolean | |
| `mounted` | boolean | |
| `flying` | boolean | |
| `swimming` | boolean | |
| `afk` | boolean | |
| `dnd` | boolean | |
| `underwater` | boolean | |
| `casting` | boolean | |
| `valid` | boolean | si la entrada de caché está poblada |

### Ejemplo

```json
{"id": 1, "method": "player.getState"}
```

```json
{
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
