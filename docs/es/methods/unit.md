# Métodos unit.*

Leen datos de una unidad del mundo del juego: salud, poder, posición, rotación, auras y flags de estado.

Los métodos aceptan un parámetro `token` que identifica la unidad:

| Nombre | Tipo | Obligatorio | Por defecto | Descripción |
| --- | --- | --- | --- | --- |
| `token` | string | No | `"player"` | Token de unidad (`player`, `target`, `focus`, `mouseover`, `partyN`, `raidN`) o una cadena GUID |

Los GUID se devuelven como arreglos `[high, low]` de dos enteros sin signo de 32 bits.

## unit.get

Devuelve una instantánea completa de una unidad.

### Parámetros

El parámetro común `token`.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `exists` | boolean | Indica si el token resolvió a una unidad válida |
| `name` | string | Nombre de la unidad |
| `level` | integer | Nivel de la unidad |
| `race` | integer | ID de raza |
| `class` | integer | ID de clase |
| `health` | integer | Salud actual |
| `maxHealth` | integer | Salud máxima |
| `power` | integer | Maná, ira, energía o poder rúnico actual |
| `maxPower` | integer | Poder máximo |
| `dead` | boolean | Indica si la unidad está muerta |
| `ghost` | boolean | Indica si la unidad es un fantasma |
| `inCombat` | boolean | Indica si la unidad está en combate |
| `moving` | boolean | Indica si la unidad se está moviendo |
| `mounted` | boolean | Indica si la unidad va montada |
| `flying` | boolean | Indica si la unidad está volando |
| `swimming` | boolean | Indica si la unidad está nadando |
| `casting` | boolean | Indica si la unidad está lanzando un hechizo |
| `channeling` | boolean | Indica si la unidad está canalizando |
| `position` | object | Posición en el mundo `{x, y, z}` |
| `mapPosition` | object | Posición de mapa normalizada `{x, y, z}` |
| `rotation` | float | Dirección de orientación en radianes |
| `guid` | array[2] | GUID de la unidad `[high, low]` |
| `targetGuid` | array[2] | GUID del objetivo `[high, low]` |

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "unit.get", "params": {"token": "player"}}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "exists": true,
    "name": "PlayerName",
    "level": 80,
    "race": 1,
    "class": 2,
    "health": 25000,
    "maxHealth": 25000,
    "power": 100,
    "maxPower": 100,
    "dead": false,
    "ghost": false,
    "inCombat": false,
    "moving": false,
    "mounted": false,
    "flying": false,
    "swimming": false,
    "casting": false,
    "channeling": false,
    "position": {"x": 1234.5, "y": 2345.6, "z": 300.1},
    "mapPosition": {"x": 0.5, "y": 0.3, "z": 0.0},
    "rotation": 5.23,
    "guid": [1234, 5678],
    "targetGuid": [4321, 8765]
  }
}
```

!!! tip
    Cuando `exists` es `false`, la respuesta solo contiene el campo `exists`. Compruébalo antes de leer cualquier otro campo.

!!! warning
    `mapPosition` usa `WorldMap_ConvertToMapPosition`, que solo resuelve posiciones de miembros del grupo y de la raid. Para el resto de unidades devuelve `{0, 0, 0}`. Usa `position` si necesitas coordenadas del mundo válidas para cualquier unidad.

## unit.state

Devuelve solo los flags de estado booleanos de una unidad.

### Parámetros

El parámetro común `token`.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `exists` | boolean | Indica si la unidad existe |
| `dead` | boolean | Indica si la unidad está muerta |
| `ghost` | boolean | Indica si la unidad es un fantasma |
| `inCombat` | boolean | Indica si la unidad está en combate |
| `moving` | boolean | Indica si la unidad se está moviendo |
| `mounted` | boolean | Indica si la unidad va montada |
| `flying` | boolean | Indica si la unidad está volando |
| `swimming` | boolean | Indica si la unidad está nadando |
| `casting` | boolean | Indica si la unidad está lanzando un hechizo |
| `channeling` | boolean | Indica si la unidad está canalizando |

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "unit.state", "params": {"token": "target"}}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "exists": true,
    "dead": false,
    "ghost": false,
    "inCombat": true,
    "moving": true,
    "mounted": false,
    "flying": false,
    "swimming": false,
    "casting": false,
    "channeling": false
  }
}
```

## unit.position

Devuelve solo la posición en el mundo de una unidad.

### Parámetros

El parámetro común `token`.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `exists` | boolean | Indica si la unidad existe |
| `position` | object | Posición en el mundo `{x, y, z}` |

## unit.mapPosition

Devuelve solo la posición de mapa normalizada de una unidad.

### Parámetros

El parámetro común `token`.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `exists` | boolean | Indica si la unidad existe |
| `mapPosition` | object | Posición de mapa normalizada `{x, y, z}` |

!!! warning
    Las posiciones de mapa solo se resuelven para miembros del grupo y de la raid. El resto de unidades devuelve `{0, 0, 0}`.
