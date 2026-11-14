# Métodos lua.*

Ejecutan, evalúan y gestionan globales en la Lua VM de WoW.

Las llamadas Lua usan `lua_pcall` con `errfunc=0` y son seguras desde el hilo del pipe RPC. Solo los valores `boolean`, `number` y `string` cruzan la frontera JSON; las tablas, funciones y userdata se convierten en `null`.

## lua.execute

Ejecuta código Lua y descarta todos los valores de retorno.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `code` | string | Sí | Código Lua que se va a ejecutar |

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | `true` cuando el código se envió |
| `error` | string | Presente solo cuando `code` está vacío |

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.execute", "params": {"code": "print('hello from the SDK')"}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true}}
```

!!! tip
    Usa `lua.evaluate` cuando necesites el resultado del código Lua.

## lua.evaluate

Ejecuta código Lua y devuelve su resultado con conversión de tipos.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `code` | string | Sí | Expresión o código Lua que se va a evaluar |

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | Indica si la evaluación se completó |
| `error` | string | Presente solo cuando `ok` es `false` |
| `result` | any | Valor de retorno, según la tabla siguiente |

**Comportamiento del valor de retorno:**

| Lua devuelve | `result` |
| --- | --- |
| Ningún valor | `null` |
| Un valor | El valor en sí: boolean, number o string |
| Más de un valor | Arreglo de valores |

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.evaluate", "params": {"code": "return UnitName('player')"}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "result": "PlayerName"}}
```

!!! note
    Las tablas, funciones y userdata no se serializan. Se convierten en `null` dentro del resultado.

## lua.getGlobal

Lee el valor de una variable global de Lua.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `name` | string | Sí | Nombre de la variable global |

### Respuesta

La respuesta usa los mismos campos `ok`, `error` y `result` que `lua.evaluate`.

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.getGlobal", "params": {"name": "MyAddonTable"}}
```

!!! tip
    Si la global no existe, `ok` es `true` y `result` es `null`.

## lua.setGlobal

Asigna a una variable global de Lua un valor boolean, number o string.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `name` | string | Sí | Nombre de la variable global |
| `value` | boolean, number o string | No | Valor que se va a asignar. Si se omite o no es compatible, la global pasa a `nil` |

### Respuesta

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true}}
```

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.setGlobal", "params": {"name": "MyVar", "value": 42}}
```

!!! note
    No se admiten arreglos ni objetos. Usa `lua.execute` para construir tablas.

## lua.createNamespace

Crea una tabla Lua vacía como global, útil como espacio de nombres para variables de un addon.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `name` | string | Sí | Nombre de la nueva tabla global |

### Respuesta

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true}}
```

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.createNamespace", "params": {"name": "MyAddon"}}
```

!!! warning
    Si el nombre ya existe como global, se sobrescribe.
