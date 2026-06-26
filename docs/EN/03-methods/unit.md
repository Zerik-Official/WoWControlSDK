# unit.* Methods

Query unit data from the game world — health, position, rotation, auras, and more. Works for `player`, `target`, `focus`, `mouseover`, `partyN`, `raidN`, and GUID-based lookups.

All methods accept a `token` parameter:

| Name | Type | Required | Default | Description |
|---|---|---|---|---|
| `token` | string | no | `"player"` | unit token or GUID string |

GUIDs are returned as `[high, low]` arrays of two `uint32` values.

---

## `unit.get`

Comprehensive snapshot of a unit's properties.

### Response

| Field | Type | Description |
|---|---|---|
| `exists` | boolean | whether the unit reference resolved to a valid unit |
| `name` | string | |
| `level` | integer | |
| `race` | integer | race ID |
| `class` | integer | class ID |
| `health` | integer | current health |
| `maxHealth` | integer | |
| `power` | integer | current mana/rage/energy |
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
| `position` | object | `{ x: float, y: float, z: float }` — world coordinates |
| `mapPosition` | object | `{ x: float, y: float, z: float }` — map-relative |
| `rotation` | float | facing direction in radians |
| `guid` | array[2] | `[high, low]` |
| `targetGuid` | array[2] | `[high, low]` |

### Example

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
> If `exists` is `false`, only the `exists` field is returned. Check it before using other fields.

> [!CAUTION]
> `mapPosition` uses `WorldMap_ConvertToMapPosition` under the hood. Due to a WoW internal limitation, this function can only resolve map positions for party and raid members. For other units, `mapPosition` will return `{0, 0, 0}`. Use `position` (world coordinates) instead — it works for all units.

---

## `unit.state`

Lightweight — returns only boolean state flags for a unit.

### Response

| Field | Type | Description |
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

### Example

```json
{"id": 1, "method": "unit.state", "params": {"token": "target"}}
```

```json
{"id": 1, "result": {"exists": true, "dead": false, "ghost": false, "inCombat": true, "moving": true, "mounted": false, "flying": false, "swimming": false, "casting": false, "channeling": false}}
```

---

## `unit.position`

Returns only the world position of a unit.

### Response

| Field | Type | Description |
|---|---|---|
| `exists` | boolean | |
| `position` | object | `{ x: float, y: float, z: float }` |

---

## `unit.mapPosition`

Returns only the map-relative position of a unit (minimap / UI coordinates).

### Response

| Field | Type | Description |
|---|---|---|
| `exists` | boolean | |
| `mapPosition` | object | `{ x: float, y: float, z: float }` |
