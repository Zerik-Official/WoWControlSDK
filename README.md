<h1 align="center">WoWControlSDK</h1>

<p align="center">
  <img src="https://img.shields.io/badge/C++17-24292F?style=for-the-badge&logo=cplusplus">
  <img src="https://img.shields.io/badge/CMake-24292F?style=for-the-badge&logo=cmake">
  <img src="https://img.shields.io/badge/Python-24292F?style=for-the-badge&logo=python">
  <img src="https://img.shields.io/badge/Windows-24292F?style=for-the-badge&logo=windows">
</p>

<p align="center">
  <img src="https://img.shields.io/badge/JSON--RPC_2.0-444444?style=for-the-badge">
  <img src="https://img.shields.io/badge/Named_Pipes-444444?style=for-the-badge">
  <img src="https://img.shields.io/badge/Detours-444444?style=for-the-badge">
  <img src="https://img.shields.io/badge/Lua_VM-444444?style=for-the-badge">
</p>

<p align="center">
  <img src=".github/images/preview.png" alt="WoWControlSDK Preview"/>
</p>

<p align="center">
  <a href="README.es.md">Ver en Español</a>
</p>

> [!CAUTION]
> This SDK is **not** focused on hacking, cheating, or botting. It is a tool for legitimate integration — launchers, stream overlays, Discord rich presence, UI automation, and similar use cases. Though clearly most methods benefit botting use cases. **Always play fair.** The author is not responsible for account suspensions or bans resulting from misuse of this software.

A lightweight DLL injector and SDK that exposes internal methods of **World of Warcraft 3.3.5a** through a clean **JSON-RPC 2.0** interface over Windows Named Pipes. Communicate with WoW from **any language** — Python, JavaScript, C#, Rust — by sending JSON commands to `\\.\pipe\WowGameCommand`.

This project was born from the need to extract real-time information from WoW (current screen, character data, map state, etc.) for an upcoming launcher — though maybe with way more features than I actually need just for a RichPresence. WoW's Lua API is very restrictive: no HTTP requests, no real-time file I/O, no way to communicate outside the game. The idea was inspired by [wow-discord-rpc](https://github.com/AipNooBest/wow-discord-rpc), a brilliant and clever project that encodes data into pixels and reads them from the screen to feed Discord's Rich Presence — it's what inspired me to create this SDK. WoWControlSDK takes a more direct approach: a lightweight injected DLL with a clean RPC interface.

This is still an early, rough version. As time goes on and I have more free time, I'll keep adding new features, events, commands, and improvements — especially to the architecture, which grew organically over time and feels a bit awkward right now.

---

## Documentation

Browse the docs structure at [`docs/`](./docs/README.en.md) for architecture, injection guide, and detailed RPC method references.

## Features

- **Lua Execution** — Run arbitrary Lua in WoW's VM (`lua.evaluate` with typed returns, `lua.execute` fire-and-forget)
- **Lua Globals** — Read/write global variables, create table namespaces
- **Unit Queries** — Get health, power, position, rotation, race, class, level, GUID, target, name, auras (ghost, dead, combat, casting, etc.) for `player`, `target`, `focus`, `mouseover`, `partyN`, `raidN`
- **Group/Raid** — Member list, combat summary, leader GUID, cached for performance
- **World State** — Is in world, is in combat, zone/map ID
- **Local Player** — XP, max XP, AFK, DND, underwater state, ghost detection
- **Thread-Safe** — Lua operations use raw `lua_pcall` with `errfunc=0` (safe from pipe thread)
- **Authentication** — Login to WoW with username/password, select realm, enter world, logout, quit
- **EULA/TOS Acceptance** — Automate acceptance of End User License Agreements and Terms of Service
- **Realm List** — Query and set custom realm lists
- **Screen Detection** — Detect current game screen (login, character select, loading, world, etc.)
- **Character Management** — List and refresh characters on the realm
- **Event System** — Subscribe to in-game events (screen changes, world state) received in real-time over the pipe

## Architecture

```
┌─────────────┐     JSON-RPC 2.0      ┌──────────────────┐
│  Your App   │ ◄──── named pipe ──── │  WoWControlSDK   │
│ (Python/JS/  │     \\.\pipe\         │  (injected DLL)   │
│  C#/etc.)   │   WowGameCommand      │                   │
└─────────────┘                       └────────┬─────────┘
                                               │
                                    ┌──────────┴──────────┐
                                    │  World of Warcraft   │
                                    │    3.3.5a client     │
                                    └─────────────────────┘
```

All game memory access is isolated in the `core/` layer. The `rpc/` layer handles JSON serialization only — no raw addresses, no memory reads.

## Dependencies

| Dependency | Version | Purpose |
|---|---|---|
| [Microsoft Detours](https://github.com/microsoft/Detours) | v4.0.1 | API hooking (DLL injection, function detours) |
| Windows SDK | 10.0+ | Named Pipes, process management |
| CMake | 3.20+ | Build system |
| Visual Studio | 2022 | C++17 compiler |

## Building

### Prerequisites
- Visual Studio 2022 with "Desktop development with C++" workload
- CMake 3.20+

### Steps

```bash
git clone https://github.com/Zerik-Official/WoWControlSDK.git
cd WoWControlSDK

cmake -B build
cmake --build build --config Release
```

Output:
- `bin/release/WoWControlSDK.dll` — The injected SDK
- `bin/release/WowInjector.exe` — The injector tool

## Usage

### Injector

```bash
WowInjector.exe --wow "C:\World of Warcraft\Wow.exe" --dll "bin\release\WoWControlSDK.dll"
```

> [!TIP]
> The included injector is recommended — it freezes the Wow.exe process during injection so the DLL is fully initialized before the game starts. However, you can use any injector you prefer, such as [Cheat Engine](https://www.cheatengine.org/).

**Parameters:**

| Argument | Description |
|---|---|
| `--wow <path>` | Path to `Wow.exe` |
| `--dll <path>` | Path to `WoWControlSDK.dll` |
| `--pipe <name>` | Named pipe to wait for before unfreezing (default: `WowGameCommand`) |
| `--no-pipe` | Unfreeze after `--wait` ms instead of waiting for a pipe |
| `--wait <ms>` | Milliseconds to wait when `--no-pipe` is active (default: 500) |
| `--kill` | Kill existing WoW process before starting |
| `--wow-args <str>` | Extra arguments for WoW.exe |
| `--pipe-cmd <json>` | Send a JSON-RPC command after pipe is ready (repeatable) |

### CLI Test Tool

An interactive Python CLI is available at `tests/wowsdk_cli.py` with menus for Lua execution, unit queries, group inspection, and more. Use it as a reference or starting point for your own integration.

**Dependencies:**

```bash
pip install pywin32
```

**Usage:**

```bash
python tests/wowsdk_cli.py
```

### RPC Protocol

Send JSON-RPC 2.0 requests to the named pipe `\\.\pipe\WowGameCommand`:

```json
{"jsonrpc": "2.0", "id": 1, "method": "unit.get", "params": {"token": "player"}}
```

**Response:**
```json
{"id": 1, "jsonrpc": "2.0", "result": {"exists": true, "health": 25000, "maxHealth": 25000, "name": "PlayerName", "position": {"x": 1234.5, "y": 2345.6, "z": 300.1}, "rotation": 5.23, "dead": false, "ghost": false, ...}}
```

**From any language:**

<details>
<summary>Python</summary>

```python
import win32pipe, win32file, json

pipe = win32file.CreateFile(
    r"\\.\pipe\WowGameCommand",
    win32file.GENERIC_READ | win32file.GENERIC_WRITE,
    0, None, win32file.OPEN_EXISTING, 0, None
)

def rpc(method, params={}):
    req = json.dumps({"jsonrpc": "2.0", "id": 1, "method": method, "params": params})
    win32file.WriteFile(pipe, (req + "\n").encode())
    _, data = win32file.ReadFile(pipe, 65536)
    return json.loads(data.decode())

print(rpc("unit.get", {"token": "player"}))
```
</details>

<details>
<summary>C#</summary>

```csharp
using System.IO.Pipes;

var pipe = new NamedPipeClientStream(".", "WowGameCommand", PipeDirection.InOut);
pipe.Connect();
var writer = new StreamWriter(pipe) { AutoFlush = true };
var reader = new StreamReader(pipe);

writer.WriteLine("{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"lua.evaluate\",\"params\":{\"code\":\"return UnitName(\\\"player\\\")\"}}");
var response = reader.ReadLine();
Console.WriteLine(response);
```
</details>

### Key RPC Methods

> There are many more methods available. These are the most commonly used ones.

| Method | Description |
|---|---|
| `lua.execute` | Execute Lua (fire-and-forget) |
| `lua.evaluate` | Execute Lua with typed return value |
| `lua.getGlobal` / `lua.setGlobal` / `lua.createNamespace` | Lua global operations |
| `unit.get` / `unit.state` / `unit.position` | Unit data queries |
| `unit.mapPosition` | Unit normalized map position **(party/raid only)** |
| `group.get` / `group.summary` | Group/raid info |
| `client.login` / `client.enterWorld` / `client.logout` / `client.quit` | Authentication and session control |
| `client.getScreen` / `client.getDebugState` | Screen and debug state detection |
| `client.getRealmlist` / `client.setRealmlist` | Realm list management |
| `client.getCharacters` / `client.refreshCharacters` | Character list |
| `client.acceptAll` / `client.acceptEULA` / `client.acceptTOS` | EULA/TOS automation |
| `client.ping` | Connection health check |
| `world.getState` | World/zone state information |
| `object.exists` / `object.getType` / `object.isUnit` / `object.isPlayer` | Object inspection |
| `events.subscribe` / `events.unsubscribe` / `events.list` | Event system — supports native Lua events (PLAYER_LOGIN, PLAYER_ENTERING_WORLD, etc.) |
| `dll.toggleConsole` / `dll.getInfo` / `dll.getLogs` / `dll.setLogConfig` | DLL management |
| `window.restore` / `window.maximize` / `window.focus` / `window.getPosition` | WoW window management |

> [!NOTE]
> There are many more methods available. Check the full documentation at [`docs/`](./docs/README.en.md).

---

> [!WARNING]
> Only **WoW 3.3.5a (build 12340)** is supported. A clean, unmodified WoW client is recommended, though lightly modded clients may also work.

## Feedback

Found a bug? Have a suggestion? Open an [issue](https://github.com/Zerik-Official/WoWControlSDK/issues) on GitHub, or reach out on [Discord](https://discord.com/users/738137581295763576).

The project is also indexed on [DeepWiki](https://deepwiki.com/Zerik-Official/WoWControlSDK) — you can ask it questions about the codebase for quick answers. It updates every 7 days.

## License

[Apache License 2.0](LICENSE)