# player.* Methods

Cached player state — read from a per-frame cache, no game-thread round-trip. All methods return `{ "error": "player not available" }` if the cache entry is not yet populated (e.g. before entering the world).

---

## `player.getHealth`

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `health` | integer | current health |
| `maxHealth` | integer | maximum health |

---

## `player.getPosition`

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `position` | object | `{ x: float, y: float, z: float }` |
| `rotation` | float | facing direction in radians |

---

## `player.getTarget`

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `guid` | array[2] | `[high, low]` unit GUID |
| `valid` | boolean | whether the target GUID is non-zero |

> [!NOTE]
> If no target is selected, returns `{ "guid": [0, 0], "valid": false }`.

---

## `player.getState`

Comprehensive cached player state snapshot.

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `guid` | array[2] | `[high, low]` |
| `health` | integer | |
| `maxHealth` | integer | |
| `power` | integer | current mana/rage/energy |
| `maxPower` | integer | |
| `level` | integer | |
| `race` | integer | race ID |
| `class` | integer | class ID |
| `xp` | integer | current experience |
| `maxXp` | integer | experience to next level |
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
| `valid` | boolean | whether the cache entry is populated |

### Example

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
