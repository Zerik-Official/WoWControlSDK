# world.* Methods

Read current world, zone, and map state. Data comes from a per-frame cache — no game-thread round-trip.

---

## `world.getState`

Returns the current world and zone information for the player.

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `mapId` | integer | current map/instance ID |
| `zoneId` | integer | current zone ID |
| `continentId` | integer | continent map ID |
| `zoneName` | string | localized zone name |
| `subZoneName` | string | localized subzone name |
| `inWorld` | boolean | whether the player is in the game world |
| `loading` | boolean | whether the world is still loading |
| `valid` | boolean | whether the cache entry is populated |

### Example

```json
{"id": 1, "method": "world.getState"}
```

```json
{
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

| Condition | Error |
|---|---|
| Not in the game world | `{ "error": "world not available" }` |
