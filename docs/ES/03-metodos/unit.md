# Métodos unit.*

Consulta datos de unidades del mundo del juego — salud, posición, rotación, auras y más. Funciona para `player`, `target`, `focus`, `mouseover`, `partyN`, `raidN` y búsquedas por GUID.

Todos los métodos aceptan un parámetro `token`:

| Nombre | Tipo | Requerido | Por defecto | Descripción |
|---|---|---|---|---|
| `token` | string | no | `"player"` | token de unidad o string GUID |

Los GUIDs se devuelven como arrays `[high, low]` de dos valores `uint32`.

---

## `unit.get`

Instantánea completa de las propiedades de una unidad.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `exists` | boolean | si la referencia a unidad se resolvió a una unidad válida |
| `name` | string | |
| `level` | integer | |
| `race` | integer | ID de raza |
| `class` | integer | ID de clase |
| `health` | integer | salud actual |
| `maxHealth` | integer | |
| `power` | integer | maná/rabia/energía actual |
| `maxPower` | integer | |
| `dead` | boolean | |
| `ghost` | boolean | |
| `inCombat` | boolean | |
| `moving` | boolean | |
| `mounted` | boolean | |
| `flying` | boolean | |
| `swimming` | boolean | |
| `casting` | boolean | |
| `channeling` | boolean | |
| `position` | object | `{ x: float, y: float, z: float }` — coordenadas del mundo |
| `mapPosition` | object | `{ x: float, y: float, z: float }` — relativo al mapa |
| `rotation` | float | dirección de mirada en radianes |
| `guid` | array[2] | `[high, low]` |
| `targetGuid` | array[2] | `[high, low]` |

### Ejemplo

```json
{"id": 1, "method": "unit.get", "params": {"token": "player"}}
```

```json
{
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

> [!TIP]
> Si `exists` es `false`, solo se devuelve el campo `exists`. Verifícalo antes de usar otros campos.

> [!CAUTION]
> `mapPosition` usa `WorldMap_ConvertToMapPosition` internamente. Debido a una limitación interna de WoW, esta función solo puede resolver posiciones de mapa para miembros de grupo y raid. Para otras unidades, `mapPosition` devolverá `{0, 0, 0}`. Usa `position` (coordenadas del mundo) — funciona para todas las unidades.

---

## `unit.state`

Ligero — devuelve solo los flags booleanos de estado de una unidad.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `exists` | boolean | |
| `dead` | boolean | |
| `ghost` | boolean | |
| `inCombat` | boolean | |
| `moving` | boolean | |
| `mounted` | boolean | |
| `flying` | boolean | |
| `swimming` | boolean | |
| `casting` | boolean | |
| `channeling` | boolean | |

### Ejemplo

```json
{"id": 1, "method": "unit.state", "params": {"token": "target"}}
```

```json
{"id": 1, "result": {"exists": true, "dead": false, "ghost": false, "inCombat": true, "moving": true, "mounted": false, "flying": false, "swimming": false, "casting": false, "channeling": false}}
```

---

## `unit.position`

Devuelve solo la posición mundial de una unidad.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `exists` | boolean | |
| `position` | object | `{ x: float, y: float, z: float }` |

---

## `unit.mapPosition`

Devuelve solo la posición relativa al mapa de una unidad (coordenadas de minimapa / UI).

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `exists` | boolean | |
| `mapPosition` | object | `{ x: float, y: float, z: float }` |
