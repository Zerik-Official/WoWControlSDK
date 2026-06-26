# group.* Methods

Group and raid information with checksum-based caching for performance. Group data is rebuilt only when the group composition changes.

---

## `group.get`

Returns full group or raid information including all members.

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `valid` | boolean | whether a group exists |
| `type` | string | `"party"`, `"raid"`, or `"none"` |
| `leaderGuid` | array[2] | `[high, low]` group leader GUID |
| `memberCount` | integer | |
| `members` | array | member objects |

**Member fields:**

| Field | Type | Description |
|---|---|---|
| `name` | string | |
| `level` | integer | |
| `race` | integer | |
| `class` | integer | |
| `health` | integer | |
| `maxHealth` | integer | |
| `power` | integer | |
| `powerMax` | integer | |
| `targetGuid` | array[2] | `[high, low]` |
| `inCombat` | boolean | |
| `dead` | boolean | |
| `ghost` | boolean | |
| `casting` | boolean | |
| `channeling` | boolean | |
| `autoAttacking` | boolean | |

### Example

```json
{"id": 1, "method": "group.get"}
```

```json
{
  "id": 1,
  "result": {
    "valid": true,
    "type": "party",
    "leaderGuid": [1234, 5678],
    "memberCount": 2,
    "members": [
      {
        "name": "PlayerName",
        "level": 80,
        "race": 1,
        "class": 2,
        "health": 25000,
        "maxHealth": 25000,
        "power": 100,
        "powerMax": 100,
        "targetGuid": [0, 0],
        "inCombat": false,
        "dead": false,
        "ghost": false,
        "casting": false,
        "channeling": false,
        "autoAttacking": false
      }
    ]
  }
}
```

> [!NOTE]
> If no group exists, returns `{ "valid": false }` with no additional fields.

---

## `group.combatSummary`

Quick combat-related state summary across all group members. Useful for at-a-glance raid awareness.

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `anyInCombat` | boolean | |
| `anyDead` | boolean | |
| `anyCasting` | boolean | |
| `anyEngaging` | boolean | |
| `countInCombat` | integer | |
| `countDead` | integer | |

### Example

```json
{"id": 1, "method": "group.combatSummary"}
```

```json
{
  "id": 1,
  "result": {
    "anyInCombat": true,
    "anyDead": false,
    "anyCasting": true,
    "anyEngaging": false,
    "countInCombat": 3,
    "countDead": 0
  }
}
```
