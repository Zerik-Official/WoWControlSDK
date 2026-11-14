# Getting started

This guide installs the tools, builds the SDK, injects it into the game and sends a first RPC request.

## Prerequisites

| Tool | Version | Used for |
| --- | --- | --- |
| Windows | 10 or 11, x64 | SDK, injector and CLI |
| Visual Studio | 2022 with the Desktop development with C++ workload | C++17 compiler |
| CMake | 3.20 or later | Build system |
| Git | 2.40 or later | Source control |
| Python | 3.10 or later | CLI test tool and documentation |
| pywin32 | Latest | CLI test tool |

## Build the SDK

1. Clone the repository and enter it:

   ```bash
   git clone https://github.com/Zerik-Official/WoWControlSDK.git
   cd WoWControlSDK
   ```

2. Configure the project with CMake:

   ```bash
   cmake -B build -A Win32
   ```

3. Build the Release configuration:

   ```bash
   cmake --build build --config Release
   ```

The build writes two artifacts:

| File | Purpose |
| --- | --- |
| `bin/release/WoWControlSDK.dll` | The SDK injected into the game |
| `bin/release/WowInjector.exe` | The included freeze-sync injector |

A Debug build writes to `bin/debug/` instead. See [Building](operations/building.md) for the full target list and build options.

## Inject the DLL

Start the game through the included injector:

```bash
WowInjector.exe --wow "C:\World of Warcraft\Wow.exe" --dll "bin\release\WoWControlSDK.dll"
```

The injector starts `Wow.exe` suspended, loads the DLL, waits up to 15 seconds for `\\.\pipe\WowGameCommand` and then resumes the game. The SDK is fully initialized before the first game frame runs.

!!! tip
    Run the injector from the repository root or use absolute paths for `--wow` and `--dll`. The injector resolves relative paths from the current working directory.

The injector accepts additional arguments, including `--kill`, `--wow-args` and repeatable `--pipe-cmd` commands. See [Injection](injection.md) for every option and for the alternative injection flow.

## Send a first request

The command pipe handles one request per connection. Install `pywin32` and run a small client:

```bash
pip install pywin32
```

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


print(rpc("client.ping"))
print(rpc("client.getScreen"))
```

`client.ping` returns `{"status": "ok"}` when the DLL is loaded and the pipe is listening. `client.getScreen` reports whether the client is on the login screen, the character select screen or in the world.

!!! note
    Lua and unit methods require the appropriate game screen. `lua.*`, `unit.*`, `group.*`, `player.*` and `world.*` generally need the player to be in the world. `client.login` requires the login screen, and `client.enterWorld` requires the character select screen.

## Use the CLI tool

The repository includes an interactive Python CLI for testing every domain:

```bash
python tests/wowsdk_cli.py
```

The tool expects `pywin32` and reads `WOW_PATH`, `DLL_PATH` and `INJECTOR` from the top of `tests/wowsdk_cli.py`. Adjust those paths before using its launch and injection options. See [CLI tool](operations/cli.md) for the menu layout and the live event listener.

## Serve the documentation

Install the documentation dependencies and start the local site:

```bash
pip install -r requirements-docs.txt
mkdocs serve
```

The English site is served at `http://127.0.0.1:8000/` and the Spanish site at `http://127.0.0.1:8000/es/`. The `docs` workflow validates both languages with `mkdocs build --strict` and publishes the site to GitHub Pages from `main`.

## Troubleshooting

| Symptom | Cause and fix |
| --- | --- |
| `The system cannot find the file specified` when opening the pipe | The DLL is not loaded or the injector has not finished. Start the game through the injector and wait for the pipe. |
| `Method not found` | The method name is misspelled or belongs to a newer SDK version. Check the [methods reference](methods/index.md). |
| `not on login screen` | `client.login` was called after the client left the login screen. Return to the login screen and retry. |
| `not on character select screen` | `client.enterWorld` requires the character list. Call `client.login` with `realmName` or wait for the character select screen. |
| `player not available` or `world not available` | The per-frame cache is empty. The player has not entered the world yet. |
| `unit.mapPosition` returns zeros | Map positions are only resolved for party and raid members. Use `unit.position` for other units. |
| The pipe disconnects after every call | This is expected. The command pipe serves one request per connection; open a new connection for each request. |

## Supported client

Only **World of Warcraft 3.3.5a (build 12340)** is supported. The offsets and structures are specific to that build. A clean, unmodified client is recommended, although lightly modified clients may also work.
