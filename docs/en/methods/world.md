# world.* methods

Read the current world, zone and loading state. The data comes from the per-frame cache, so these calls do not block on the game thread.

## world.getState

Returns the world and zone information of the player.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `mapId` | integer | Current map or instance ID |
| `zoneId` | integer | Current zone ID |
| `continentId` | integer | Continent map ID |
| `zoneName` | string | Localized zone name |
| `subZoneName` | string | Localized subzone name |
| `mapAreaId` | integer | `WorldMapArea.dbc` ID of the current map, the value of `GetCurrentMapAreaID()` |
| `instance` | object | Instance and difficulty information, see below |
| `inWorld` | boolean | Whether the player is in the game world |
| `loading` | boolean | Whether the world is still loading |
| `valid` | boolean | Whether the cache entry is populated |

**`instance` fields** (the data behind `GetInstanceInfo`, `GetInstanceDifficulty`, `GetDungeonDifficulty` and `GetRaidDifficulty`):

| Field | Type | Description |
| --- | --- | --- |
| `inInstance` | boolean | Whether the current map is an instance (`type` is not `"none"`) |
| `mapId` | integer | `Map.dbc` ID used by the instance information |
| `mapName` | string | Localized map name |
| `mapDirectory` | string | Internal map directory, for example `"Northrend"` |
| `type` | string | `"none"`, `"party"`, `"raid"`, `"pvp"` (battleground) or `"arena"` |
| `typeId` | integer | Raw instance type, `0` to `4` in the order above |
| `dynamic` | boolean | Whether the raid difficulty can be changed inside (Icecrown Citadel and similar) |
| `difficulty` | integer | Current difficulty, starting at `1`: dungeons `1` normal and `2` heroic; raids `1` 10 normal, `2` 25 normal, `3` 10 heroic and `4` 25 heroic |
| `difficultyIndex` | integer | `difficulty - 1`, the raw value used by `MapDifficulty.dbc` |
| `difficultyKey` | string | Global string key of the difficulty, for example `"RAID_DIFFICULTY_10PLAYER"`; empty outside instances |
| `mapDifficultyId` | integer | `MapDifficulty.dbc` ID, `0` when the map has none |
| `maxPlayers` | integer | Maximum number of players, `0` outside instances |
| `resetSeconds` | integer | Lockout duration in seconds, `0` when the instance has none |
| `requirementMessage` | string | Requirement text of `MapDifficulty.dbc`, usually empty |
| `playerDifficulty` | integer | Raw `playerDifficulty` value of `GetInstanceInfo` |
| `dungeonDifficulty` | integer | Dungeon difficulty selected outside the instance, `1` or `2` |
| `raidDifficulty` | integer | Raid difficulty selected outside the instance, `1` to `4` |

### Example

```json
{"jsonrpc": "2.0", "id": 1, "method": "world.getState"}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "mapId": 1,
    "zoneId": 1519,
    "continentId": 0,
    "zoneName": "Stormwind City",
    "subZoneName": "Dwarven District",
    "mapAreaId": 301,
    "instance": {
      "inInstance": false,
      "mapId": 0,
      "mapName": "Eastern Kingdoms",
      "mapDirectory": "Azeroth",
      "type": "none",
      "typeId": 0,
      "dynamic": false,
      "difficulty": 1,
      "difficultyIndex": 0,
      "difficultyKey": "",
      "mapDifficultyId": 0,
      "maxPlayers": 0,
      "resetSeconds": 0,
      "requirementMessage": "",
      "playerDifficulty": 0,
      "dungeonDifficulty": 1,
      "raidDifficulty": 1
    },
    "inWorld": true,
    "loading": false,
    "valid": true
  }
}
```

### Errors

| Condition | Response |
| --- | --- |
| The player is not in the game world | `{"error": "world not available"}` |
