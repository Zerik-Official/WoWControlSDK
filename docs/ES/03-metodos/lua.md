# Métodos lua.*

Ejecuta, evalúa y gestiona globales de Lua en la máquina virtual de WoW.

---

## `lua.execute`

Ejecución Lua fire-and-forget. No se recoge ningún valor de retorno.

### Parámetros

| Nombre | Tipo | Requerido | Descripción |
|---|---|---|---|
| `code` | string | sí | Código Lua a ejecutar |

### Respuesta

```json
{ "ok": true }
```

### Ejemplo

```json
{"id": 1, "method": "lua.execute", "params": {"code": "print('hola desde SDK')"}}
```

> [!TIP]
> Usa `lua.evaluate` si necesitas el resultado del código Lua.

---

## `lua.evaluate`

Ejecuta código Lua y devuelve el resultado. Seguro para expresiones tipadas arbitrarias via `lua_pcall` con coerción de tipos.

### Parámetros

| Nombre | Tipo | Requerido | Descripción |
|---|---|---|---|
| `code` | string | sí | Expresión o código Lua a evaluar |

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `ok` | boolean | si la evaluación fue exitosa |
| `error` | string | presente solo si `ok` es falso |
| `result` | any | el valor de retorno (ver notas) |

**Comportamiento del valor de retorno:**

| Retornos | Valor de `result` |
|---|---|
| 0 valores | `null` |
| 1 valor | el valor mismo (boolean, number o string) |
| >1 valores | array de valores |

### Ejemplo

```json
{"id": 1, "method": "lua.evaluate", "params": {"code": "return UnitName('player')"}}
```

```json
{"id": 1, "result": {"ok": true, "result": "PlayerName"}}
```

> [!NOTE]
> Solo se soportan tipos Bool, Number y String de Lua. Tablas, funciones y userdata se convierten en `null`.

---

## `lua.getGlobal`

Lee el valor de una variable global de Lua.

### Parámetros

| Nombre | Tipo | Requerido | Descripción |
|---|---|---|---|
| `name` | string | sí | nombre de la variable global |

### Respuesta

Misma estructura que `lua.evaluate` (`ok`, `error`, `result`).

### Ejemplo

```json
{"id": 1, "method": "lua.getGlobal", "params": {"name": "MyAddonTable"}}
```

> [!TIP]
> Si la global no existe, `ok` es `true` pero `result` es `null`.

---

## `lua.setGlobal`

Establece una variable global de Lua a un valor dado.

### Parámetros

| Nombre | Tipo | Requerido | Descripción |
|---|---|---|---|
| `name` | string | sí | nombre de la variable global |
| `value` | boolean\|number\|string | no | valor a establecer. Si se omite o es tipo no soportado, resulta en `nil` |

### Respuesta

```json
{ "ok": true }
```

### Ejemplo

```json
{"id": 1, "method": "lua.setGlobal", "params": {"name": "MyVar", "value": 42}}
```

> [!NOTE]
> Tipos soportados: `boolean` → Lua Bool, `number` → Lua Number, `string` → Lua String. Arrays y objetos no están soportados.

---

## `lua.createNamespace`

Crea una tabla Lua vacía como global (útil como namespace para variables de addons).

### Parámetros

| Nombre | Tipo | Requerido | Descripción |
|---|---|---|---|
| `name` | string | sí | nombre de la nueva tabla global |

### Respuesta

```json
{ "ok": true }
```

### Ejemplo

```json
{"id": 1, "method": "lua.createNamespace", "params": {"name": "MyAddon"}}
```

> [!WARNING]
> Si el nombre ya existe como global, será sobrescrito.
