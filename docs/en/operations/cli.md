# CLI tool

The repository includes `tests/wowsdk_cli.py`, an interactive Python client that exercises every RPC domain. It is useful for manual testing, for inspecting responses and as a reference for a custom integration.

## Requirements

| Tool | Version | Purpose |
| --- | --- | --- |
| Python | 3.10 or later | Runs the CLI |
| pywin32 | Latest | Named pipe access on Windows |

```bash
pip install pywin32
```

## Configuration

The top of `tests/wowsdk_cli.py` defines the paths used by its launch and injection actions:

```python
RPC_PIPE   = r"\\.\pipe\WowGameCommand"
EVENT_PIPE = r"\\.\pipe\WowGameEvent"

WOW_PATH  = r"D:\Juegos\World Of Warcraft\Wow.exe"
DLL_PATH  = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "bin", "release", "WoWControlSDK.dll"))
INJECTOR  = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "bin", "release", "WowInjector.exe"))
```

Adjust `WOW_PATH` before using the launch options. `DLL_PATH` and `INJECTOR` already resolve to the Release output.

## Run

```bash
python tests/wowsdk_cli.py
```

The CLI checks whether the command pipe is reachable and prints the menu. Each action sends one request, prints the raw JSON response and returns to the menu.

## Menu

| Section | Actions |
| --- | --- |
| Auth | Login, accept EULA, TOS, termination, scanning and contest agreements, accept all, get and refresh characters, enter world, logout, quit, get and set realmlist |
| Player | Player state, health, position and target |
| Unit | `unit.get`, `unit.state`, `unit.position` |
| Object | `object.*` inspection |
| World and group | `world.getState`, `group.get`, `group.combatSummary` |
| Events | List, subscribe, subscribe defaults, unsubscribe or clear, view history, clear history, set buffer, check hook readiness and listen live |
| Window | Every `window.*` method, including `setIcon` and `restoreIcon` |
| Lua | `lua.execute`, `lua.evaluate`, `lua.getGlobal`, `lua.setGlobal` and `lua.createNamespace` |

## Client behavior

`send()` opens a new command pipe connection for every request, writes the JSON object, reads the response and closes the handle. This matches the one-request-per-connection behavior of `\\.\pipe\WowGameCommand`.

```python
def send(method, params={}):
    req = {"jsonrpc": "2.0", "method": method, "params": params, "id": next_id()}
    h = win32file.CreateFile(RPC_PIPE, win32file.GENERIC_READ | win32file.GENERIC_WRITE, 0, None, win32file.OPEN_EXISTING, 0, None)
    win32file.WriteFile(h, json.dumps(req).encode("utf-8"))
    _, data = win32file.ReadFile(h, 65536)
    win32file.CloseHandle(h)
    return json.loads(data.decode("utf-8").rstrip("\x00"))
```

If the pipe is unavailable, `send()` returns `{"error": "<Windows error>"}` instead of raising.

## Live event listener

**Events → Listen live** connects to `\\.\pipe\WowGameEvent` and prints each event as it arrives. The listener reads a byte stream, splits it on newlines and parses each line as JSON.

The event pipe accepts one client at a time. Close other listeners before starting this option.

!!! tip
    Subscribe to the events you need before opening the listener. SDK events such as `client.screenChanged` and the loading events are always streamed, but WoW UI events only appear after `events.subscribe`.
