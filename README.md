<h1 align="center">WoWControlSDK</h1>

<p align="center">
  <img src="https://img.shields.io/badge/C++17-111111?style=for-the-badge&logo=cplusplus">
  <img src="https://img.shields.io/badge/CMake-111111?style=for-the-badge&logo=cmake">
  <img src="https://img.shields.io/badge/Python-111111?style=for-the-badge&logo=python">
  <img src="https://img.shields.io/badge/Windows-111111?style=for-the-badge&logo=windows">
</p>

<p align="center">
  <img src="https://img.shields.io/badge/JSON--RPC_2.0-444444?style=for-the-badge">
  <img src="https://img.shields.io/badge/Named_Pipes-444444?style=for-the-badge">
  <img src="https://img.shields.io/badge/Detours-444444?style=for-the-badge">
  <img src="https://img.shields.io/badge/Lua_VM-444444?style=for-the-badge">
</p>

<p align="center">
  <img src=".github/images/preview.png" alt="WoWControlSDK preview"/>
</p>

<p align="center">
  <a href="./README.es.md">Español</a> | <strong>English</strong>
</p>

> [!CAUTION]
> WoWControlSDK is not a hacking, cheating or botting tool. It is intended for legitimate integrations such as launchers, stream overlays, Discord Rich Presence and UI automation. Many of its methods can also support botting, so use it responsibly. The author is not responsible for account suspensions or bans caused by misuse.

WoWControlSDK is a lightweight injectable DLL and SDK for **World of Warcraft 3.3.5a (build 12340)**. It exposes the internal methods of the game through **JSON-RPC 2.0** over Windows named pipes, so launchers, overlays, automation tools and other external applications can read game state and control the client from any language.

The DLL runs inside `Wow.exe`. A client connects to `\\.\pipe\WowGameCommand`, sends one JSON-RPC request and receives one JSON response. A second pipe, `\\.\pipe\WowGameEvent`, streams events such as screen changes, loading progress and subscribed WoW UI events.

## What it provides

| Area | Methods and behavior |
| --- | --- |
| Lua | `lua.execute`, `lua.evaluate`, `lua.getGlobal`, `lua.setGlobal`, `lua.createNamespace` |
| Units | Health, power, position, rotation, race, class, level, GUID, target and state flags for `player`, `target`, `focus`, `mouseover`, `partyN`, `raidN` and GUIDs |
| Group and raid | Member list, combat summary and leader GUID, rebuilt only when the composition changes |
| Player and world | Cached player state, experience, AFK, DND, underwater state, map and zone information |
| Client control | Login, realm selection, character list, enter world, logout, quit, EULA and TOS acceptance |
| Screen state | Current glue screen, login state and low-level authentication diagnostics |
| Events | Subscribe to WoW UI events and stream them over the event pipe, with a circular history buffer |
| Window | Show, hide, minimize, maximize, focus, move, resize, opacity, top-most, title, icon and taskbar flash |
| Diagnostics | In-game console overlay, captured game logs and DLL information |

## Architecture

```mermaid
flowchart LR
    app["Your application<br/>Python, JavaScript, C#, Rust"]
    dll["WoWControlSDK.dll<br/>injected into Wow.exe"]
    wow["World of Warcraft 3.3.5a<br/>build 12340"]

    app -- "JSON-RPC 2.0 request<br/>named pipe WowGameCommand" --> dll
    dll -- "JSON response" --> app
    dll -. "events<br/>named pipe WowGameEvent" .-> app
    dll <--> wow
```

All game memory access is isolated in the `core/` layer. The `rpc/` layer parses and serializes JSON only; it never reads memory or uses raw addresses. See the [architecture documentation](docs/en/architecture.md) for the full layer model.

## Requirements

| Tool | Version | Used for |
| --- | --- | --- |
| Windows | 10 or 11, x64 | SDK and injector |
| Visual Studio | 2022 with the Desktop development with C++ workload | C++17 compiler |
| CMake | 3.20 or later | Build system |
| Git | 2.40 or later | Source control |
| Python | 3.10 or later | CLI test tool and documentation |
| pywin32 | Latest | CLI test tool |

Only **WoW 3.3.5a (build 12340)** is supported. A clean, unmodified client is recommended; lightly modified clients may also work.

## Build

```bash
git clone https://github.com/Zerik-Official/WoWControlSDK.git
cd WoWControlSDK

cmake -B build -A Win32
cmake --build build --config Release
```

The build writes two files:

| File | Purpose |
| --- | --- |
| `bin/release/WoWControlSDK.dll` | The SDK injected into the game |
| `bin/release/WowInjector.exe` | The included freeze-sync injector |

## Inject

```bash
WowInjector.exe --wow "C:\World of Warcraft\Wow.exe" --dll "bin\release\WoWControlSDK.dll"
```

The injector starts `Wow.exe` suspended, loads the DLL and waits up to 15 seconds for `\\.\pipe\WowGameCommand` before resuming the game. This guarantees that hooks and pipe servers are ready before the first frame. Any other DLL injector can be used, but the game will then initialize the SDK asynchronously.

## Send a request

The command pipe handles one request per connection: connect, write the JSON object, read the response and close the handle. The following Python example requires `pip install pywin32`:

```python
import json

import win32file


def rpc(method, params=None, request_id=1):
    pipe = win32file.CreateFile(
        r"\\.\pipe\WowGameCommand",
        win32file.GENERIC_READ | win32file.GENERIC_WRITE,
        0,
        None,
        win32file.OPEN_EXISTING,
        0,
        None,
    )
    try:
        request = {
            "jsonrpc": "2.0",
            "id": request_id,
            "method": method,
            "params": params or {},
        }
        win32file.WriteFile(pipe, json.dumps(request).encode("utf-8"))
        _, data = win32file.ReadFile(pipe, 65536)
        return json.loads(data.decode("utf-8").rstrip("\x00"))
    finally:
        win32file.CloseHandle(pipe)


print(rpc("unit.get", {"token": "player"}))
```

A request and its response look like this:

```json
{"jsonrpc": "2.0", "id": 1, "method": "unit.get", "params": {"token": "player"}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"exists": true, "health": 25000, "maxHealth": 25000, "name": "PlayerName", "position": {"x": 1234.5, "y": 2345.6, "z": 300.1}, "dead": false}}
```

## Common methods

| Method | Description |
| --- | --- |
| `lua.execute` / `lua.evaluate` | Run Lua with or without a typed return value |
| `unit.get` / `unit.state` / `unit.position` | Full unit snapshot, state flags or world position |
| `unit.mapPosition` | Normalized map position for party and raid members only |
| `group.get` / `group.combatSummary` | Group or raid roster and combat summary |
| `player.getState` / `world.getState` | Cached player and world state |
| `client.login` / `client.enterWorld` | Authenticate, select a realm and enter the world |
| `client.getScreen` / `client.getCharacters` | Screen detection and character list |
| `events.subscribe` / `events.getHistory` | Subscribe to WoW events and read buffered events |
| `dll.getInfo` / `dll.getLogs` | DLL diagnostics and captured game logs |
| `window.getInfo` / `window.setBounds` | Window inspection and layout control |

The full reference, with parameters, response fields and examples for every method, is in the [methods documentation](docs/en/methods/index.md).

## Documentation

The documentation is published at <https://zerik-official.github.io/WoWControlSDK/docs/> and built with MkDocs Material in English and Spanish. The source lives in [`docs/`](docs/README.md).

To serve it locally:

```bash
pip install -r requirements-docs.txt
mkdocs serve
```

The project is also indexed on [DeepWiki](https://deepwiki.com/Zerik-Official/WoWControlSDK), which updates every 7 days.

## Feedback

Found a bug or have a suggestion? Open an [issue](https://github.com/Zerik-Official/WoWControlSDK/issues) on GitHub or reach out on [Discord](https://discord.com/users/738137581295763576).

## License

[Apache License 2.0](LICENSE)
