# events.* methods

Subscribe to WoW UI events and read or stream them through the SDK. The event system has a circular history buffer and a dedicated event pipe at `\\.\pipe\WowGameEvent`.

Events fall into two groups:

| Group | Delivery | Examples |
| --- | --- | --- |
| WoW UI events | Pushed only after `events.subscribe` | `PLAYER_LOGIN`, `PLAYER_ENTERING_WORLD`, `UNIT_HEALTH`, `ZONE_CHANGED` |
| SDK events | Pushed automatically | `client.screenChanged`, `loading.startup`, `loading.progress`, `loading.finished`, `loading.cancelled` |

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

`background` describes the loading screen image from `LoadingScreens.dbc`, or is `null` when the map has none and the generic `Interface\Glues\loading` image is used:

| Field | Type | Description |
| --- | --- | --- |
| `id` | integer | `LoadingScreens.dbc` row id |
| `name` | string | Internal name of the loading screen |
| `file` | string | Path of the `.blp` image, for example `Interface\Glues\LoadingScreens\LoadScreenNorthrend.blp` |
| `hasWideScreen` | boolean | Whether a `Wide` variant exists; the client shows it on wide resolutions |

Loading events are polled from memory once per frame rather than hooked from Lua, and `loading.progress` is emitted at most once every 100 milliseconds. A loading screen is reported only while the client's loading bar exists, so the screen drawn and destroyed in the same frame when logging out to the character list does not produce events.

## Live event streaming

Connect to `\\.\pipe\WowGameEvent` to receive events as newline-delimited JSON. The pipe accepts one client at a time and stays open until the client disconnects.

```python
import json

import win32file

pipe = win32file.CreateFile(
    r"\\.\pipe\WowGameEvent",
    win32file.GENERIC_READ,
    0,
    None,
    win32file.OPEN_EXISTING,
    0,
    None,
)

try:
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
