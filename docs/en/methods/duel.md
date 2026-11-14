# duel.* methods

Read a duel request and accept or decline it natively. Duels are also pushed as [SDK events](events.md#sdk-event-payloads): `duel.requested`, `duel.accepted`, `duel.closed` and `duel.boundsChanged`.

## duel.get

Returns the current duel request or duel.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `pending` | boolean | A duel request waits for an answer |
| `active` | boolean | You accepted a duel that has not finished yet |
| `challengerName` | string | Name of the player who challenged you |
| `challengerGuid` | array[2] | `[high, low]` GUID of the challenger |
| `arbiterGuid` | array[2] | `[high, low]` GUID of the duel flag |
| `requestedAtMs` | integer | `GetTickCount64()` time of the request, `0` when none |

### Example

```json
{"jsonrpc": "2.0", "id": 1, "method": "duel.get"}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "pending": true,
    "active": false,
    "challengerName": "Arthas",
    "challengerGuid": [0, 1234],
    "arbiterGuid": [4026597376, 81],
    "requestedAtMs": 81234567
  }
}
```

!!! note
    `challengerGuid` and `arbiterGuid` are read from the client's last duel and keep their values after the duel ends. Use `pending` and `active` to know whether a duel is in progress.

## duel.accept

Accepts the duel like the dialog's **Accept** button (`AcceptDuel`, `CMSG_DUEL_ACCEPTED`) and closes the dialog.

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
| No duel request is pending | `{"error": "no pending duel"}` |

## duel.decline

Declines the request like the dialog's **Decline** button (`CancelDuel`, `CMSG_DUEL_CANCELLED`) and closes the dialog. During an accepted duel it forfeits, like calling `CancelDuel` in the game.

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
| There is no duel request and no active duel | `{"error": "no pending duel"}` |
