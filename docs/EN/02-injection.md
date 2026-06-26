# Injection

## Overview

WoWControlSDK ships with `WowInjector.exe`, a standalone injector that launches Wow.exe, injects the DLL, and synchronizes startup so the SDK is fully initialized before the game begins.

## Recommended Injection Flow

The included injector uses a freeze-sync approach:

1. **Launch suspended** — Wow.exe is started with `CREATE_SUSPENDED`, freezing all threads before any game code runs
2. **DLL injection** — `CreateRemoteThread` + `LoadLibrary` loads `WoWControlSDK.dll` into the frozen process
3. **DLL init** — The main thread is resumed momentarily, letting `DllMain` run (console, hooks, pipe threads initialize)
4. **Pipe wait** — The game stays frozen. The SDK's RPC pipe server (`\\.\pipe\WowGameCommand`) starts listening
5. **Unfreeze** — Once the pipe is accepting connections (or after `--wait` ms with `--no-pipe`), the game resumes normally

This ensures the SDK is ready before any game frame executes — no race conditions, no missed events.

```
WowInjector.exe
    │
    ├── CreateProcess(Wow.exe, CREATE_SUSPENDED)
    ├── VirtualAllocEx + WriteProcessMemory + CreateRemoteThread(LoadLibrary)
    ├── ResumeThread (DllMain runs briefly)
    ├── Wait for named pipe
    └── ResumeThread (game starts)
```

## Usage

```bash
WowInjector.exe --wow "C:\World of Warcraft\Wow.exe" --dll "bin\release\WoWControlSDK.dll"
```

### Arguments

| Argument | Description |
|---|---|
| `--wow <path>` | Path to `Wow.exe` |
| `--dll <path>` | Path to `WoWControlSDK.dll` |
| `--pipe <name>` | Named pipe to wait for before unfreezing (default: `WowGameCommand`) |
| `--no-pipe` | Unfreeze after `--wait` ms instead of waiting for the pipe |
| `--wait <ms>` | Milliseconds to wait when `--no-pipe` is active (default: 500) |
| `--kill` | Kill existing WoW process before launching |
| `--wow-args <str>` | Extra arguments passed to Wow.exe |
| `--pipe-cmd <json>` | Send a JSON-RPC command after the pipe is ready (repeatable) |

### Example with options

```bash
WowInjector.exe ^
    --wow "C:\World of Warcraft\Wow.exe" ^
    --dll "C:\WoWControlSDK\bin\release\WoWControlSDK.dll" ^
    --pipe-cmd "{\"jsonrpc\":\"2.0\",\"method\":\"dll.toggleConsole\",\"id\":1}"
```

> [!TIP]
> The freeze-sync approach is the recommended method — it guarantees the DLL is fully initialized before any game frame runs. This is the safest way to ensure reliable operation.

## Alternative Injection

You are not required to use the included injector. Any DLL injector can be used, such as [Cheat Engine](https://www.cheatengine.org/).

When using a third-party injector:
- The game must already be running
- The DLL will initialize asynchronously (hooks attach during the next frame)
- The pipe will become available shortly after injection
- Some events occurring between game start and injection may be missed

## RPC Pipe Commands at Injection

With `--pipe-cmd`, you can queue JSON-RPC commands to execute immediately after the SDK initializes. This is useful for automation:

```bash
WowInjector.exe --wow "..." --dll "..." ^
    --pipe-cmd "{\"jsonrpc\":\"2.0\",\"method\":\"dll.setLogConfig\",\"params\":{\"showInConsole\":true},\"id\":1}" ^
    --pipe-cmd "{\"jsonrpc\":\"2.0\",\"method\":\"dll.toggleConsole\",\"id\":2}"
```

Commands are sent in order. Each waits for a response before sending the next. Safe commands at startup include `dll.toggleConsole`, `dll.setLogConfig`, and `dll.getInfo`. Avoid game-dependent commands (`lua.*`, `client.login`, `unit.*`) until the player has reached the appropriate screen.

## Event Pipe

In addition to the main RPC pipe (`\\.\pipe\WowGameCommand`), the SDK opens a secondary pipe at `\\.\pipe\WowGameEvent` for streaming events in real-time. This pipe is write-only from the SDK side — connect to it as a client to receive event notifications as they happen.

See [events.* Methods](03-methods/events.md) for available events and the Python CLI listener example.
