# unit.* methods

Read unit data from the game world: health, power, position, rotation, auras and state flags.

The methods accept a `token` parameter that identifies the unit:

| Name | Type | Required | Default | Description |
| --- | --- | --- | --- | --- |
| `token` | string | No | `"player"` | Unit token (`player`, `target`, `focus`, `mouseover`, `partyN`, `raidN`) or a GUID string |

GUIDs are returned as `[high, low]` arrays of two 32-bit unsigned integers.

## unit.get

Returns a complete snapshot of a unit.

### Parameters

The common `token` parameter.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `exists` | boolean | Whether the token resolved to a valid unit |
| `name` | string | Unit name |
| `level` | integer | Unit level |
| `race` | integer | Race ID |
| `class` | integer | Class ID |
| `health` | integer | Current health |
| `maxHealth` | integer | Maximum health |
| `power` | integer | Current mana, rage, energy or runic power |
| `maxPower` | integer | Maximum power |
| `dead` | boolean | Whether the unit is dead |
| `ghost` | boolean | Whether the unit is a ghost |
| `inCombat` | boolean | Whether the unit is in combat |
| `moving` | boolean | Whether the unit is moving |
| `mounted` | boolean | Whether the unit is mounted |
| `flying` | boolean | Whether the unit is flying |
| `swimming` | boolean | Whether the unit is swimming |
| `casting` | boolean | Whether the unit is casting |
| `channeling` | boolean | Whether the unit is channeling |
| `position` | object | World position `{x, y, z}` |
| `mapPosition` | object | Normalized map position `{x, y, z}` |
| `rotation` | float | Facing direction in radians |
| `guid` | array[2] | `[high, low]` unit GUID |
| `targetGuid` | array[2] | `[high, low]` target GUID |

### Example

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
    When `exists` is `false`, the response contains only the `exists` field. Check it before reading any other field.

!!! warning
    `mapPosition` uses `WorldMap_ConvertToMapPosition`, which can only resolve positions for party and raid members. For other units it returns `{0, 0, 0}`. Use `position` for world coordinates that work for every unit.

## unit.state

Returns only the boolean state flags of a unit.

### Parameters

The common `token` parameter.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `exists` | boolean | Whether the unit exists |
| `dead` | boolean | Whether the unit is dead |
| `ghost` | boolean | Whether the unit is a ghost |
| `inCombat` | boolean | Whether the unit is in combat |
| `moving` | boolean | Whether the unit is moving |
| `mounted` | boolean | Whether the unit is mounted |
| `flying` | boolean | Whether the unit is flying |
| `swimming` | boolean | Whether the unit is swimming |
| `casting` | boolean | Whether the unit is casting |
| `channeling` | boolean | Whether the unit is channeling |

### Example

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

Returns only the world position of a unit.

### Parameters

The common `token` parameter.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `exists` | boolean | Whether the unit exists |
| `position` | object | World position `{x, y, z}` |

## unit.mapPosition

Returns only the normalized map position of a unit.

### Parameters

The common `token` parameter.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `exists` | boolean | Whether the unit exists |
| `mapPosition` | object | Normalized map position `{x, y, z}` |

!!! warning
    Map positions are only resolved for party and raid members. Other units return `{0, 0, 0}`.
