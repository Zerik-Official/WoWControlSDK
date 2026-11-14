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
| `inWorld` | boolean | Whether the player is in the game world |
| `loading` | boolean | Whether the world is still loading |
| `valid` | boolean | Whether the cache entry is populated |

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
