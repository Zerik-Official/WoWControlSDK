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
| `SDK.Print(text, r, g, b)` | Writes a line to the chat; see [Chat messages](#chat-messages) |
| `SDK.RegisterCommand(names, handler)` | Adds a chat command without globals; see [Chat commands](#chat-commands) |
| `SDK.UnregisterCommand(name)` | Removes a command registered by the same addon |
| `SDK.Expose(name, function)` | Lets the application call the function with `lua.call`; see [Application to addon](#application-to-addon-luacall) |
| `SDK.Save(key, value)`, `SDK.Load(key, default)`, `SDK.Delete(key)`, `SDK.Keys()` | Keep data on disk without SavedVariables; see [Saved data](#saved-data) |
| `SDK.OnDataChanged(function)` | Runs when the application changes a saved key |

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

Expose functions with `SDK.Expose` and call them from the application with `lua.call` and the name of the addon. Every return value comes back as JSON:

```lua
SDK.Expose("Ping", function(text)
    return "pong", text
end)
```

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.call", "params": {"addon": "WoWControlSDKExample", "function": "Ping", "args": ["hi"]}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "result": ["pong", "hi"], "results": ["pong", "hi"]}}
```

Exposed functions are kept inside the SDK. A global table also works with `lua.call` and a path, but any addon or `/run` can then find the functions and call them; `SDK.Expose` leaves nothing in `_G`. Names are Lua identifiers of up to 64 characters, and exposing a name again replaces the function.

The example exposes:

| Function | Description |
| --- | --- |
| `Ping(text)` | Returns `"pong"` and the text |
| `GetStatus()` | Returns the player, level, zone and options as a table |
| `ShowMessage(text)` | Shows the text in a panel and emits `example.messageShown` |
| `HideMessage()` | Hides the panel |
| `SetOption(key, value)` | Changes a known option, saves it and emits `example.optionChanged` |
| `Request(id, action)` | Answers with an `example.response` event that carries the same `id` |

The example leaves no globals: its frame has no name, its options are saved with `SDK.Save` and `/wcs` is registered with `SDK.RegisterCommand`. `/wcs status` checks it.

### Request and response

`Request` shows how to answer work that finishes later, for example after a game event. The application sends an id and waits for the event with the same id:

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.call", "params": {"addon": "WoWControlSDKExample", "function": "Request", "args": [7, "status"]}}
```

```json
{"event": "lua.custom", "args": {"name": "example.response", "addon": "WoWControlSDKExample", "args": [{"id": 7, "ok": true, "data": {"addon": "WoWControlSDKExample", "version": "1.0.0", "player": "PlayerName", "level": 80, "zone": "Dalaran", "options": {"forwardLevelUp": true, "greeting": "Hello from WoWControlSDK"}}}]}}
```

## Other features for addons

| Feature | Example |
| --- | --- |
| Read saved data | [`lua.getStorage`](methods/lua.md#luagetstorage) with `{"addon": "WoWControlSDKExample"}` |
| Change saved data | [`lua.setStorage`](methods/lua.md#luasetstorage) with `{"addon": "WoWControlSDKExample", "key": "options", "value": {"forwardLevelUp": false, "greeting": "Hi"}}` |
| Watch calls of a global function | [`lua.hook`](methods/lua.md#luahook) with `{"function": "ToggleCharacter"}` sends each call as `lua.hooked` |
| Keep a reference to a frame | [`lua.getRef`](methods/lua.md#luagetref) with `{"path": "PlayerFrame"}`, then `lua.call` with `{"ref": 1, "method": "Hide"}` |
| Several calls in one frame | [`lua.batch`](methods/lua.md#luabatch) with a list of operations |
| Addons with access | [`lua.getAddons`](methods/lua.md#luagetaddons) lists the addons that received the object |

Functions called by the application run tainted, like the addon itself. They cannot call protected functions unless the application uses `secure: true`, which is [dangerous](methods/lua.md#taint-and-secure-mode).

## Chat messages

```lua
SDK.Print("|cff33ccff[MyAddon]|r Connected")
SDK.Print("Something failed", 1, 0.2, 0.2)
```

`SDK.Print` writes to the default chat frame with the `AddMessage` method of the chat frame type, which the SDK keeps when it starts. Many chat addons replace `ChatFrame1.AddMessage` to read, log or change every line; those replacements do not see the messages of `SDK.Print`. The color values go from `0` to `1` and default to white, and the text is cut at 4096 characters. It returns `false` when there is no chat frame, for example at the login screen.

## Chat commands

```lua
SDK.RegisterCommand({ "/mytool", "/mt" }, function(message)
    SDK.Emit("mytool.command", message)
end)
```

A command registered the classic way creates `SLASH_MYTOOL1` and a `SlashCmdList.MYTOOL` entry, so any code, including `/run`, can find the handler and call it with any text. `SDK.RegisterCommand` creates neither: the handler stays inside the SDK, and the game only reaches it through the list of typed commands it already keeps (`hash_SlashCmdList`). The SDK runs the handler only when the text of the chat box that sent it really starts with the command and contains the same message, so calling that list entry with any other text does nothing.

| Rule | Detail |
| --- | --- |
| Names | One name or a list of up to 8, each starting with `/` and using letters, digits or `_`, up to 32 characters. Case does not matter |
| Conflicts | Secure commands such as `/cast`, commands of the game or of other addons, and commands of another addon registered with the SDK raise an error |
| Handler | Receives the text after the command, without surrounding spaces |
| Macros | `/mytool` in a macro also runs the handler, because macros send their lines through the chat |
| Removing | `SDK.UnregisterCommand("/mytool")` removes one name; it returns `false` when the name was not registered by the same addon |

Anything that can make the chat send a line, such as a macro or code that fills a chat box and sends it, can still run the command, as if the user typed it. Treat the message as untrusted text.

## Saved data

The game only keeps SavedVariables in globals, so any addon can read and change them, and they are written to disk only when the player logs out or reloads. The SDK keeps an addon's data out of `_G` and writes it to disk as soon as it changes:

```lua
local options = SDK.Load("options", { volume = 1 })
options.volume = 0.5
SDK.Save("options", options)

SDK.OnDataChanged(function(key, value)
    if key == "options" then
        options = value
    end
end)
```

| Function | Description |
| --- | --- |
| `SDK.Save(key, value)` | Saves a copy of the value. `nil` deletes the key |
| `SDK.Load(key, default)` | Returns a copy of the saved value, or `default` when the key is not saved |
| `SDK.Delete(key)` | Deletes the key |
| `SDK.Keys()` | Saved keys, sorted |
| `SDK.OnDataChanged(function(key, value))` | Runs when the application changes a key with [`lua.setStorage`](methods/lua.md#luasetstorage); `value` is `nil` when the key was deleted. Saves of the addon itself do not call it |

| Rule | Detail |
| --- | --- |
| Values | Strings, finite numbers, booleans and tables of them, with string or number keys. Functions, frames, cycles, more than 12 levels or more than 2000 entries in one value raise an error |
| Keys | Strings of 1 to 128 characters; up to 1000 keys per addon |
| Copies | `Save` and `Load` copy the value, so changing a table after saving it does not change the saved data until `Save` runs again |
| Availability | The data is ready when the addon's first file runs, so it can be read at load time without waiting for `ADDON_LOADED` |
| File | `<game folder>\WTF\WoWControlSDK\AddonData\<addon>.json`, written within a quarter of a second after a change. The file is shared by every account and character of that game folder |
| Number keys | Tables are saved as JSON, so a table with keys `1` to `n` comes back as a list and other number keys come back as strings |

Only addons have saved data; scripts loaded with `lua.loadScript` do not.

## Chat commands of the example

| Command | Description |
| --- | --- |
| `/wcs status` | Shows the addon name, that there is no global `SDK` and that the object is read-only |
| `/wcs ping [text]` | Sends `example.ping` |
| `/wcs types` | Sends one event with every supported value type |
| `/wcs flood` | Sends 300 events to show the per-frame limit and `lua.dropped` |

## Scripts without an addon folder

An application can also send Lua code through the pipe with [`lua.loadScript`](methods/lua.md#lualoadscript). The SDK runs it as a virtual addon called `@name`, with its own `SDK` object, so the user does not have to copy a folder to `Interface\AddOns` or restart the game. A script fits up to 1 MiB of code, the request limit of the command pipe.

| | Addon | Script |
| --- | --- | --- |
| Installation | A folder in `Interface\AddOns` with a `.toc` | `lua.loadScript` from the application |
| Name | The folder name | `@name`; names starting with `@` are reserved for scripts |
| `SDK` object | `ns.SDK` | `ns.SDK`, with `SDK.script = true` |
| After `/reload` | The game loads it again | The SDK runs it again when `persist` is `true` |
| SavedVariables, XML and templates from a `.toc` | Yes | No |
| Saved data with `SDK.Save` | Yes | No |
| Can be removed without `/reload` | No | Yes, with `lua.unloadScript` |

A script receives the same arguments as an addon file:

```lua
local name, ns = ...
local SDK = ns.SDK

local frame = CreateFrame("Frame", "MyToolFrame", UIParent)
frame:RegisterEvent("PLAYER_TARGET_CHANGED")
frame:SetScript("OnEvent", function()
    SDK.Emit("tool.target", UnitName("target"))
end)

SDK.RegisterCommand("/mytool", function(text)
    SDK.Emit("tool.command", text)
end)

SDK.OnUnload(function()
    print(name .. " unloaded")
end)
```

### Extra SDK functions for scripts

| Function | Description |
| --- | --- |
| `SDK.OnUnload(function)` | Runs the function when the script is unloaded, before the SDK removes its resources. The last one registered runs first |
| `SDK.HookScript(frame, handler, function)` | Like `frame:HookScript`, but the hook stops running when the script is unloaded. Use it for frames that the script did not create |
| `SDK.IsActive()` | `false` once the script was unloaded |

After unloading, `SDK.Emit`, `SDK.Print`, `SDK.RegisterCommand` and `SDK.Expose` raise an error, so old timers or callbacks cannot act in the script's name. The functions the script exposed are removed.

### What unloading removes

The SDK runs each script in its own environment and records what it creates:

| Resource | When unloaded |
| --- | --- |
| Frames created with `CreateFrame`, and their children | Events unregistered, handlers such as `OnEvent` and `OnUpdate` removed, hidden; a global frame name is cleared |
| Hooks created with `hooksecurefunc` or `SDK.HookScript` | They stop running |
| Globals written by the script, including functions and `SLASH_*` names | They get back the value they had before the script |
| Entries added to `SlashCmdList` | Removed, together with the game's cache of typed commands |
| Commands registered with `SDK.RegisterCommand` | Removed |

The SDK cannot see changes made through other tables, such as new fields in `StaticPopupDialogs`, `frame:HookScript` on a frame the script did not create, or `rawset(_G, ...)`. Undo those in an `SDK.OnUnload` function.

### Timing and taint

- The script runs once the player is in the world, after `PLAYER_LOGIN`, also after every `/reload`. That event has already fired when the script starts, so do not wait for it: the interface, the addons and the player data are ready.
- A script runs tainted, with `@name` as its source, exactly like an addon. Protected functions are blocked, and the game's notice names the script.
- `client.getAddonMemory` reports the memory of each running script in `scripts`.

## Security

- Treat every argument that arrives from the application as untrusted data. Check its type and size before using it.
- Never run received text as code with `loadstring` or `RunScript`.
- Do not expose functions that cast spells, target units, move the character or act on other players.
- The application must also treat `lua.custom` events as untrusted data, because they come from the game.
