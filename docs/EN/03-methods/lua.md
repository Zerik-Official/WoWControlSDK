# lua.* Methods

Execute, evaluate, and manage Lua globals in WoW's Lua VM.

---

## `lua.execute`

Fire-and-forget Lua execution. No return value is collected.

### Parameters

| Name | Type | Required | Description |
|---|---|---|---|
| `code` | string | yes | Lua source code to execute |

### Response

```json
{ "ok": true }
```

### Example

```json
{"id": 1, "method": "lua.execute", "params": {"code": "print('hello from SDK')"}}
```

> [!TIP]
> Use `lua.evaluate` instead if you need the result of the Lua code.

---

## `lua.evaluate`

Execute Lua code and return the result. Safe for arbitrary typed expressions via `lua_pcall` with typed return coercion.

### Parameters

| Name | Type | Required | Description |
|---|---|---|---|
| `code` | string | yes | Lua expression or code to evaluate |

### Response

| Field | Type | Description |
|---|---|---|
| `ok` | boolean | whether evaluation succeeded |
| `error` | string | present only if `ok` is false |
| `result` | any | the return value (see notes) |

**Return value behavior:**

| Returns | `result` value |
|---|---|
| 0 values | `null` |
| 1 value | the value itself (boolean, number, or string) |
| >1 values | array of values |

### Example

```json
{"id": 1, "method": "lua.evaluate", "params": {"code": "return UnitName('player')"}}
```

```json
{"id": 1, "result": {"ok": true, "result": "PlayerName"}}
```

> [!NOTE]
> Only Bool, Number, and String Lua types are supported. Tables, functions, and userdata become `null`.

---

## `lua.getGlobal`

Read the value of a Lua global variable.

### Parameters

| Name | Type | Required | Description |
|---|---|---|---|
| `name` | string | yes | name of the global variable |

### Response

Same structure as `lua.evaluate` (`ok`, `error`, `result`).

### Example

```json
{"id": 1, "method": "lua.getGlobal", "params": {"name": "MyAddonTable"}}
```

> [!TIP]
> If the global does not exist, `ok` is `true` but `result` is `null`.

---

## `lua.setGlobal`

Set a Lua global variable to a given value.

### Parameters

| Name | Type | Required | Description |
|---|---|---|---|
| `name` | string | yes | name of the global variable |
| `value` | boolean\|number\|string | no | value to set. Omitted or unsupported types result in `nil` |

### Response

```json
{ "ok": true }
```

### Example

```json
{"id": 1, "method": "lua.setGlobal", "params": {"name": "MyVar", "value": 42}}
```

> [!NOTE]
| Supported value types: `boolean` → Lua Bool, `number` → Lua Number, `string` → Lua String. Arrays and objects are not supported.

---

## `lua.createNamespace`

Create an empty Lua table as a global (useful as a namespace for addon variables).

### Parameters

| Name | Type | Required | Description |
|---|---|---|---|
| `name` | string | yes | name for the new table global |

### Response

```json
{ "ok": true }
```

### Example

```json
{"id": 1, "method": "lua.createNamespace", "params": {"name": "MyAddon"}}
```

> [!WARNING]
> If the name already exists as a global, it will be overwritten.
