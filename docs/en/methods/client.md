# client.* methods

Control the game session: login, realm selection, character management, screen detection, diagnostics and legal agreement automation.

Most state-changing calls are posted to the game thread. Calls that need a result, such as `client.login` and `client.enterWorld`, block the RPC thread while polling.

## client.ping

Health check. It succeeds whenever the DLL is loaded and the command pipe is listening.

### Parameters

None.

### Response

```json
{"status": "ok"}
```

## client.getScreen

Returns the current game screen and login state.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `screen` | string | `"login"`, `"charselect"`, `"world"` or the raw glue screen name |
| `inWorld` | boolean | Whether the player is in the game world |
| `loginState` | integer | Raw WoW login state value |

## client.getDebugState

Returns low-level authentication and connection state for diagnostics.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `netClientPtr` | integer | Memory address of the `NetClient` object |
| `authResultCode` | integer | Raw WoW authentication result code |
| `errorFlag` | integer | Net error flag |
| `authStatus` | integer | Raw authentication status |
| `loginState` | integer | Raw login state |
| `screen` | string | Current glue screen name |
| `inWorld` | boolean | Whether the player is in the game world |
| `authResultStr` | string | Captured login result string |

!!! warning
    The fields are raw memory values that depend on the client build. Use this method for diagnosis, not for application logic.

## client.getNetStats

Returns the latency and bandwidth that the game measures for its server connection, the same values as `GetNetStats()` in Lua. The SDK reads them from the game's network client every 500 milliseconds and answers from that cache, so the call is cheap.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `connected` | boolean | `true` in the world; outside the world the other values are `0` |
| `latencyMs` | integer | Average round trip time to the server in milliseconds |
| `bandwidthIn` | number | Incoming bandwidth in KB/s |
| `bandwidthOut` | number | Outgoing bandwidth in KB/s |
| `ageMs` | integer | Milliseconds since the values were read |

```json
{"connected": true, "latencyMs": 48, "bandwidthIn": 2.418, "bandwidthOut": 0.351, "ageMs": 212}
```

!!! note
    The game averages the latency of its last messages, so the value changes slowly. Polling more often than once per second gives no extra information.

## client.getFramerate

Returns the frames per second, averaged by the game over its last 30 frames, the same value as `GetFramerate()` in Lua. It uses the same 500 millisecond cache as `client.getNetStats`.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `fps` | number | Frames per second, with one decimal |
| `ageMs` | integer | Milliseconds since the value was read |

```json
{"fps": 59.8, "ageMs": 140}
```

## client.getAddonMemory

Returns the memory used by each loaded addon and by the whole Lua VM, the same values as `GetAddOnMemoryUsage()` and `collectgarbage("count")`. The call runs on the game thread.

Measuring the memory walks every Lua object, which can take a few milliseconds with many addons. The SDK measures at most once per second; a call made sooner answers with the last measurement, and `ageMs` tells how old it is.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `addons` | array | Addons that use memory, largest first, as `{"name", "kilobytes"}` |
| `scripts` | array | Scripts loaded with [`lua.loadScript`](lua.md#lualoadscript), as `{"name", "kilobytes"}` |
| `addonsKilobytes` | number | Sum of the addons |
| `totalKilobytes` | number | Memory of the whole Lua VM, including Blizzard's interface |
| `ageMs` | integer | Milliseconds since the measurement |

```json
{
  "addons": [
    {"name": "Questie", "kilobytes": 18452.6},
    {"name": "DBM-Core", "kilobytes": 6210.3}
  ],
  "scripts": [{"name": "@MyTool", "kilobytes": 12.4}],
  "addonsKilobytes": 24662.9,
  "totalKilobytes": 31877.2,
  "ageMs": 0
}
```

Addons that are not loaded do not appear.

## client.getLastCrash

Returns the last fatal error of the game, such as `ERROR #132 (0x85100084) Fatal Exception`, with the log the game wrote to its `Errors` folder.

The SDK registers a handler in the game's own error reporting. It runs after the game writes the log and before it shows the error window, so the data is ready while the window is open. The same data is sent to the event pipe as [`client.crashed`](events.md#sdk-event-payloads).

### Parameters

None.

### Response

Without errors in this session:

```json
{"crashed": false}
```

After an error:

| Field | Type | Description |
| --- | --- | --- |
| `crashed` | boolean | `true` |
| `kind` | string | `"crash"` for an exception, `"error"` for other fatal errors |
| `errorNumber` | integer | The number the game shows, for example `132` |
| `code` | string | Full error code, for example `"0x85100084"` |
| `title` | string | Error title, for example `"Fatal Exception"` |
| `description` | string | Short description, such as the exception and its address |
| `details` | string | Longer explanation written by the game |
| `exception` | object or `null` | `{"code", "address", "module", "offset"}` of the exception; `null` for other errors |
| `logPath` | string or `null` | Path of the log in the `Errors` folder. `null` when the game wrote no log. A crash that damages the process memory can also leave the path unreadable |
| `log` | string or `null` | Contents of the log, up to 64 KB. `null` when the log cannot be read, for example when `logPath` is `null` or damaged |
| `logTruncated` | boolean | Present and `true` when the log was longer than 64 KB |
| `time` | integer | Milliseconds since Windows started |

```json
{
  "crashed": true,
  "kind": "crash",
  "errorNumber": 132,
  "code": "0x85100084",
  "title": "Fatal Exception",
  "description": "0x00000004 (ACCESS_VIOLATION) at 0023:00000004",
  "details": "The instruction at \"0x00000004\" referenced memory at \"0x00000004\".",
  "exception": {"code": "0xC0000005", "address": "0x00000004"},
  "logPath": "C:\\Games\\World of Warcraft\\Errors\\2026-11-16 17-05-12.txt",
  "log": "==============================================================================\r\nWorld of WarCraft: Retail Build ...",
  "time": 5183920
}
```

!!! note
    A crash can leave the game unable to answer. A launcher should also read the newest file in the `Errors` folder after the game process ends, in case neither the event nor this method could be delivered.

## client.login

Performs a complete login sequence. It optionally sets the realm list and realm name CVars, sends the credentials and waits for the login result.

The login screen ignores credentials until the client finishes its startup checks and accepts the user agreements, so a login sent as soon as the screen appears would be lost. The method waits up to 30 seconds for the client to accept a login before sending the credentials, so it can be called as soon as the login screen is reported.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `username` | string | Yes | Account username |
| `password` | string | Yes | Account password |
| `realmList` | string | No | If provided, sets the `realmList` CVar before login |
| `realmName` | string | No | If provided, selects the realm and waits for the character select screen |

### Response

On success:

```json
{"ok": true}
```

On failure:

```json
{"error": "incorrect password"}
```

When the login succeeds but a later wait fails, the response also contains `ok: false`:

```json
{"ok": false, "error": "realm list timeout"}
```

### Error strings

| Error | Meaning |
| --- | --- |
| `not on login screen` | The client is not on the login screen |
| `login screen not ready` | The client did not accept a login within 30 seconds |
| `username and password required` | One of the credentials is empty |
| `login failed` | Generic authentication failure |
| `server down` | The login server is offline |
| `unknown account` | The account does not exist |
| `incorrect password` | Wrong password |
| `disconnected` | The connection was closed |
| `parental control` | Parental controls block the account |
| `chargeback` | The account has a chargeback |
| `conversion required` | The account must be converted |
| `account banned` | The account is banned |
| `account suspended` | The account is suspended |
| `account locked` | The account is locked |
| `game account locked` | The game account is locked |
| `unlockable lock` | The account has an unlockable lock |
| `already online` | The account is already logged in |
| `bad version` | The client version is not accepted |
| `no time remaining` | The subscription expired |
| `database busy` | The login database is busy |
| `trial expired` | The trial period expired |
| `account converted` | The account was converted |
| `login timeout` | No login result arrived within 20 seconds |
| `realm list timeout` | The realm list was not ready within 5 seconds |
| `realm select timeout` | The character select screen did not appear within 15 seconds |
| `unknown error` | An unmapped login result |

### Example

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "client.login", "params": {"username": "myuser", "password": "mypass", "realmName": "MyServer"}}
```

!!! note
    The method first waits up to 30 seconds for the login screen to accept a login, then up to 20 seconds for the login result. When `realmName` is provided, the method also waits up to 5 seconds for the realm list and up to 15 seconds for the character select screen.

!!! warning
    This method blocks the RPC thread while polling with `Sleep(50)`. Do not call it from time-sensitive code.

## client.enterWorld

Selects a character and enters the world.

Right after the login the client is still receiving the character list, and entering at that moment sends it back to the character list. The method waits for the list: it enters as soon as the character select screen is ready or, when the list has not arrived yet, at the moment the client receives it. With `wait`, it then checks that the character stays in the world for 1.5 seconds and enters again, up to two more times, if the client returned to the character list.

### Parameters

| Name | Type | Required | Default | Description |
| --- | --- | --- | --- | --- |
| `index` | integer | No | `0` | Character slot index, used when `name` is not provided |
| `name` | string | No | | Character name, which takes precedence over `index` |
| `wait` | boolean | No | `true` | Wait until the world is fully loaded, up to 30 seconds |

### Response

```json
{"ok": true}
```

### Errors

| Condition | Error |
| --- | --- |
| Not on the character select screen | `not on character select screen` |
| The name does not exist | `character not found` |
| The index is out of range | `index out of range` |
| The world did not load within 30 seconds | `enter world timeout` |
| The client returned to the character list after three attempts | `the game returned to the character list after entering the world` |

## client.logout

Logs out from the world to the character select screen.

### Parameters

None.

### Response

```json
{"ok": true}
```

!!! note
    The call is posted to the game thread and returns immediately. It does not wait for the logout to complete.

## client.quit

Quits the game client.

### Parameters

None.

### Response

```json
{"ok": true}
```

!!! warning
    The response is sent before the process exits. The pipe connection is closed shortly after.

## client.getCharacters

Returns the characters on the selected realm.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `count` | integer | Number of characters |
| `characters` | array | Character objects |

**Character fields:**

| Field | Type | Description |
| --- | --- | --- |
| `index` | integer | Slot index, starting at 0 |
| `name` | string | Character name |
| `level` | integer | Character level |
| `race` | integer | Race ID |
| `class` | integer | Class ID |
| `gender` | integer | Gender ID |
| `map` | integer | Last known map ID |
| `zone` | integer | Last known zone ID |

!!! note
    Call this method on the character select screen, after `client.login` or `client.refreshCharacters`.

## client.refreshCharacters

Requests a fresh character list from the server.

### Parameters

None.

### Response

```json
{"ok": true}
```

!!! note
    The request is posted to the game thread and does not wait for completion. Call `client.getCharacters` after a short delay.

## client.getRealmlist

Returns the current `realmList` CVar.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `realmList` | string | Current realm list address |

## client.setRealmlist

Sets the `realmList` CVar.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `realmList` | string | Yes | New realm list address |

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | Whether the CVar was set |

!!! note
    Set the realm list while the client is on the login screen. The value is used by the next login attempt.

## EULA and TOS methods

The following methods dismiss the corresponding legal agreement dialog:

| Method | Agreement |
| --- | --- |
| `client.acceptEULA` | End User License Agreement |
| `client.acceptTOS` | Terms of Service |
| `client.acceptTermination` | Termination Without Cause |
| `client.acceptScanning` | Scanning agreement |
| `client.acceptContest` | Contest agreement |
| `client.acceptAll` | All of the above |

### Parameters

None, for all six methods.

### Response

```json
{"ok": true}
```

!!! note
    These calls are designed for the first-run login flow. Calling them when no agreement is displayed is harmless. Each call invokes the game function and hides the agreement frame through Lua.
