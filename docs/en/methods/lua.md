# lua.* methods

Run code, call functions, read and write values, watch calls and manage the addon bridge in WoW's Lua VM.

## How Lua calls run

| Topic | Behavior |
| --- | --- |
| Thread | Every `lua.*` method runs on the game thread at the start of the next frame, so the response arrives after at most one frame. |
| Request size | The command pipe accepts requests of up to 1 MiB, enough for long Lua code. Use [`lua.loadScript`](#lualoadscript) for code that must survive `/reload`. |
| Errors | A failed call returns `{"ok": false, "error": "..."}`. Lua errors also carry `stack` with the Lua stack trace. |
| Blocked actions | When the game blocks a protected function, the call returns `ok: false` with a `blocked` list. See [Taint and secure mode](#taint-and-secure-mode). |
| Script errors | Errors raised by any Lua script of the game or an addon are sent to the event pipe as [`lua.error`](events.md#sdk-event-payloads), even when an error addon such as BugGrabber is installed. |
| Reload | `/reload` and logging out create a new Lua VM. Refs and hooks are lost and must be created again. |

## Taint and secure mode

WoW marks code that does not come from Blizzard as *tainted*. Tainted code cannot call protected functions such as `CastSpellByName`, `TargetUnit` or `UseAction`. When it tries, the game raises `ADDON_ACTION_BLOCKED` or `ADDON_ACTION_FORBIDDEN` and the function does nothing.

By default the SDK runs Lua with the taint source `WoWControlSDK`, the same way the game runs an addon. Protected functions are blocked, and every value the SDK writes, such as a global set with `lua.setGlobal`, is tainted too.

`lua.execute`, `lua.evaluate` and `lua.call` accept `"secure": true` to run the code as Blizzard code, without taint.

!!! danger "Secure mode is dangerous"
    Secure code can call every protected function: cast spells, target units, use items and move the character. Only use `secure: true` with code that you wrote and control, never with code or arguments that come from an addon, a chat message, the server or another program. Blizzard's interface trusts values written by secure code, so a mistake can break action bars, raid frames or combat behavior until the next `/reload`.

When the game blocks the SDK:

- The response is `{"ok": false, "error": "blocked by the game: <function>", "blocked": [...]}`, even if the rest of the code ran.
- The event pipe receives a [`lua.blocked`](events.md#sdk-event-payloads) event.
- The game's "interface action failed" notice is hidden. Use [`lua.setBlockedNotice`](#luasetblockednotice) to show it again. Blocks caused by real addons are never hidden.

`ok: true` only means that the code ran. For example, `CastSpellByName` on a spell in cooldown returns `ok: true` because the game accepted the call, even though the spell is not cast.

## Values

Lua values are converted to JSON as follows:

| Lua | JSON |
| --- | --- |
| `nil` | `null` |
| boolean | boolean |
| number | number; integers up to 2^53 are written without decimals |
| string | string; invalid UTF-8 bytes are replaced |
| table with keys `1..n` | array |
| other table | object; number keys become strings |
| function | `{"$type": "function"}` |
| userdata, such as a frame handle | `{"$type": "userdata"}` |
| coroutine | `{"$type": "thread"}` |
| table already being converted | `{"$type": "table", "$cycle": true}` |
| table past the depth or entry limit | `{"$type": "table", "$truncated": true}` |

When a limit cuts a table, the response also carries `"truncated": true`. The methods that return values accept these limits:

| Name | Type | Default | Range | Description |
| --- | --- | --- | --- | --- |
| `maxDepth` | integer | `4` | `0` to `16` | Nested table levels to convert |
| `maxEntries` | integer | `500` | `1` to `5000` | Total table entries to convert per response |

Arguments sent to Lua accept the same JSON values: arrays and objects become tables, up to 16 levels deep. `{"$ref": n}` passes the value stored in a [ref](#luagetref).

### Result fields

Methods that return Lua values answer with:

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | `true` when the code ran without errors or blocked actions |
| `result` | any | `null` without values, the value itself with one value, or an array with several values |
| `results` | array | Every returned value, always as an array |
| `truncated` | boolean | Present and `true` when a limit cut a table |
| `error` | string | Present when `ok` is `false` |
| `stack` | string | Lua stack trace, present for Lua errors |
| `blocked` | array | Blocked actions, present when the game blocked a protected function |

Each `blocked` entry is `{"event": "ADDON_ACTION_FORBIDDEN" | "ADDON_ACTION_BLOCKED", "function": "<function>"}`.

Prefer `results` in new code. `result` keeps the shape used before SDK 2.2.

## Paths

`lua.call`, `lua.getGlobal`, `lua.setGlobal`, `lua.createNamespace`, `lua.getRef` and the hook methods take a path instead of Lua code:

| Path | Meaning |
| --- | --- |
| `UnitName` | The global `UnitName` |
| `MyAddonDB.options.volume` | Field `volume` of `options` of the global `MyAddonDB` |
| `RAID_CLASS_COLORS.MAGE` | Field `MAGE` of a global table |
| `MyAddonDB.list.3` | Index `3` of a table |
| `PlayerFrame:Hide` | Method `Hide` of `PlayerFrame`, called with `PlayerFrame` as `self`; only in `lua.call` and the hook methods |

A path starts with a Lua identifier, continues with identifiers or indexes of up to 9 digits separated by `.`, and is at most 256 characters long. Paths are never run as code.

## lua.execute

Runs Lua code and discards its return values.

### Parameters

| Name | Type | Required | Default | Description |
| --- | --- | --- | --- | --- |
| `code` | string | Yes | | Lua code to run |
| `chunkName` | string | No | `sdk_exec` | Name shown in error messages |
| `secure` | boolean | No | `false` | Run without taint. See [Taint and secure mode](#taint-and-secure-mode) |

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | `true` when the code ran and nothing was blocked |
| `error` | string | `code is required`, or the blocked function |
| `blocked` | array | Blocked actions |

Lua errors are not returned: the game shows them like any other script error. Use `lua.evaluate` to receive them.

### Example

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "lua.execute", "params": {"code": "CastSpellByName('Attack')"}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": false, "error": "blocked by the game: CastSpellByName()", "blocked": [{"event": "ADDON_ACTION_FORBIDDEN", "function": "CastSpellByName()"}]}}
```

## lua.evaluate

Runs Lua code and returns every value it returns.

### Parameters

| Name | Type | Required | Default | Description |
| --- | --- | --- | --- | --- |
| `code` | string | Yes | | Lua code; use `return` to send values back |
| `chunkName` | string | No | `sdk_eval` | Name shown in errors and in `stack` |
| `secure` | boolean | No | `false` | Run without taint |
| `maxDepth` | integer | No | `4` | See [Values](#values) |
| `maxEntries` | integer | No | `500` | See [Values](#values) |

### Response

See [Result fields](#result-fields).

### Example

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "lua.evaluate", "params": {"code": "return UnitName('player'), UnitLevel('player'), {UnitClass('player')}"}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "result": ["PlayerName", 80, ["Mage", "MAGE"]], "results": ["PlayerName", 80, ["Mage", "MAGE"]]}}
```

A Lua error returns the message and the stack:

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": false, "error": "sdk_eval:1: attempt to call global 'Nope' (a nil value)", "stack": "sdk_eval:1: in main chunk\n"}}
```

## lua.call

Calls a Lua function by path or by ref without writing Lua code. Arguments are passed as values, so they are never run as code.

### Parameters

| Name | Type | Required | Default | Description |
| --- | --- | --- | --- | --- |
| `function` | string | Yes, without `ref` | | Path of the function, such as `GetItemInfo` or `PlayerFrame:Hide` |
| `ref` | integer | Yes, without `function` | | Ref of a function, or of an object when `method` is set |
| `method` | string | No | | With `ref`, name of the method to call on the object, which is passed as `self` |
| `args` | array | No | `[]` | Arguments, converted as described in [Values](#values) |
| `secure` | boolean | No | `false` | Run without taint |
| `maxDepth` | integer | No | `4` | See [Values](#values) |
| `maxEntries` | integer | No | `500` | See [Values](#values) |

### Response

See [Result fields](#result-fields).

### Example

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "lua.call", "params": {"function": "UnitClass", "args": ["player"]}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "result": ["Mage", "MAGE"], "results": ["Mage", "MAGE"]}}
```

## lua.getGlobal

Reads the value at a path.

### Parameters

| Name | Type | Required | Default | Description |
| --- | --- | --- | --- | --- |
| `name` | string | Yes | | Path to read, such as `MyAddonDB.options` |
| `maxDepth` | integer | No | `4` | See [Values](#values) |
| `maxEntries` | integer | No | `500` | See [Values](#values) |

### Response

See [Result fields](#result-fields). A missing value returns `ok: true` and `result: null`. A value in the middle of the path that is not a table returns an error such as `MyAddonDB.options is not a table`.

### Example

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "lua.getGlobal", "params": {"name": "RAID_CLASS_COLORS.MAGE"}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "result": {"r": 0.41, "g": 0.8, "b": 0.94}, "results": [{"r": 0.41, "g": 0.8, "b": 0.94}]}}
```

## lua.setGlobal

Writes a value at a path. The tables in the path must already exist.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `name` | string | Yes | Path to write, such as `MyAddonDB.options.volume` |
| `value` | any | No | Value to write, converted as described in [Values](#values). Omitted or `null` writes `nil` |

### Response

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true}}
```

### Example

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "lua.setGlobal", "params": {"name": "MyAddonDB.options", "value": {"volume": 0.5, "channels": ["GUILD", "PARTY"]}}}
```

!!! note
    The written value is tainted. Do not overwrite globals that Blizzard's interface reads, such as frames or configuration tables.

## lua.createNamespace

Writes an empty table at a path. Use it to create a table before writing fields with `lua.setGlobal`.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `name` | string | Yes | Path of the new table, such as `MyTool` or `MyTool.cache` |

### Response

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true}}
```

!!! warning
    A value that already exists at the path is replaced.

## lua.getRef

Stores the value at a path and returns a number that refers to it. Use refs to call methods on frames, or to pass the same table to several calls without looking it up again.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `path` | string | Yes | Path of the value to keep, such as `PlayerFrame` |

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | `true` when the ref was created |
| `ref` | integer | Number to pass as `ref` or as `{"$ref": n}` |
| `type` | string | Lua type of the value, such as `table` or `function` |

### Example

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "lua.getRef", "params": {"path": "PlayerFrame"}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "ref": 1, "type": "table"}}
```

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 2, "method": "lua.call", "params": {"ref": 1, "method": "SetAlpha", "args": [0.5]}}
```

!!! note
    The SDK keeps up to 256 refs. A ref keeps its value alive until `lua.releaseRef` or the next `/reload`, so release the ones that are no longer needed.

## lua.releaseRef

Releases a ref created with `lua.getRef`.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `ref` | integer | Yes | Ref to release |

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | `true` when the call ran |
| `released` | boolean | `false` when the ref did not exist |

## lua.hook

Watches a Lua function. Every call made by the game or by an addon is sent to the event pipe as a [`lua.hooked`](events.md#sdk-event-payloads) event with its arguments. The original function keeps working unchanged.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `function` | string | Yes | Path of the function, such as `AcceptQuest` or `StaticPopup1Button1:Click` |

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | `true` when the hook is active |
| `function` | string | The path |
| `hooked` | boolean | Always `true` |

### Example

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "lua.hook", "params": {"function": "AcceptQuest"}}
```

The game cannot remove a hook. `lua.unhook` stops the events, and `lua.hook` on the same path starts them again.

## lua.unhook

Stops the events of a hooked function.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `function` | string | Yes | Path used in `lua.hook` |

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | `true` when the call ran |
| `function` | string | The path |
| `unhooked` | boolean | `false` when the path was not hooked |

## lua.getHooks

Lists the active hooks.

### Response

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "hooks": ["AcceptQuest", "StaticPopup1Button1:Click"]}}
```

## lua.batch

Runs several `lua.*` methods in the same frame and returns every result in one response. The game state cannot change between the operations.

### Parameters

| Name | Type | Required | Default | Description |
| --- | --- | --- | --- | --- |
| `operations` | array | Yes | | 1 to 32 objects with `method` and `params`. Any `lua.*` method except `lua.batch` |
| `stopOnError` | boolean | No | `false` | Stop at the first operation whose result has `error` |

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | `true` when the batch ran, even if an operation failed |
| `results` | array | The result of each operation, in order |
| `completed` | integer | Number of operations that ran |
| `stopped` | boolean | Present and `true` when `stopOnError` stopped the batch |

### Example

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "lua.batch", "params": {"operations": [
  {"method": "lua.call", "params": {"function": "UnitName", "args": ["player"]}},
  {"method": "lua.call", "params": {"function": "GetMoney"}},
  {"method": "lua.getGlobal", "params": {"name": "MyAddonDB.options"}}
]}}
```

## lua.setAllowedAddons

Limits the [addon bridge](../addons.md) to a list of addons. Addons outside the list do not receive the `SDK` object the next time they load, and their events are discarded from that moment.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `addons` | array[string] or `null` | Yes | Addon folder names, or `null` to allow every addon that opts in |

### Response

Same as [`lua.getAddons`](#luagetaddons).

### Example

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "lua.setAllowedAddons", "params": {"addons": ["WoWControlSDKExample"]}}
```

!!! tip
    Call it right after injecting, before the game loads the addons, so that only the listed addons receive the object.

## lua.getAddons

Lists the addons that received the `SDK` object and the current allowlist.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | Always `true` |
| `granted` | array[string] | Addons that received the object since the last `/reload` |
| `allowed` | array[string] or `null` | The allowlist, or `null` when every addon that opts in is allowed |

## lua.loadScript

Loads Lua code sent through the pipe as a virtual addon. The script receives its own `SDK` object, like an addon that declares `X-WoWControlSDK`, so it can send events, and everything it creates can be removed later with `lua.unloadScript`. Use it to deploy logic on demand without installing an addon folder or restarting the game. The [addon integration guide](../addons.md#scripts-without-an-addon-folder) explains how to write one.

### Parameters

| Name | Type | Required | Default | Description |
| --- | --- | --- | --- | --- |
| `name` | string | Yes | | 1 to 48 letters, digits, `_`, `-`, `.` or `/`. The script is called `@name` |
| `code` | string | Yes | | Lua code. It receives `name, ns` like an addon file, with the SDK object in `ns.SDK` |
| `persist` | boolean | No | `true` | Run the script again after every `/reload` |
| `replace` | boolean | No | `true` | Unload a script with the same name first; `false` returns an error instead |

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | `true` when the script ran or was stored |
| `name` | string | Name of the script, with `@` |
| `bytes` | integer | Size of the code |
| `persist` | boolean | Whether it runs again after `/reload` |
| `loaded` | boolean | Whether it is running now |
| `pending` | boolean | `true` when it was stored to run once the world loads |
| `resources` | object or `null` | `{"frames", "hooks", "globals", "slashCommands"}` created so far |

### Example

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "lua.loadScript", "params": {"name": "MyTool", "code": "local name, ns = ...\nlocal frame = CreateFrame('Frame')\nframe:RegisterEvent('PLAYER_TARGET_CHANGED')\nframe:SetScript('OnEvent', function() ns.SDK.Emit('tool.target', UnitName('target')) end)"}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "name": "@MyTool", "bytes": 190, "persist": true, "loaded": true, "pending": false, "resources": {"frames": 1, "hooks": 0, "globals": 0, "slashCommands": 0}}}
```

- A script runs once the player is in the world, after `PLAYER_LOGIN`, so the interface and every addon are already loaded. Sent before that, for example from the login screen or during a loading screen, it is stored with `pending: true` and runs as soon as the player is in the world. A [`lua.script`](events.md#sdk-event-payloads) event reports the result.
- A Lua error returns `ok: false` with `error` and `stack`, and everything the script created before the error is removed.
- With an allowlist set by `lua.setAllowedAddons`, the list must include `@name`.
- Persisted scripts live in the SDK's memory, not on disk: they survive `/reload` and logging out, but not closing the game.

## lua.unloadScript

Stops a script and removes what it created: its frames stop receiving events and scripts and are hidden, its hooks stop running, the globals it wrote get their previous value back and its slash commands disappear. Callbacks registered with `SDK.OnUnload` run first.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `name` | string | Yes | Script name, with or without `@` |

### Response

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "name": "@MyTool", "removed": {"frames": 1, "hooks": 0, "globals": 0, "slashCommands": 0}}}
```

`removed` counts zero for a script that was pending. An unknown name returns `{"ok": false, "error": "script not found"}`.

!!! note
    WoW cannot delete frames. An unloaded frame stays hidden and silent, and its memory is freed at the next `/reload`.

## lua.getScripts

Lists the loaded and pending scripts.

### Response

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "scripts": [{"name": "@MyTool", "bytes": 190, "persist": true, "loaded": true, "resources": {"frames": 1, "hooks": 0, "globals": 0, "slashCommands": 0}}]}}
```

`resources` is `null` for a script that is not running. The memory used by each script appears in [`client.getAddonMemory`](client.md#clientgetaddonmemory).

## lua.setBlockedNotice

Chooses whether the game shows its notice when it blocks a protected function called by the SDK.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `show` | boolean | Yes | `true` to show the notice, `false` to hide it |

### Response

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "show": true}}
```

The notice is hidden by default. The response and the `lua.blocked` event report the block either way.

## Changes in SDK 2.2

| Change | Before | Now |
| --- | --- | --- |
| Taint | Lua ran as Blizzard code | Lua runs tainted unless `secure: true` |
| `lua.execute` | Always `ok: true` | `ok: false` when the game blocks an action |
| Tables, functions and userdata | `null` | Converted as described in [Values](#values) |
| `lua.getGlobal`, `lua.setGlobal`, `lua.createNamespace` | `name` was a global name | `name` is a [path](#paths); names that are not valid paths are rejected |
| `lua.setGlobal` | Only booleans, numbers and strings | Any JSON value |
| Thread | Pipe thread | Game thread, at the next frame |
| Request size | 4096 bytes | 1 MiB |
| Scripts | Only through an addon folder | [`lua.loadScript`](#lualoadscript), removable with `lua.unloadScript` |
