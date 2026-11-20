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

Since SDK 2.0 both pipes require a session token. Generate a new random token of 32 to 128 hexadecimal characters for each game session and keep it somewhere safe: your client will need the same value. For example, `python -c "import secrets; print(secrets.token_hex(32))"` prints a valid 64-character token. Store it in an environment variable so both the injector and your client use it:

```cmd
set "WOW_SDK_TOKEN=paste_your_hex_token_here"
```

Start the game through the included injector, passing the token through standard input:

```cmd
echo %WOW_SDK_TOKEN% | WowInjector.exe --wow "C:\World of Warcraft\Wow.exe" --dll "bin\release\WoWControlSDK.dll" --token-stdin
```

The injector starts `Wow.exe` suspended, loads the DLL, waits up to 15 seconds for `\\.\pipe\WowGameCommand`, registers the token with `auth.init` while the game is still frozen and then resumes it. The SDK starts in a pending state and rejects every command until the token is registered, so no other process can claim the session first. The SDK is fully initialized before the first game frame runs.

!!! important
    The injector needs exactly one of `--token-stdin` or `--no-auth`. Use `--no-auth` only for local development tools: it lets any process running as your Windows user control the game. See [Authentication](protocol.md#authentication) for the full flow.

!!! tip
    Run the injector from the repository root or use absolute paths for `--wow` and `--dll`. The injector resolves relative paths from the current working directory.

The injector accepts additional arguments, including `--kill`, `--wow-args` and repeatable `--pipe-cmd` commands. See [Injection](injection.md) for every option and for the alternative injection flow.

## Send a first request

The command pipe handles one request per connection. Every request carries the session token you registered at injection in a top-level `auth` field. Install `pywin32` and run a small client:

```bash
pip install pywin32
```

```python
import json
import os

import win32file

TOKEN = os.environ["WOW_SDK_TOKEN"]


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
            "auth": TOKEN,
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

A missing or wrong token returns the JSON-RPC error `-32001 Unauthorized`. If you started the game with `--no-auth`, remove the `auth` field. To receive events, connect to `\\.\pipe\WowGameEvent` and send `{"auth":"<token>"}` followed by a newline first; see [Authentication](protocol.md#authentication).

!!! note
    Lua and unit methods require the appropriate game screen. `lua.*`, `unit.*`, `group.*`, `player.*` and `world.*` generally need the player to be in the world. `client.login` requires the login screen, and `client.enterWorld` requires the character select screen.

## Use the desktop client

The repository includes a desktop client for testing every domain. Download `WoWControlClient-<version>.zip` from a release, or run it from source:

```bash
pip install -r requirements.txt
cd tools
python -m wowsdk_client
```

Set the path of `Wow.exe` in **Settings** and press **Launch WoW**: the client starts the game with a session token and uses it for every request. See [Desktop client](operations/cli.md) for the window, the settings and the live event view.

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
| `-32001 Unauthorized` | The `auth` field is missing, has the wrong token, or authentication is still pending. Reuse the token passed to the injector, or start the game with `--no-auth` for local development. |
| `auth.init rejected` / the game closes during launch | The token is not 32-128 hexadecimal characters, or another client already configured authentication. The injector terminates the game it started on any authentication failure. |
| `Method not found` | The method name is misspelled or belongs to a newer SDK version. Check the [methods reference](methods/index.md). |
| `not on login screen` | `client.login` was called after the client left the login screen. Return to the login screen and retry. |
| `not on character select screen` | `client.enterWorld` requires the character list. Call `client.login` with `realmName` or wait for the character select screen. |
| `player not available` or `world not available` | The per-frame cache is empty. The player has not entered the world yet. |
| `unit.mapPosition` returns zeros | Map positions are only resolved for party and raid members. Use `unit.position` for other units. |
| The pipe disconnects after every call | This is expected. The command pipe serves one request per connection; open a new connection for each request. |

## Supported client

Only **World of Warcraft 3.3.5a (build 12340)** is supported. The offsets and structures are specific to that build. A clean, unmodified client is recommended, although lightly modified clients may also work.
