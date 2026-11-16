# Addon integration

An addon can talk to the application that controls the game through the SDK. The addon sends events with `SDK.Emit`, and the application calls the addon's functions with [`lua.call`](methods/lua.md#luacall).

The repository includes a complete example in [`tools/addons/WoWControlSDKExample`](https://github.com/Zerik-Official/WoWControlSDK/tree/main/tools/addons/WoWControlSDKExample). It uses every feature on this page and can be copied as a starting point.

## Install the example

1. Copy the `WoWControlSDKExample` folder to `World of Warcraft\Interface\AddOns\`.
2. Start the game with the SDK injected, for example from the [desktop client](operations/cli.md).
3. Enter the world. The chat shows `Connected as WoWControlSDKExample`, and the event pipe receives `lua.custom` with `name: example.ready`.

## Get the SDK object

Add this line to the addon's `.toc` file:

```text
## X-WoWControlSDK: 1
```

When the game loads the addon from `Interface\AddOns`, the SDK places a read-only `SDK` object in the private table that every file of the addon receives:

```lua
local addonName, ns = ...
local SDK = ns.SDK

if not SDK then
    return
end
```

`ns.SDK` is `nil` when the SDK is not injected, so the addon keeps working without it.

| Field | Description |
| --- | --- |
| `SDK.addon` | Name of the addon that received the object |
| `SDK.Emit(name, ...)` | Sends an event to the application |

### Who can reach the object

There is no global `SDK`. Only files loaded by the game from the addon's folder receive the object. Code from other sources cannot reach it:

- `/run`, `/script`, `RunScript` and `loadstring`.
- Lua sent by the server, including Warden packets.
- Addon messages from other players (`SendAddonMessage`) and chat links.

Keep the object in a local and never store it in a global or in a table that other code can read, or that code will be able to send events in the addon's name.

The application can also limit the bridge to specific addons with [`lua.setAllowedAddons`](methods/lua.md#luasetallowedaddons). Addons outside the list do not receive the object and their events are discarded.

## Addon to application: `SDK.Emit`

```lua
SDK.Emit("example.ready", { player = UnitName("player"), level = UnitLevel("player") })
```

The application receives a [`lua.custom`](methods/events.md#sdk-event-payloads) event:

```json
{"event": "lua.custom", "args": {"name": "example.ready", "addon": "WoWControlSDKExample", "args": [{"player": "PlayerName", "level": 80}]}}
```

| Rule | Value |
| --- | --- |
| Event name | String of 1 to 64 characters. Anything else raises a Lua error, so wrap `SDK.Emit` in `pcall` when the name is not fixed |
| Arguments | Strings, numbers, booleans, `nil` and tables, converted as described in [Values](methods/lua.md#values). Functions and userdata become `{"$type": ...}` markers |
| Size | Up to 64 arguments, 3 levels of nested tables and 200 table entries per event. The event carries `truncated: true` when a limit cut it |
| Rate | Up to 256 events per frame. The excess is discarded and reported once as `lua.dropped` with a `count` |
| Delivery | On the next frame, in the order they were emitted |

Prefix event names with the addon name, such as `example.ready`, so the application can tell addons apart. `addon` is always set by the SDK and cannot be faked by the addon.

## Application to addon: `lua.call`

Expose functions in a global table and call them from the application. Every return value comes back as JSON:

```lua
WoWControlSDKExample = {}

function WoWControlSDKExample.Ping(text)
    return "pong", text
end
```

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.call", "params": {"function": "WoWControlSDKExample.Ping", "args": ["hi"]}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "result": ["pong", "hi"], "results": ["pong", "hi"]}}
```

The example exposes:

| Function | Description |
| --- | --- |
| `WoWControlSDKExample.Ping(text)` | Returns `"pong"` and the text |
| `WoWControlSDKExample.GetStatus()` | Returns the player, level, zone and options as a table |
| `WoWControlSDKExample.ShowMessage(text)` | Shows the text in a panel and emits `example.messageShown` |
| `WoWControlSDKExample.HideMessage()` | Hides the panel |
| `WoWControlSDKExample.SetOption(key, value)` | Changes a known option and emits `example.optionChanged` |
| `WoWControlSDKExample.Request(id, action)` | Answers with an `example.response` event that carries the same `id` |

### Request and response

`Request` shows how to answer work that finishes later, for example after a game event. The application sends an id and waits for the event with the same id:

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.call", "params": {"function": "WoWControlSDKExample.Request", "args": [7, "status"]}}
```

```json
{"event": "lua.custom", "args": {"name": "example.response", "addon": "WoWControlSDKExample", "args": [{"id": 7, "ok": true, "data": {"addon": "WoWControlSDKExample", "version": "1.0.0", "player": "PlayerName", "level": 80, "zone": "Dalaran", "options": {"forwardLevelUp": true, "greeting": "Hello from WoWControlSDK"}}}]}}
```

## Other features for addons

| Feature | Example |
| --- | --- |
| Read saved variables | [`lua.getGlobal`](methods/lua.md#luagetglobal) with `{"name": "WoWControlSDKExampleDB.options"}` |
| Change saved variables | [`lua.setGlobal`](methods/lua.md#luasetglobal) with `{"name": "WoWControlSDKExampleDB.options.greeting", "value": "Hi"}` |
| Watch calls | [`lua.hook`](methods/lua.md#luahook) with `{"function": "WoWControlSDKExample.ShowMessage"}` sends each call as `lua.hooked` |
| Keep a reference to a frame | [`lua.getRef`](methods/lua.md#luagetref) with `{"path": "WoWControlSDKExampleFrame"}`, then `lua.call` with `{"ref": 1, "method": "Hide"}` |
| Several calls in one frame | [`lua.batch`](methods/lua.md#luabatch) with a list of operations |
| Addons with access | [`lua.getAddons`](methods/lua.md#luagetaddons) lists the addons that received the object |

Functions called by the application run tainted, like the addon itself. They cannot call protected functions unless the application uses `secure: true`, which is [dangerous](methods/lua.md#taint-and-secure-mode).

## Chat commands of the example

| Command | Description |
| --- | --- |
| `/wcs status` | Shows the addon name, that there is no global `SDK` and that the object is read-only |
| `/wcs ping [text]` | Sends `example.ping` |
| `/wcs types` | Sends one event with every supported value type |
| `/wcs flood` | Sends 300 events to show the per-frame limit and `lua.dropped` |

## Security

- Treat every argument that arrives from the application as untrusted data. Check its type and size before using it.
- Never run received text as code with `loadstring` or `RunScript`.
- Do not expose functions that cast spells, target units, move the character or act on other players.
- The application must also treat `lua.custom` events as untrusted data, because they come from the game.
