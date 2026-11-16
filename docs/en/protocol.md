# RPC protocol

The SDK exposes every method through JSON-RPC 2.0 over two Windows named pipes. This page describes the transport, the request and response envelope, the error model and the client patterns that work with it.

## Transport

| Pipe | Direction | Mode | Purpose |
| --- | --- | --- | --- |
| `\\.\pipe\WowGameCommand` | Duplex | Message | One JSON-RPC request and one JSON response per connection |
| `\\.\pipe\WowGameEvent` | Duplex | Byte | Authentication handshake, then a newline-delimited event stream, one client at a time |

The command pipe reads one message of up to **4096 bytes**, dispatches it and writes the response. It then flushes the buffer and disconnects the client. This means:

- Open a new pipe handle for each request.
- Write the whole JSON object in a single `WriteFile`.
- Keep requests under 4096 bytes, including large Lua snippets.
- Read the response before closing the handle.

The event pipe is different. It is a long-lived connection: the client authenticates first, then the SDK writes one JSON object per line and the client keeps reading until the pipe closes.

## Authentication

Since SDK 2.0 both pipes require a session token, and they only accept local connections from the Windows user that runs the game (remote clients are rejected and other users have no access).

1. The launcher generates a random token of 32 to 128 hexadecimal characters for each game session.
2. It passes the token to `WowInjector.exe` through standard input with `--token-stdin`, so it never appears in a command line.
3. While the game is still frozen, the injector registers it with `auth.init`. The SDK accepts `auth.init` once, and only from the injector that started the game when it was launched that way.
4. Every command carries the token in a top-level `auth` field. A missing or wrong token returns `-32001 Unauthorized`.
5. An event client sends `{"auth":"<token>"}` followed by a newline as soon as it connects. The SDK answers with an `auth.ok` event and starts streaming, or with `auth.failed` and closes the connection. The handshake must arrive within 5 seconds.

`--no-auth` registers `{"disabled": true}` instead, for development tools. Without the injector, the first client that calls `auth.init` configures authentication.

| Method | Authentication | Description |
| --- | --- | --- |
| `auth.init` | Not required | Configure authentication once. Params: `{"token": "<hex>"}` or `{"disabled": true}`. Result: `{"ok": true, "state": "enabled"}` or `{"ok": false, "error": "<reason>"}`. |
| `auth.status` | Not required | Result: `{"state": "pending" \| "enabled" \| "disabled"}`. |

## Request

Every request uses the JSON-RPC 2.0 envelope:

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "unit.get",
  "params": {
    "token": "player"
  }
}
```

| Field | Type | Required | Description |
| --- | --- | --- | --- |
| `jsonrpc` | string | Yes | Must be exactly `"2.0"`. |
| `id` | integer | Recommended | Request identifier echoed in the response. Use a new number for each request. |
| `method` | string | Yes | Full method name, such as `client.ping` or `lua.evaluate`. |
| `params` | object | No | Method parameters. Defaults to an empty object. |
| `auth` | string | Yes, except `auth.*` | Session token, unless authentication is disabled. |

A numeric `id` is echoed as-is. String identifiers are accepted by the parser but the response uses `0`, so use integers.

## Response

A successful call returns the handler result inside `result`:

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "ok": true
  }
}
```

The shape of `result` depends on the method. It can be an object, an array, a string, a number, a boolean or `null`.

### Protocol errors

The dispatcher returns a JSON-RPC error object when the envelope itself is invalid or the method does not exist:

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "error": {
    "code": -32601,
    "message": "Method not found: unit.nope"
  }
}
```

| Code | Name | When it is returned |
| --- | --- | --- |
| `-32700` | Parse error | The message is not valid JSON. |
| `-32600` | Invalid request | The envelope is missing `jsonrpc` or `method`, or `jsonrpc` is not `"2.0"`. |
| `-32601` | Method not found | The method name is not registered. |
| `-32602` | Invalid params | Reserved by the protocol definition; handlers validate their own parameters. |
| `-32603` | Internal error | Reserved by the protocol definition. |
| `-32001` | Unauthorized | The `auth` token is missing or wrong, or authentication is not configured yet. |

### Method errors

Most domain failures are not protocol errors. The handler still returns a successful JSON-RPC response, and the error appears inside `result`:

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "error": "not on login screen"
  }
}
```

Common examples:

| Method | Condition | `result` |
| --- | --- | --- |
| `client.login` | Wrong screen or missing credentials | `{"error": "not on login screen"}` or `{"error": "username and password required"}` |
| `client.enterWorld` | Character name not found | `{"error": "character not found"}` |
| `player.*` | The player cache is empty | `{"error": "player not available"}` |
| `world.getState` | The world cache is empty | `{"error": "world not available"}` |
| `window.getInfo` | The window was not found | `{"error": "Window not found"}` |
| `lua.evaluate` | Lua raised an error | `{"ok": false, "error": "..."}` |

Always inspect `result` before reading its fields. A response without a top-level `error` only means that the request was valid, not that the operation succeeded.

## Method catalog

The SDK registers 94 methods across seventeen domains.

| Domain | Methods | Page |
| --- | --- | --- |
| `lua.*` | 5 | [lua.*](methods/lua.md) |
| `unit.*` | 4 | [unit.*](methods/unit.md) |
| `group.*` | 5 | [group.*](methods/group.md) |
| `player.*` | 4 | [player.*](methods/player.md) |
| `world.*` | 1 | [world.*](methods/world.md) |
| `friends.*` | 1 | [friends.*](methods/friends.md) |
| `summon.*` | 3 | [summon.*](methods/summon.md) |
| `lfg.*` | 4 | [lfg.*](methods/lfg.md) |
| `trade.*` | 6 | [trade.*](methods/trade.md) |
| `duel.*` | 3 | [duel.*](methods/duel.md) |
| `battlefield.*` | 3 | [battlefield.*](methods/battlefield.md) |
| `chat.*` | 3 | [chat.*](methods/chat.md) |
| `object.*` | 4 | [object.*](methods/object.md) |
| `client.*` | 17 | [client.*](methods/client.md) |
| `events.*` | 8 | [events.*](methods/events.md) |
| `dll.*` | 4 | [dll.*](methods/dll.md) |
| `window.*` | 19 | [window.*](methods/window.md) |

The [methods overview](methods/index.md) explains the conventions shared by every page and lists every method name.

## Client examples

### Python

The helper opens a new connection for each request, which matches the one-request-per-connection behavior of the command pipe:

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


print(rpc("client.getScreen"))
print(rpc("unit.get", {"token": "target"}))
```

### C#

`NamedPipeClientStream` is created and disposed inside the helper for the same reason:

```csharp
using System.IO.Pipes;
using System.Text;
using System.Text.Json;

static string Rpc(string method, object? parameters = null, int id = 1)
{
    using var pipe = new NamedPipeClientStream(".", "WowGameCommand", PipeDirection.InOut);
    pipe.Connect(5000);

    var request = JsonSerializer.Serialize(new
    {
        jsonrpc = "2.0",
        id,
        method,
        @params = parameters ?? new { }
    });

    var bytes = Encoding.UTF8.GetBytes(request);
    pipe.Write(bytes, 0, bytes.Length);
    pipe.Flush();

    using var reader = new StreamReader(pipe, Encoding.UTF8);
    return reader.ReadToEnd();
}

Console.WriteLine(Rpc("client.ping"));
```

### Event stream

The event pipe stays open and delivers one object per line:

```python
import json

import win32file

pipe = win32file.CreateFile(
    r"\\.\pipe\WowGameEvent",
    win32file.GENERIC_READ | win32file.GENERIC_WRITE,
    0,
    None,
    win32file.OPEN_EXISTING,
    0,
    None,
)

win32file.WriteFile(pipe, (json.dumps({"auth": TOKEN}) + "\n").encode("utf-8"))

buffer = ""
while True:
    _, data = win32file.ReadFile(pipe, 65536)
    buffer += data.decode("utf-8", errors="replace")
    while "\n" in buffer:
        line, buffer = buffer.split("\n", 1)
        if line.strip():
            print(json.loads(line))
```

See [events.*](methods/events.md) for the event names, payloads and the CLI listener.
