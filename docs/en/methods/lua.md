# lua.* methods

Execute, evaluate and manage globals in WoW's Lua VM.

Lua calls use `lua_pcall` with `errfunc=0` and are safe to run from the RPC pipe thread. Only `boolean`, `number` and `string` values cross the JSON boundary; tables, functions and userdata become `null`.

## lua.execute

Executes Lua code and discards every return value.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `code` | string | Yes | Lua source code to execute |

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | `true` when the code was submitted |
| `error` | string | Present only when `code` is empty |

### Example

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.execute", "params": {"code": "print('hello from the SDK')"}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true}}
```

!!! tip
    Use `lua.evaluate` when the result of the Lua code is needed.

## lua.evaluate

Executes Lua code and returns its result with typed coercion.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `code` | string | Yes | Lua expression or code to evaluate |

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | Whether evaluation succeeded |
| `error` | string | Present only when `ok` is `false` |
| `result` | any | Return value, following the table below |

**Return value behavior:**

| Lua returns | `result` |
| --- | --- |
| No values | `null` |
| One value | The value itself: boolean, number or string |
| More than one value | Array of values |

### Example

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.evaluate", "params": {"code": "return UnitName('player')"}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "result": "PlayerName"}}
```

!!! note
    Tables, functions and userdata are not serialized. They become `null` in the result.

## lua.getGlobal

Reads the value of a Lua global variable.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `name` | string | Yes | Name of the global variable |

### Response

The response uses the same `ok`, `error` and `result` fields as `lua.evaluate`.

### Example

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.getGlobal", "params": {"name": "MyAddonTable"}}
```

!!! tip
    If the global does not exist, `ok` is `true` and `result` is `null`.

## lua.setGlobal

Sets a Lua global variable to a boolean, number or string.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `name` | string | Yes | Name of the global variable |
| `value` | boolean, number or string | No | Value to set. Omitted or unsupported values set the global to `nil` |

### Response

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true}}
```

### Example

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.setGlobal", "params": {"name": "MyVar", "value": 42}}
```

!!! note
    Arrays and objects are not supported. Use `lua.execute` to build tables.

## lua.createNamespace

Creates an empty Lua table as a global, which is useful as a namespace for addon variables.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `name` | string | Yes | Name for the new table global |

### Response

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true}}
```

### Example

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.createNamespace", "params": {"name": "MyAddon"}}
```

!!! warning
    If the name already exists as a global, it is overwritten.
