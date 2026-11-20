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

Returns general information about the DLL, including the commit it was built from. It is the only method that also works without the game: see [Reading the DLL without the game](#reading-the-dll-without-the-game).

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `version` | string | SDK version string |
| `commit` | object or `null` | Commit the DLL was built from. `null` when it was built outside a Git repository |
| `inGame` | boolean | `true` when the DLL runs inside the game |
| `uptimeMs` | integer | Milliseconds since DLL initialization |
| `console` | boolean | Whether the console system is initialized |
| `consoleVisible` | boolean | Whether the console overlay is visible |
| `logCount` | integer | Log entries currently buffered |
| `logMaxEntries` | integer | Maximum log buffer size |
| `logShowInConsole` | boolean | Whether captured logs are echoed to the console |

`uptimeMs`, `console`, `consoleVisible` and the `log*` fields are only present inside the game.

**`commit` fields:**

| Field | Type | Description |
| --- | --- | --- |
| `hash` | string | Full commit hash |
| `shortHash` | string | Abbreviated commit hash |
| `date` | string | Commit date in ISO 8601 with seconds and time zone offset, for example `2026-11-19T12:55:26-05:00` |
| `timestamp` | integer | Commit date in seconds since 1970-01-01 UTC |
| `dirty` | boolean | `true` when `src/`, `cmake/` or `CMakeLists.txt` had uncommitted changes, so the DLL does not match the commit exactly |

The commit and its date are written into the DLL at build time, so two builds of the same commit return the same values. The date is the commit date, not the build date.

### Example

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "dll.getInfo"}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "version": "2.3.0",
    "commit": {
      "hash": "4f09723ebef967c9c66e5ed16a19eb7a42a14d97",
      "shortHash": "4f09723",
      "date": "2026-11-19T12:55:26-05:00",
      "timestamp": 1795110926,
      "dirty": false
    },
    "inGame": true,
    "uptimeMs": 123456,
    "console": true,
    "consoleVisible": false,
    "logCount": 42,
    "logMaxEntries": 1000,
    "logShowInConsole": false
  }
}
```

### Reading the DLL without the game

The DLL only starts the SDK when the game loads it. Loaded by any other program, it starts no hooks, pipes or threads, and only answers through its exported function:

```cpp
extern "C" int __cdecl WoWControlSDK_Invoke(const char* request, char* response, int responseSize);
```

`request` is a JSON-RPC request. The function writes the JSON-RPC response to `response`, ending in a null character, and returns its size including that character. When `responseSize` is too small, it writes nothing and returns the size it needs, so a first call with `nullptr` and `0` gives the size of the buffer. Only `dll.getInfo` is available; any other method returns the error `-32601`.

The DLL is 32-bit, so only a 32-bit process can load it. The injector does it with [`--call`](../injection.md#reading-the-dll-without-the-game):

```powershell
.\WowInjector.exe --dll .\WoWControlSDK.dll --call dll.getInfo
```

```json
{"id":1,"jsonrpc":"2.0","result":{"commit":{"date":"2026-11-19T12:55:26-05:00","dirty":false,"hash":"4f09723ebef967c9c66e5ed16a19eb7a42a14d97","shortHash":"4f09723","timestamp":1795110926},"inGame":false,"version":"2.3.0"}}
```

A 64-bit program can read the version without loading the DLL: the Windows version resource has `FileVersion`, `ProductVersion` (version, short commit, `-dirty` when it had uncommitted changes, and date, for example `2.3.0 (4f09723, 2026-11-19T12:55:26-05:00)`), `Commit` and `CommitDate`. They can be read with `GetFileVersionInfo` and `VerQueryValue`, and `ProductVersion` appears in **Properties → Details** of the file.

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
    Reducing `maxEntries` trims the oldest entries immediately.
