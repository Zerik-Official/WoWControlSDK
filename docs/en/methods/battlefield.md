# battlefield.* methods

Read the battleground and arena queues and answer the "You may now enter" invitation natively. Queue changes are also pushed as [SDK events](events.md#sdk-event-payloads): `battlefield.statusChanged` and `battlefield.ready`.

## battlefield.getQueues

Returns both queue slots with the data of `GetBattlefieldStatus`, `GetBattlefieldPortExpiration`, `GetBattlefieldTimeWaited` and `GetBattlefieldEstimatedWaitTime`.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `queues` | array | Two queue objects, index `1` and `2` |

**Queue fields:**

| Field | Type | Description |
| --- | --- | --- |
| `index` | integer | Queue slot, `1` or `2` |
| `status` | string | `"none"`, `"queued"`, `"confirm"` (the invitation is open), `"active"` (inside) or `"error"` |
| `statusId` | integer | Raw status, `0` to `3` in the order above |
| `typeId` | integer | `BattlemasterList.dbc` ID, `32` for a random battleground |
| `typeName` | string | Localized battleground or arena name |
| `random` | boolean | Random battleground queue |
| `arena` | boolean | Arena queue (`teamSize > 0`) |
| `rated` | boolean | Rated match (`registeredMatch`) |
| `teamSize` | integer | `2`, `3` or `5` for arenas, `0` for battlegrounds |
| `mapId` | integer | `Map.dbc` ID; for an open random invitation, the battleground chosen by the server |
| `mapName` | string | Localized map name once the invitation is open or you are inside, otherwise `typeName` |
| `instanceId` | integer | Battleground instance number shown in the game |
| `minLevel`, `maxLevel` | integer | Level bracket |
| `queueGuid` | array[2] | `[high, low]` queue identifier sent back to the server |
| `instanceGuid` | array[2] | `[high, low]` raw instance identifier |
| `estimatedWaitMs` | integer | Estimated wait in milliseconds |
| `timeWaitedMs` | integer | Time spent in the queue in milliseconds |
| `portExpiresInMs` | integer | Milliseconds left to enter while `status` is `"confirm"` |
| `canLeave` | boolean | Whether the invitation allows leaving the queue; arena invitations cannot be declined |

### Example

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "battlefield.getQueues"}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "queues": [
      {
        "index": 1,
        "status": "confirm",
        "statusId": 2,
        "typeId": 32,
        "typeName": "Random Battleground",
        "random": true,
        "arena": false,
        "rated": false,
        "teamSize": 0,
        "mapId": 489,
        "mapName": "Warsong Gulch",
        "instanceId": 12,
        "minLevel": 80,
        "maxLevel": 80,
        "queueGuid": [503316480, 32],
        "instanceGuid": [0, 131072],
        "estimatedWaitMs": 120000,
        "timeWaitedMs": 95000,
        "portExpiresInMs": 78000,
        "canLeave": true
      },
      {
        "index": 2,
        "status": "none",
        "statusId": 0,
        "typeId": 0,
        "typeName": "",
        "random": false,
        "arena": false,
        "rated": false,
        "teamSize": 0,
        "mapId": 0,
        "mapName": "",
        "instanceId": 0,
        "minLevel": 0,
        "maxLevel": 0,
        "queueGuid": [0, 0],
        "instanceGuid": [0, 0],
        "estimatedWaitMs": 0,
        "timeWaitedMs": 0,
        "portExpiresInMs": 0,
        "canLeave": true
      }
    ]
  }
}
```

## battlefield.acceptPort

Enters the battleground or arena like the **Enter Battle** button (`AcceptBattlefieldPort(index, 1)`, `CMSG_BATTLEFIELD_PORT`).

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `index` | integer | Yes | Queue slot, `1` or `2` |

### Response

```json
{"ok": true}
```

### Errors

| Condition | Response |
| --- | --- |
| `index` is not `1` or `2` | `{"error": "index must be 1 or 2"}` |
| The player is not in the world | `{"error": "not in world"}` |
| The queue has no open invitation | `{"error": "queue is not in the required status"}` |

## battlefield.leaveQueue

Leaves a queue, or declines its invitation, like the **Leave Queue** button (`AcceptBattlefieldPort(index, nil)`).

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `index` | integer | Yes | Queue slot, `1` or `2` |

### Response

```json
{"ok": true}
```

### Errors

| Condition | Response |
| --- | --- |
| `index` is not `1` or `2` | `{"error": "index must be 1 or 2"}` |
| The player is not in the world | `{"error": "not in world"}` |
| The queue is not queued or waiting for an answer | `{"error": "queue is not in the required status"}` |
| The invitation is an arena match | `{"error": "this queue cannot be left from the invitation"}` |

!!! note
    The SDK builds `CMSG_BATTLEFIELD_PORT` with the client's own packet functions because `AcceptBattlefieldPort` reads its arguments from Lua. The client-side checks of the button (for example being in combat) are left to the server.
