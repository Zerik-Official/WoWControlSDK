# client.* Methods

Authentication, session control, character management, realm configuration, and connection diagnostics.

---

## `client.ping`

Health check. Always succeeds if the DLL is loaded and the pipe is connected.

### Parameters

None.

### Response

```json
{ "status": "ok" }
```

---

## `client.getScreen`

Returns the current game screen / UI state.

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `screen` | string | current glue screen name (`"login"`, `"charselect"`, or empty string if in-world) |
| `inWorld` | boolean | whether the player is in the game world |
| `loginState` | integer | raw WoW login state value |

---

## `client.getDebugState`

Detailed low-level authentication and connection state for debugging.

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `netClientPtr` | integer | memory address of the NetClient object |
| `authResultCode` | integer | raw WoW authentication result code |
| `errorFlag` | integer | net error flag |
| `authStatus` | integer | raw authentication status |
| `loginState` | integer | raw login state |
| `screen` | string | glue screen name |
| `inWorld` | boolean | |
| `authResultStr` | string | captured login result string |

> [!WARNING]
> Intended for diagnostic use. Fields are raw memory values that vary by WoW version/build.

---

## `client.login`

Performs a full login sequence: sets realmlist CVars (optional), sends credentials, waits for login result, and optionally waits for realm list and character select.

### Parameters

| Name | Type | Required | Description |
|---|---|---|---|
| `username` | string | yes | account username |
| `password` | string | yes | account password |
| `realmList` | string | no | if provided, sets `realmList` CVar before login |
| `realmName` | string | no | if provided, auto-selects the realm and waits for char select |

### Response

On success:
```json
{ "ok": true }
```

On failure:
```json
{ "error": "<reason>" }
```

### Possible error strings

| Error | Description |
|---|---|
| `"not on login screen"` | Must be called on the login screen |
| `"username and password required"` | Missing credentials |
| `"SERVER_DOWN"` | Server is offline |
| `"BANNED"` | Account banned |
| `"SUSPENDED"` | Account suspended |
| `"INCORRECT_PASSWORD"` | Wrong password |
| `"UNKNOWN_ACCOUNT"` | Account not found |
| `"ALREADYONLINE"` | Character already logged in |
| `"BADVERSION"` | Client version mismatch |
| `"NO_TIME"` | Expired subscription |
| `"realm list timeout"` | Realm list not ready within 5s |
| `"realm select timeout"` | Realm selection took longer than 15s |

### Example

```json
{"id": 1, "method": "client.login", "params": {"username": "myuser", "password": "mypass", "realmName": "MyServer"}}
```

> [!NOTE]
> Login timeout is 20 seconds. If `realmName` is provided, additionally waits up to 5 seconds for realm list readiness and up to 15 seconds for the character select screen.

> [!CAUTION]
> This method blocks the RPC thread with polling (`Sleep(50)`). Avoid calling it from time-sensitive code.

---

## `client.enterWorld`

Select a character and enter the world. Optionally waits for world loading to complete.

### Parameters

| Name | Type | Required | Default | Description |
|---|---|---|---|---|
| `index` | integer | no | 0 | character slot index (used if `name` is not provided) |
| `name` | string | no | | character name (overrides `index`) |
| `wait` | boolean | no | true | if true, blocks until world is fully loaded (up to 30s) |

### Response

```json
{ "ok": true }
```

### Errors

| Condition | Error |
|---|---|
| Not on character select | `"not on character select screen"` |
| Name not found | `"character not found"` |
| Index out of range | `"index out of range"` |
| World load timeout (30s) | `"enter world timeout"` |

---

## `client.logout`

Logs out from the world to the character select screen.

### Parameters

None.

### Response

```json
{ "ok": true }
```

> [!NOTE]
> Posted asynchronously to the game thread. Returns immediately without waiting for logout to complete.

---

## `client.quit`

Quits the game client entirely.

### Parameters

None.

### Response

```json
{ "ok": true }
```

> [!WARNING]
> The response is sent before the game exits. The pipe connection will be terminated shortly after.

---

## `client.getCharacters`

Returns the list of characters on the currently selected realm. Must be called on the character select screen.

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `count` | integer | number of characters |
| `characters` | array | character objects |

**Character fields:**

| Field | Type | Description |
|---|---|---|
| `index` | integer | slot index (0-based) |
| `name` | string | |
| `level` | integer | |
| `race` | integer | race ID |
| `class` | integer | class ID |
| `gender` | integer | gender ID |
| `map` | integer | last known map ID |
| `zone` | integer | last known zone ID |

---

## `client.refreshCharacters`

Requests the client to re-fetch the character list from the server.

### Parameters

None.

### Response

```json
{ "ok": true }
```

> [!NOTE]
> This is posted to the game thread and does NOT wait for completion. Call `client.getCharacters` after a short delay to get the updated list.

---

## `client.getRealmlist`

Returns the current value of the `realmList` CVar.

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `realmList` | string | current realmlist address |

---

## `client.setRealmlist`

Sets the `realmList` CVar to a new value.

### Parameters

| Name | Type | Required | Description |
|---|---|---|---|
| `realmList` | string | yes | new realmlist address |

### Response

| Field | Type | Description |
|---|---|---|
| `ok` | boolean | whether the CVar was successfully set |

> [!NOTE]
| Requires the game to be on the login screen. Executed on the game thread.

---

## `client.acceptEULA`

Accepts the End User License Agreement dialog.

## `client.acceptTOS`

Accepts the Terms of Service dialog.

## `client.acceptTermination`

Accepts the Termination Without Cause agreement.

## `client.acceptScanning`

Accepts the Scanning agreement.

## `client.acceptContest`

Accepts the Contest Agreement.

## `client.acceptAll`

Accepts all legal agreement dialogs at once (calls all of the above).

### Parameters

None (all six methods).

### Response

```json
{ "ok": true }
```

> [!NOTE]
> These are designed for the first-run login flow. Calling them when no agreement is shown is harmless. Each method calls the respective game function and dismisses the agreement UI frame via Lua.
