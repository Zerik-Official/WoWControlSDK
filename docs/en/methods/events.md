# events.* methods

Subscribe to WoW UI events and read or stream them through the SDK. The event system has a circular history buffer and a dedicated event pipe at `\\.\pipe\WowGameEvent`.

Events fall into two groups:

| Group | Delivery | Examples |
| --- | --- | --- |
| WoW UI events | Pushed only after `events.subscribe` | `PLAYER_LOGIN`, `PLAYER_ENTERING_WORLD`, `UNIT_HEALTH`, `ZONE_CHANGED` |
| SDK events | Pushed automatically | `client.screenChanged`, `loading.*`, `friends.*`, `group.invite*`, `summon.*`, `lfg.*`, `trade.*`, `duel.*`, `battlefield.*`, `chat.whisperReceived` |

Every event that reaches the event pipe is also stored in the history buffer.

## events.subscribe

Subscribes to one or more WoW events.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `events` | array[string] | Yes | WoW event names to subscribe to |

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | Always `true` |
| `subscribed` | integer | Number of names processed |

### Example

```json
{"jsonrpc": "2.0", "id": 1, "method": "events.subscribe", "params": {"events": ["PLAYER_LOGIN", "PLAYER_ENTERING_WORLD", "UNIT_HEALTH"]}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "subscribed": 3}}
```

!!! note
    Subscribing to an event that is already subscribed is idempotent.

## events.unsubscribe

Unsubscribes from one or more WoW events.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `events` | array[string] | Yes | WoW event names to unsubscribe from |

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | Always `true` |
| `unsubscribed` | integer | Number of names processed |

### Example

```json
{"jsonrpc": "2.0", "id": 1, "method": "events.unsubscribe", "params": {"events": ["UNIT_HEALTH"]}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "unsubscribed": 1}}
```

## events.list

Lists the currently subscribed events.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `events` | array[string] | Subscribed event names |

### Example

```json
{"jsonrpc": "2.0", "id": 1, "method": "events.list"}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"events": ["PLAYER_LOGIN", "PLAYER_ENTERING_WORLD"]}}
```

## events.clear

Removes every subscription.

### Parameters

None.

### Response

```json
{"ok": true}
```

## events.getHistory

Returns a slice of the circular history buffer.

### Parameters

| Name | Type | Required | Default | Description |
| --- | --- | --- | --- | --- |
| `offset` | integer | No | `0` | Position in the buffer to start from |
| `count` | integer | No | `50` | Number of entries to return |

### Response

| Field | Type | Description |
| --- | --- | --- |
| `total` | integer | Entries currently in the buffer |
| `returned` | integer | Entries returned in this response |
| `offset` | integer | Offset used by the request |
| `entries` | array | Event entry objects |

**Entry fields:**

| Field | Type | Description |
| --- | --- | --- |
| `event` | string | Event name |
| `args` | any | Event arguments, whose shape depends on the event |
| `time` | integer | Timestamp in milliseconds |

### Example

```json
{"jsonrpc": "2.0", "id": 1, "method": "events.getHistory", "params": {"offset": 0, "count": 2}}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "total": 10,
    "returned": 2,
    "offset": 0,
    "entries": [
      {"event": "PLAYER_LOGIN", "args": [], "time": 1712345678000},
      {"event": "client.screenChanged", "args": {"old": 2, "new": 3}, "time": 1712345679000}
    ]
  }
}
```

!!! note
    The buffer is circular and defaults to 256 entries. When it is full, the oldest entry is overwritten. Use `events.setMaxBuffer` to change the size.

## events.clearHistory

Clears the history buffer.

### Parameters

None.

### Response

```json
{"ok": true}
```

## events.setMaxBuffer

Sets the maximum size of the history buffer.

### Parameters

| Name | Type | Required | Default | Description |
| --- | --- | --- | --- | --- |
| `maxBuffer` | integer | No | `256` | New maximum buffer size |

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | Always `true` |
| `maxBuffer` | integer | Configured maximum |

!!! note
    Reducing the size does not trim existing entries immediately. They are overwritten as new events arrive.

## events.isReady

Checks whether the event hook system is initialized.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ready` | boolean | Whether the hook system is ready |

## SDK event payloads

| Event | Description | Arguments |
| --- | --- | --- |
| `client.screenChanged` | The glue screen changed | `{"old": <int>, "new": <int>}` where `0` is unknown, `1` is login, `2` is character select and `3` is world |
| `loading.startup` | A loading screen appeared | `{"mapId": <int>, "background": <object or null>}` |
| `loading.progress` | Loading progress update | `{"progress": <float>}` between `0.0` and `1.0` |
| `loading.finished` | Loading completed and the world was entered | `{"mapId": <int>}` |
| `loading.cancelled` | The loading screen closed without entering the world within 3 seconds | `{"mapId": <int>}` |
| `friends.online` | A friend connected | [Friend object](friends.md#friendslist) |
| `friends.offline` | A friend disconnected | [Friend object](friends.md#friendslist) |
| `friends.statusChanged` | An online friend became AFK or DND, or stopped being | [Friend object](friends.md#friendslist) |
| `group.inviteReceived` | Someone invited you to a group | `{"inviterName": <string>}` |
| `group.inviteClosed` | The invitation ended | `{"inviterName", "reason", "roles"}`; `reason` is `"accepted"`, `"declined"`, `"cancelled"` (the inviter withdrew it or it expired) or `"joined"`; `roles` are the [roles](group.md#roles) sent when accepting |
| `summon.requested` | Someone summons you | [Summon object](summon.md#summonget) without `canTeleport` |
| `summon.closed` | The summon was answered or cancelled | Summon object plus `reason`: `"accepted"`, `"declined"` or `"cancelled"` |
| `lfg.proposalShown` | The Dungeon Finder found a group | [Proposal object](lfg.md#lfggetproposal) |
| `lfg.proposalUpdated` | A member answered the proposal | Proposal object |
| `lfg.proposalFailed` | Someone declined or the proposal expired | Proposal object (usually `{"exists": false}`) |
| `lfg.proposalSucceeded` | Everyone accepted | Proposal object |
| `trade.requested` | Someone wants to trade with you | `{"requesterName": <string>}` |
| `trade.requestClosed` | The request ended | `{"requesterName", "reason"}`; `reason` is `"accepted"`, `"declined"`, `"cancelled"` or `"closed"` |
| `trade.opened` | The trade window opened | [Trade object](trade.md#tradeget) |
| `trade.updated` | Items, money or accept states changed | Trade object plus `change`: `"update"`, `"accept"`, `"playerItem"`, `"targetItem"` or `"money"` |
| `trade.closed` | The trade window closed | `{"bothAccepted": <bool>}`; `true` means both players had accepted, so the trade most likely completed |
| `duel.requested` | Someone challenged you to a duel | [Duel object](duel.md#duelget) |
| `duel.accepted` | You accepted the duel | Duel object plus `reason: "accepted"` |
| `duel.closed` | The request or the duel ended | Duel object plus `reason`: `"declined"`, `"cancelled"`, `"forfeited"` or `"finished"` |
| `duel.boundsChanged` | You left or re-entered the duel area | `{"inBounds": <bool>}` |
| `battlefield.statusChanged` | A queue changed status | [Queue object](battlefield.md#battlefieldgetqueues) plus `previousStatusId` |
| `battlefield.ready` | A battleground or arena is ready to enter | Queue object |
| `chat.whisperReceived` | You received a whisper | `{"battleNet", "text", "author", "language", "status", "lineId", "guid", "presenceId", "args"}`; `status` is `"AFK"`, `"DND"`, `"GM"` or empty, `presenceId` is only set for Battle.net whispers and `args` holds the raw event arguments |

`background` describes the loading screen image from `LoadingScreens.dbc`, or is `null` when the map has none and the generic `Interface\Glues\loading` image is used:

| Field | Type | Description |
| --- | --- | --- |
| `id` | integer | `LoadingScreens.dbc` row id |
| `name` | string | Internal name of the loading screen |
| `file` | string | Path of the `.blp` image, for example `Interface\Glues\LoadingScreens\LoadScreenNorthrend.blp` |
| `hasWideScreen` | boolean | Whether a `Wide` variant exists; the client shows it on wide resolutions |

Social, trade, duel, queue and whisper events are built from the matching WoW events (`PARTY_INVITE_REQUEST`, `CONFIRM_SUMMON`, `LFG_PROPOSAL_SHOW`, `FRIENDLIST_UPDATE`, `TRADE_*`, `DUEL_*`, `UPDATE_BATTLEFIELD_STATUS`, `CHAT_MSG_WHISPER`…) without subscribing to them, and their data is read from memory. Answers given with the game's own buttons are also reported, because the SDK follows the native functions behind those buttons. Friend events only compare entries already known in the same session, so logging in does not report every online friend. When the SDK closes a game dialog after answering for you, the WoW event it uses for that is not reported as a new SDK event.

Loading events are polled from memory once per frame rather than hooked from Lua, and `loading.progress` is emitted at most once every 100 milliseconds. A loading screen is reported only while the client's loading bar exists, so the screen drawn and destroyed in the same frame when logging out to the character list does not produce events.

## Live event streaming

Connect to `\\.\pipe\WowGameEvent` to receive events as newline-delimited JSON. The pipe accepts one client at a time and stays open until the client disconnects. Send the authentication handshake first; the SDK answers with `auth.ok` or `auth.failed` (see [authentication](../protocol.md#authentication)). With `--no-auth` no handshake is needed.

```python
import json

import win32file

pipe = win32file.CreateFile(
    r"\\.\pipe\WowGameEvent",
    win32file.GENERIC_READ | win32file.GENERIC_WRITE,
    0,
    None,
    win32file.OPEN_EXISTING,
    0,
    None,
)

try:
    win32file.WriteFile(pipe, (json.dumps({"auth": TOKEN}) + "\n").encode("utf-8"))
    buffer = ""
    while True:
        _, data = win32file.ReadFile(pipe, 65536)
        buffer += data.decode("utf-8", errors="replace")
        while "\n" in buffer:
            line, buffer = buffer.split("\n", 1)
            if line.strip():
                print(json.loads(line))
finally:
    win32file.CloseHandle(pipe)
```

The [CLI tool](../operations/cli.md) exposes the same stream through **Events → Listen live**.
