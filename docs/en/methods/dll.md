# dll.* methods

Manage the SDK DLL itself: the in-game console overlay, log capture and diagnostics.

The SDK reads WoW's console line list on every frame to capture console output. Most entries come from the game itself, such as Lua errors, debug messages and CVar changes.

## dll.toggleConsole

Toggles the visibility of the in-game console overlay.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | Always `true` |
| `visible` | boolean | Whether the console is visible after the call |

## dll.getInfo

Returns general information about the DLL.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `version` | string | SDK version string |
| `uptimeMs` | integer | Milliseconds since DLL initialization |
| `console` | boolean | Whether the console system is initialized |
| `consoleVisible` | boolean | Whether the console overlay is visible |
| `logCount` | integer | Log entries currently buffered |
| `logMaxEntries` | integer | Maximum log buffer size |
| `logShowInConsole` | boolean | Whether captured logs are echoed to the console |

### Example

```json
{"jsonrpc": "2.0", "id": 1, "method": "dll.getInfo"}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "version": "2.2.0",
    "uptimeMs": 123456,
    "console": true,
    "consoleVisible": false,
    "logCount": 42,
    "logMaxEntries": 1000,
    "logShowInConsole": false
  }
}
```

## dll.getLogs

Returns buffered log entries.

### Parameters

| Name | Type | Required | Default | Description |
| --- | --- | --- | --- | --- |
| `offset` | integer | No | `0` | Position in the buffer to start from |
| `count` | integer | No | `50` | Number of entries to return |

### Response

| Field | Type | Description |
| --- | --- | --- |
| `total` | integer | Log entries in the buffer |
| `returned` | integer | Entries returned in this response |
| `offset` | integer | Offset used by the request |
| `entries` | array | Log entry objects |

**Entry fields:**

| Field | Type | Description |
| --- | --- | --- |
| `text` | string | Log message |
| `style` | integer | Severity: `0` info, `1` warning, `2` error |
| `timestampMs` | integer | Timestamp in milliseconds |

## dll.setLogConfig

Configures log capture.

### Parameters

| Name | Type | Required | Default | Description |
| --- | --- | --- | --- | --- |
| `maxEntries` | integer | No | Current value, `1000` at start | Maximum number of buffered entries |
| `showInConsole` | boolean | No | Current value, `false` at start | Echo captured logs to the in-game console |

Omitted parameters keep their current value. When `showInConsole` turns on, or the console opens later, the buffered messages are printed first, so the messages from the game startup are not lost. The console is written from a separate thread, so printing never slows the game down.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | Always `true` |
| `maxEntries` | integer | Configured maximum |
| `showInConsole` | boolean | Configured echo setting |

!!! note
    Reducing `maxEntries` does not trim the buffer immediately. Old entries are overwritten as new ones arrive.
