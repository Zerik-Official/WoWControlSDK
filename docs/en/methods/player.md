# player.* methods

Read cached player state. These methods read the per-frame cache and never post a request to the game thread, so they return immediately even during heavy load.

The cache is populated once the player enters the world. Before that, every method returns `{"error": "player not available"}`.

## player.getHealth

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `health` | integer | Current health |
| `maxHealth` | integer | Maximum health |

## player.getPosition

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `position` | object | World position `{x, y, z}` |
| `rotation` | float | Facing direction in radians |

## player.getTarget

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `guid` | array[2] | `[high, low]` GUID of the current target |
| `valid` | boolean | Whether the target GUID is non-zero |

!!! note
    When no target is selected, the response is `{"guid": [0, 0], "valid": false}`.

## player.getState

Returns the complete cached player snapshot.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `guid` | array[2] | `[high, low]` player GUID |
| `health` | integer | Current health |
| `maxHealth` | integer | Maximum health |
| `power` | integer | Current mana, rage, energy or runic power |
| `maxPower` | integer | Maximum power |
| `level` | integer | Player level |
| `race` | integer | Race ID |
| `class` | integer | Class ID |
| `xp` | integer | Current experience |
| `maxXp` | integer | Experience required for the next level |
| `position` | object | World position `{x, y, z}` |
| `rotation` | float | Facing direction in radians |
| `targetGuid` | array[2] | `[high, low]` target GUID |
| `inCombat` | boolean | Whether the player is in combat |
| `dead` | boolean | Whether the player is dead |
| `ghost` | boolean | Whether the player is a ghost |
| `mounted` | boolean | Whether the player is mounted |
| `flying` | boolean | Whether the player is flying |
| `swimming` | boolean | Whether the player is swimming |
| `afk` | boolean | Whether the player is AFK |
| `dnd` | boolean | Whether the player is marked as do not disturb |
| `underwater` | boolean | Whether the player is underwater |
| `casting` | boolean | Whether the player is casting |
| `valid` | boolean | Whether the cache entry is populated |

### Example

```json
{"jsonrpc": "2.0", "id": 1, "method": "player.getState"}
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
