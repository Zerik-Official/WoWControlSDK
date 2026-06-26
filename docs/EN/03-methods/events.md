# events.* Methods

Subscribe to WoW events and receive them in real-time over the named pipe. Supports standard WoW UI events (`PLAYER_LOGIN`, `PLAYER_ENTERING_WORLD`, `COMBAT_LOG_EVENT_UNFILTERED`, etc.) as well as SDK custom events.

Events are captured into a circular buffer and can also be streamed over the event pipe (separate named pipe connection).

---

## `events.subscribe`

Subscribe to one or more WoW events.

### Parameters

| Name | Type | Required | Description |
|---|---|---|---|
| `events` | array[string] | yes | list of WoW event names to subscribe to |

### Response

| Field | Type | Description |
|---|---|---|
| `ok` | boolean | always `true` |
| `subscribed` | integer | count of events successfully subscribed |

### Example

```json
{"id": 1, "method": "events.subscribe", "params": {"events": ["PLAYER_LOGIN", "PLAYER_ENTERING_WORLD", "UNIT_HEALTH"]}}
```

```json
{"id": 1, "result": {"ok": true, "subscribed": 3}}
```

> [!NOTE]
> Subscribing to an already-subscribed event is idempotent.

---

## `events.unsubscribe`

Unsubscribe from one or more WoW events.

### Parameters

| Name | Type | Required | Description |
|---|---|---|---|
| `events` | array[string] | yes | list of WoW event names to unsubscribe from |

### Response

| Field | Type | Description |
|---|---|---|
| `ok` | boolean | always `true` |
| `unsubscribed` | integer | count of events unsubscribed |

### Example

```json
{"id": 1, "method": "events.unsubscribe", "params": {"events": ["UNIT_HEALTH"]}}
```

```json
{"id": 1, "result": {"ok": true, "unsubscribed": 1}}
```

---

## `events.list`

List currently subscribed events.

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `events` | array[string] | currently subscribed event names |

### Example

```json
{"id": 1, "method": "events.list"}
```

```json
{"id": 1, "result": {"events": ["PLAYER_LOGIN", "PLAYER_ENTERING_WORLD"]}}
```

---

## `events.clear`

Clear all event subscriptions.

### Parameters

None.

### Response

```json
{ "ok": true }
```

---

## `events.getHistory`

Retrieve a slice of buffered events from the circular buffer.

### Parameters

| Name | Type | Required | Default | Description |
|---|---|---|---|---|
| `offset` | integer | no | 0 | position in the buffer to start from |
| `count` | integer | no | 50 | number of entries to return |

### Response

| Field | Type | Description |
|---|---|---|
| `total` | integer | total entries currently in the buffer |
| `returned` | integer | number of entries actually returned |
| `offset` | integer | the offset used |
| `entries` | array | event entry objects |

**Entry fields:**

| Field | Type | Description |
|---|---|---|
| `event` | string | WoW event name |
| `args` | json | event arguments (structure depends on the specific WoW event) |
| `time` | integer | timestamp in milliseconds |

### Example

```json
{"id": 1, "method": "events.getHistory", "params": {"offset": 0, "count": 2}}
```

```json
{
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

> [!NOTE]
> The buffer is circular. When full (default 256 entries), old entries are overwritten. Use `events.setMaxBuffer` to configure the size.

---

## `events.clearHistory`

Clear all buffered events.

### Parameters

None.

### Response

```json
{ "ok": true }
```

---

## `events.setMaxBuffer`

Set the maximum size of the event history circular buffer.

### Parameters

| Name | Type | Required | Default | Description |
|---|---|---|---|---|
| `maxBuffer` | integer | no | 256 | new maximum buffer size |

### Response

| Field | Type | Description |
|---|---|---|
| `ok` | boolean | always `true` |
| `maxBuffer` | integer | the new max buffer size |

> [!NOTE]
> Reducing the buffer size does not immediately trim existing entries. They will be overwritten as new events arrive.

---

## `events.isReady`

Check whether the event hook system is initialized and ready.

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `ready` | boolean | |

---

### Available Event Types

The SDK provides two categories of events:

**WoW UI Events** — standard game events fired by `FrameScript_SignalEvent`. Subscribe to any WoW event name. Examples:
- `PLAYER_LOGIN`
- `PLAYER_ENTERING_WORLD`
- `PLAYER_LEAVE_COMBAT`
- `PLAYER_DEAD`
- `COMBAT_LOG_EVENT_UNFILTERED`
- `UNIT_HEALTH`
- `UNIT_POWER_UPDATE`
- `UNIT_NAME_UPDATE`
- `UNIT_AURA`
- `ZONE_CHANGED`

**SDK Custom Events** — pushed to the event pipe automatically without subscription:

| Event | Description | Args |
|---|---|---|
| `client.screenChanged` | game screen changed | `{ "old": <int>, "new": <int> }` where `0=UNKNOWN`, `1=LOGIN`, `2=CHARSELECT`, `3=WORLD` |
| `loading.startup` | loading screen appeared | `{}` |
| `loading.progress` | loading progress update | `{ "progress": <float> }` |
| `loading.finished` | loading completed, world entered | `{}` |

Only these four events push directly to the event pipe. WoW UI events (`PLAYER_LOGIN`, `UNIT_HEALTH`, etc.) require explicit subscription via `events.subscribe` before they appear.

> [!TIP]
> Loading events are polled from memory, not hooked from Lua. `loading.progress` is emitted at most once per 100ms. The progress value is a float between 0.0 and 1.0.

---

### Live Event Streaming (Python CLI)

The test CLI (`tests/wowsdk_cli.py`) can connect to a secondary event pipe at `\\.\pipe\WowGameEvent` to receive events in real-time. This is separate from the main RPC pipe.

To use it, run the CLI and select the **Listen live** option from the Events menu. It will:
1. Connect to `\\.\pipe\WowGameEvent`
2. Stream all incoming events to stdout as JSON
3. Run until interrupted (Ctrl+C)

This is useful for monitoring events while testing other functionality, or for building real-time reactive integrations.

```python
import win32pipe, win32file, json

pipe = win32file.CreateFile(
    r"\\.\pipe\WowGameEvent",
    win32file.GENERIC_READ,
    0, None, win32file.OPEN_EXISTING, 0, None
)
while True:
    _, data = win32file.ReadFile(pipe, 65536)
    print(json.loads(data.decode()))
```
