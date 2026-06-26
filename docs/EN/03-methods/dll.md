# dll.* Methods

Manage the SDK DLL itself — console overlay, logging, and diagnostics.

The SDK hooks WoW's internal `PrintMessage` function at `0x00765270` to capture all game console output. Most captured logs come from WoW itself — Lua errors, debug messages, CVar changes, etc.

---

## `dll.toggleConsole`

Toggle the visibility of the in-game console overlay.

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `ok` | boolean | always `true` |
| `visible` | boolean | whether the console is now visible |

---

## `dll.getInfo`

Returns general information about the SDK DLL.

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `version` | string | SDK version string |
| `uptimeMs` | integer | milliseconds since DLL initialization |
| `console` | boolean | whether the console system is initialized |
| `consoleVisible` | boolean | whether the console overlay is currently shown |
| `logCount` | integer | total log entries currently buffered |
| `logMaxEntries` | integer | maximum log buffer size |
| `logShowInConsole` | boolean | whether log messages echo to the console |

### Example

```json
{"id": 1, "method": "dll.getInfo"}
```

```json
{
  "id": 1,
  "result": {
    "version": "1.0.0",
    "uptimeMs": 123456,
    "console": true,
    "consoleVisible": false,
    "logCount": 42,
    "logMaxEntries": 256,
    "logShowInConsole": false
  }
}
```

---

## `dll.getLogs`

Retrieve buffered log messages from the SDK's internal log capture.

### Parameters

| Name | Type | Required | Default | Description |
|---|---|---|---|---|
| `offset` | integer | no | 0 | position in the log buffer to start from |
| `count` | integer | no | 50 | number of entries to return |

### Response

| Field | Type | Description |
|---|---|---|
| `total` | integer | total log entries in buffer |
| `returned` | integer | number of entries returned |
| `offset` | integer | the offset used |
| `entries` | array | log entry objects |

**Entry fields:**

| Field | Type | Description |
|---|---|---|
| `text` | string | log message text |
| `style` | integer | severity (0=info, 1=warning, 2=error) |
| `timestampMs` | integer | timestamp in milliseconds |

---

## `dll.setLogConfig`

Configure the log capture system.

### Parameters

| Name | Type | Required | Default | Description |
|---|---|---|---|---|
| `maxEntries` | integer | no | 256 | maximum log entries to buffer |
| `showInConsole` | boolean | no | false | echo logs to in-game console |

### Response

| Field | Type | Description |
|---|---|---|
| `ok` | boolean | always `true` |
| `maxEntries` | integer | configured max entries |
| `showInConsole` | boolean | configured show-in-console setting |

> [!NOTE]
> Reducing `maxEntries` does not immediately trim the buffer. Old entries will persist until overwritten by new ones.
