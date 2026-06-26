# Architecture

## Layered Design

WoWControlSDK follows a strict layered architecture. Each layer has defined responsibilities and **must not** cross boundaries into other layers' domains.

```
┌─────────────────────────────────────────────────────┐
│                     injector/                        │
│          (standalone EXE, process launcher)          │
└──────────────────────┬──────────────────────────────┘
                       │ DLL injection
┌──────────────────────▼──────────────────────────────┐
│                       ipc/                           │
│           (named pipe transport, no game access)      │
└──────────────────────┬──────────────────────────────┘
                       │ JSON-RPC 2.0
┌──────────────────────▼──────────────────────────────┐
│                       rpc/                           │
│           (protocol, dispatcher, handlers)            │
│          ❌ No game memory access — calls core/       │
└──────────────────────┬──────────────────────────────┘
                       │ core/ API calls
┌──────────────────────▼──────────────────────────────┐
│                    runtime/                           │
│      (console, logging, event bus, state cache)       │
│          ❌ No game memory access — calls core/       │
└──────────────────────┬──────────────────────────────┘
                       │ core/ API calls
┌──────────────────────▼──────────────────────────────┐
│                     hooks/                            │
│    (detour hooks on game functions, event capture)    │
│          ❌ No hardcoded addresses — uses offsets/    │
└──────────────────────┬──────────────────────────────┘
                       │ calls core/ + hooks data
┌──────────────────────▼──────────────────────────────┐
│                      core/                            │
│   ✅ ONLY layer allowed to read/write game memory    │
│   (native wrappers, API layer, offsets, resolvers)    │
└──────────────────────┬──────────────────────────────┘
                       │ reads/writes
┌──────────────────────▼──────────────────────────────┐
│              World of Warcraft 3.3.5a                 │
│              (game process memory, Lua VM)             │
└─────────────────────────────────────────────────────┘
```

### Layer Responsibilities

| Layer | Role | Game Memory Access |
|---|---|---|
| `injector/` | Standalone EXE, launches WoW and injects DLL | None |
| `ipc/` | Named pipe transport (read/write JSON) | None |
| `rpc/` | JSON-RPC 2.0 protocol, handlers, serialization | None |
| `runtime/` | State cache, event bus, console, logging, realm list | None (calls `core/`) |
| `hooks/` | Detour hooks on game functions, event signal capture | Via offsets only |
| `core/` | Native wrappers, API layer, memory reads/writes | Yes |
| `client/` | Thin wrappers around `core/` and game Lua | Minimal addresses |
| `offsets/` | All hardcoded addresses as `constexpr uintptr_t` | Constants only |

## Data Flow

### RPC Request (read operation)

```
RPC pipe → ipc/ → rpc/dispatcher → rpc/handler → core/API → game memory
                                              ↘ runtime/cache
```

1. JSON-RPC request arrives on the named pipe
2. `ipc/` reads and parses the raw JSON
3. `rpc/` dispatcher routes to the matching handler
4. Handler calls `core/` API functions (or reads from `runtime/` cache)
5. `core/` reads game memory through native wrappers
6. Response flows back: `handler → dispatcher → ipc/ → pipe`

### Event Push (write / notification)

```
game signal → hooks/ → EventBus → EventPipe → event pipe
                              ↘ StateCache (polled per frame)
```

1. Game fires a UI event (e.g., `PLAYER_LOGIN`)
2. `hooks/EventHooks` captures it via detour
3. Pushes to the event named pipe via `EventPipe`
4. `StateCache` polls player/world/screen state each frame
5. `client.screenChanged` is pushed to `EventPipe` (the only diff event that reaches external subscribers)

## Threading Model

| Thread | Role |
|---|---|
| **Game thread** (Main/Frame) | Runs WoW's frame loop. Hook callbacks execute here. `StateCache::update()` called each frame. Game memory reads happen on this thread. |
| **RPC pipe thread** | Reads incoming JSON-RPC requests, dispatches to handlers. Blocks on `ReadFile`. |
| **Event pipe thread** | Pushes events to the secondary event pipe. Blocks on `WriteFile`. |
| **RPC handler execution** | Some handlers post work to the game thread via `Hooks::Frame::Post` (async), others block the RPC thread with polling. |

> [!NOTE]
> Lua operations (`lua.evaluate`, `lua.execute`) use `lua_pcall` with `errfunc=0` and are safe to call from the RPC thread. State-changing operations (login, logout, enter world) are posted to the game thread to avoid races.

## Offsets Isolation

All hardcoded game addresses, function pointers, and struct offsets are defined as `constexpr uintptr_t` in `offsets/Offsets*.h`. No raw `0x...` literals appear outside this folder.

```
offsets/
├── OffsetsClient.h      Client state, net client, in-world flag
├── OffsetsConsole.h     CVar functions
├── OffsetsFrame.h       Render frame, D3D
├── OffsetsGroup.h       Group/raid structures
├── OffsetsLoading.h     Loading flag and progress
├── OffsetsLogin.h       Authentication, logout, character list
├── OffsetsLua.h         Lua state, signal event handler
├── OffsetsMap.h         Map position conversion
├── OffsetsObjectMgr.h   Object manager
├── OffsetsPlayer.h      Player-specific offsets
└── OffsetsUnit.h        Unit structure, movement, auras
```

Source files reference these as `Offsets::Namespace::CONSTANT`, never as raw hex literals.

## Event System

The event system has two layers:

1. **EventBus** (`runtime/events/EventBus.h`) — in-process pub/sub. Used by `StateCache` to emit events (`player.healthChanged`, `world.zoneChanged`, etc.). Currently no internal component subscribes to these events. Available for future in-process use.

2. **EventPipe** (`runtime/events/EventPipe.h`) — secondary named pipe at `\\.\pipe\WowGameEvent`. Pushes events as JSON for external consumers. Written to by `StateCache` (only `client.screenChanged`) and `LoadingHooks` (loading events).

Events pushed to the EventPipe include:
- SDK custom events: `client.screenChanged`, `loading.startup`, `loading.progress`, `loading.finished`
- WoW UI events captured by `EventHooks` (if subscribed via `events.subscribe`)
