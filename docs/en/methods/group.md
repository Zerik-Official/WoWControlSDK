# group.* methods

Read party and raid information. Group data is checksum-based and rebuilt only when the group composition changes, so repeated calls are cheap.

## group.get

Returns the full group or raid roster.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `valid` | boolean | Whether a group exists |
| `type` | string | `"party"`, `"raid"` or `"none"` |
| `leaderGuid` | array[2] | `[high, low]` GUID of the group leader |
| `memberCount` | integer | Number of members |
| `members` | array | Member objects |

**Member fields:**

| Field | Type | Description |
| --- | --- | --- |
| `name` | string | Member name |
| `level` | integer | Member level |
| `race` | integer | Race ID |
| `class` | integer | Class ID |
| `health` | integer | Current health |
| `maxHealth` | integer | Maximum health |
| `power` | integer | Current power |
| `powerMax` | integer | Maximum power |
| `targetGuid` | array[2] | `[high, low]` target GUID |
| `inCombat` | boolean | Whether the member is in combat |
| `dead` | boolean | Whether the member is dead |
| `ghost` | boolean | Whether the member is a ghost |
| `casting` | boolean | Whether the member is casting |
| `channeling` | boolean | Whether the member is channeling |
| `autoAttacking` | boolean | Whether the member is auto-attacking |

### Example

```json
{"jsonrpc": "2.0", "id": 1, "method": "group.get"}
```

```json
{
  "jsonrpc": "2.0",
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

!!! note
    When no group exists, the response is `{"valid": false}` and contains no other fields.

## group.combatSummary

Returns a combat summary across all group members.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `anyInCombat` | boolean | Whether at least one member is in combat |
| `anyDead` | boolean | Whether at least one member is dead |
| `anyCasting` | boolean | Whether at least one member is casting |
| `anyEngaging` | boolean | Whether at least one member is engaging |
| `countInCombat` | integer | Number of members in combat |
| `countDead` | integer | Number of dead members |

### Example

```json
{"jsonrpc": "2.0", "id": 1, "method": "group.combatSummary"}
```

```json
{
  "jsonrpc": "2.0",
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
