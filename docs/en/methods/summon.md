# summon.* methods

Read a pending summon (for example from a meeting stone or a warlock's Ritual of Summoning) and accept or decline it natively. The [`summon.requested`](events.md#sdk-event-payloads) event announces a new summon.

## summon.get

Returns the pending summon, the same data the game shows in the `CONFIRM_SUMMON` dialog.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `pending` | boolean | Whether a summon is waiting and has not expired |
| `summonerGuid` | array[2] | `[high, low]` GUID of the summoner |
| `summonerName` | string | Name of the summoner (`GetSummonConfirmSummoner`) |
| `areaId` | integer | `AreaTable.dbc` ID of the destination |
| `areaName` | string | Localized destination (`GetSummonConfirmAreaName`) |
| `timeLeftMs` | integer | Milliseconds until the summon expires (`GetSummonConfirmTimeLeft`), `0` when expired |
| `canTeleport` | boolean | Whether the game would allow accepting now (not in combat, not stunned, not feared or otherwise locked) |

### Example

```json
{"jsonrpc": "2.0", "id": 1, "method": "summon.get"}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "pending": true,
    "summonerGuid": [0, 1234],
    "summonerName": "Arthas",
    "areaId": 4395,
    "areaName": "Dalaran",
    "timeLeftMs": 108000,
    "canTeleport": true
  }
}
```

## summon.accept

Accepts the summon like the dialog's **Accept** button (`ConfirmSummon`, `CMSG_SUMMON_RESPONSE` with `true`) and closes the dialog.

### Parameters

None.

### Response

```json
{"ok": true}
```

### Errors

| Condition | Response |
| --- | --- |
| The player is not in the world | `{"error": "not in world"}` |
| There is no summon, or it expired | `{"error": "no pending summon"}` |
| The player is in combat or cannot teleport | `{"error": "cannot teleport now"}` (the summon stays pending) |

## summon.decline

Declines the summon like the dialog's **Cancel** button (`CancelSummon`, `CMSG_SUMMON_RESPONSE` with `false`) and closes the dialog.

### Parameters

None.

### Response

```json
{"ok": true}
```

### Errors

| Condition | Response |
| --- | --- |
| The player is not in the world | `{"error": "not in world"}` |
| There is no summon, or it expired | `{"error": "no pending summon"}` |
