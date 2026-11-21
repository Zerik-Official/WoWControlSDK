# Glossary

## Transport and protocol

**JSON-RPC 2.0**
:   A small remote procedure call protocol that encodes requests and responses as JSON objects. Every request names a `method` and carries optional `params`; every response carries `result` or `error`.

**Named pipe**
:   A Windows inter-process communication channel identified by a path such as `\\.\pipe\WowGameCommand`. The SDK creates the server end and a client application opens the client end.

**Command pipe**
:   `\\.\pipe\WowGameCommand`. Accepts one request per connection, dispatches it and returns one response.

**Event pipe**
:   `\\.\pipe\WowGameEvent`. A duplex pipe that streams newline-delimited JSON events to one client at a time. When authentication is enabled, the client first sends a `{"auth":"<token>"}` handshake and the SDK answers with `auth.ok` or `auth.failed` before streaming.

**Message mode**
:   A named pipe mode that preserves write boundaries. The command pipe uses it so one write is one request. The event pipe uses byte mode instead.

**Notification**
:   A JSON-RPC request without an `id`. The SDK expects a request identifier and always returns a response, so use a numeric `id`.

## Injection and hooks

**DLL injection**
:   Loading a dynamic library into another process. The injector writes the DLL path into `Wow.exe` and calls `LoadLibrary` through a remote thread.

**Freeze-sync**
:   The injection strategy used by `WowInjector.exe`: start the game suspended, inject the DLL, wait for the command pipe and only then resume the game.

**VTable hook**
:   Gives one game object a copy of its virtual table with a slot replaced. The game code stays untouched. The SDK combines it with game scheduler callbacks, Windows event and CBT hooks and debug register breakpoints instead of patching game functions.

**Offset**
:   The numeric address or field displacement of a structure inside the game binary. Every offset lives in `offsets/` as a `constexpr uintptr_t`.

**Lua VM**
:   The Lua interpreter embedded in WoW. `lua.*` methods execute code in this VM and serialize booleans, numbers and strings as JSON; tables become arrays or objects, and other types are represented with `$type` markers.

## Game state

**Glue screen**
:   A non-gameplay screen such as the login screen or the character select screen. `client.getScreen` reports `login`, `charselect`, `world` or the raw glue name.

**Unit token**
:   A string that identifies a unit, for example `player`, `target`, `focus`, `mouseover`, `party1` or `raid1`. A GUID string is also accepted.

**GUID**
:   A 64-bit game identifier returned as a `[high, low]` array of two 32-bit unsigned integers.

**World position**
:   Game coordinates in the current map, returned as `{x, y, z}`. Works for every unit.

**Map position**
:   Normalized coordinates used by the map and minimap. The game only resolves them for party and raid members; other units return zeros.

**State cache**
:   The per-frame snapshot of player, world and screen state maintained by `runtime/state/StateCache`. The `player.*` and `world.*` domains read it instead of blocking on the game thread.

**EventBus**
:   The in-process publish/subscribe bus in `runtime/events/EventBus.h`. It is used internally and is not exposed to RPC clients.

**EventPipe**
:   The external event stream in `runtime/events/EventPipe.h` that backs `\\.\pipe\WowGameEvent` and the circular history buffer.

## Windows and delivery

**HWND**
:   A Windows window handle. The SDK returns it as an unsigned integer through `window.getHandle`.

**Strict build**
:   `mkdocs build --strict`, which fails on any warning, including missing pages in the navigation and broken internal links.

**Manifest**
:   The `manifest.json` file generated for a release. It records the schema version, the tag, the commit and the SHA-256 hash and size of each binary.

**Desktop client**
:   WoWControl Client, the customtkinter application in `tools/wowsdk_client/`. It calls every RPC method from generated forms, launches the game with a session token and shows live events. Releases ship it as `WoWControlClient-<version>.zip`.
