# Métodos lua.*

Ejecutan código, llaman funciones, leen y escriben valores, vigilan llamadas y gestionan el puente de addons en la Lua VM de WoW.

## Cómo se ejecutan las llamadas Lua

| Tema | Comportamiento |
| --- | --- |
| Hilo | Todos los métodos `lua.*` se ejecutan en el hilo del juego al inicio del siguiente frame, así que la respuesta llega como mucho un frame después. |
| Tamaño de la petición | El pipe de comandos lee como máximo 4096 bytes por petición, así que el código Lua debe caber en ese límite. Pon el código más grande en un addon y llámalo con `lua.call`. |
| Errores | Una llamada fallida devuelve `{"ok": false, "error": "..."}`. Los errores de Lua también incluyen `stack` con la traza de la pila de Lua. |
| Acciones bloqueadas | Cuando el juego bloquea una función protegida, la llamada devuelve `ok: false` con una lista `blocked`. Consulta [Taint y modo seguro](#taint-y-modo-seguro). |
| Recarga | `/reload` y cerrar sesión crean una Lua VM nueva. Las refs y los hooks se pierden y hay que crearlos de nuevo. |

## Taint y modo seguro

WoW marca como *contaminado* (tainted) el código que no viene de Blizzard. El código contaminado no puede llamar funciones protegidas como `CastSpellByName`, `TargetUnit` o `UseAction`. Si lo intenta, el juego lanza `ADDON_ACTION_BLOCKED` o `ADDON_ACTION_FORBIDDEN` y la función no hace nada.

Por defecto el SDK ejecuta Lua con el origen de taint `WoWControlSDK`, igual que el juego ejecuta un addon. Las funciones protegidas quedan bloqueadas, y todo valor que escribe el SDK, como un global asignado con `lua.setGlobal`, también queda contaminado.

`lua.execute`, `lua.evaluate` y `lua.call` aceptan `"secure": true` para ejecutar el código como código de Blizzard, sin taint.

!!! danger "El modo seguro es peligroso"
    El código seguro puede llamar todas las funciones protegidas: lanzar hechizos, seleccionar unidades, usar objetos y mover al personaje. Usa `secure: true` solo con código que escribiste y controlas, nunca con código o argumentos que vengan de un addon, de un mensaje de chat, del servidor o de otro programa. La interfaz de Blizzard confía en los valores escritos por código seguro, así que un error puede romper las barras de acción, los marcos de banda o el combate hasta el siguiente `/reload`.

Cuando el juego bloquea al SDK:

- La respuesta es `{"ok": false, "error": "blocked by the game: <función>", "blocked": [...]}`, aunque el resto del código se haya ejecutado.
- El pipe de eventos recibe un evento [`lua.blocked`](events.md#cargas-utiles-de-los-eventos-del-sdk).
- El aviso de "acción de interfaz fallida" del juego se oculta. Usa [`lua.setBlockedNotice`](#luasetblockednotice) para mostrarlo de nuevo. Los bloqueos causados por addons reales nunca se ocultan.

`ok: true` solo significa que el código se ejecutó. Por ejemplo, `CastSpellByName` con un hechizo en reutilización devuelve `ok: true` porque el juego aceptó la llamada, aunque el hechizo no se lance.

## Valores

Los valores de Lua se convierten a JSON así:

| Lua | JSON |
| --- | --- |
| `nil` | `null` |
| boolean | boolean |
| number | number; los enteros hasta 2^53 se escriben sin decimales |
| string | string; los bytes UTF-8 inválidos se reemplazan |
| tabla con claves `1..n` | array |
| otra tabla | objeto; las claves numéricas pasan a ser strings |
| función | `{"$type": "function"}` |
| userdata, como el handle de un frame | `{"$type": "userdata"}` |
| corrutina | `{"$type": "thread"}` |
| tabla que ya se está convirtiendo | `{"$type": "table", "$cycle": true}` |
| tabla que supera el límite de profundidad o de entradas | `{"$type": "table", "$truncated": true}` |

Cuando un límite corta una tabla, la respuesta también incluye `"truncated": true`. Los métodos que devuelven valores aceptan estos límites:

| Nombre | Tipo | Por defecto | Rango | Descripción |
| --- | --- | --- | --- | --- |
| `maxDepth` | integer | `4` | `0` a `16` | Niveles de tablas anidadas que se convierten |
| `maxEntries` | integer | `500` | `1` a `5000` | Total de entradas de tablas que se convierten por respuesta |

Los argumentos enviados a Lua aceptan los mismos valores JSON: los arrays y objetos pasan a ser tablas, con hasta 16 niveles de profundidad. `{"$ref": n}` pasa el valor guardado en una [ref](#luagetref).

### Campos del resultado

Los métodos que devuelven valores de Lua responden con:

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | `true` cuando el código se ejecutó sin errores ni acciones bloqueadas |
| `result` | any | `null` sin valores, el propio valor con un valor, o un array con varios valores |
| `results` | array | Todos los valores devueltos, siempre como array |
| `truncated` | boolean | Presente y `true` cuando un límite cortó una tabla |
| `error` | string | Presente cuando `ok` es `false` |
| `stack` | string | Traza de la pila de Lua, presente en los errores de Lua |
| `blocked` | array | Acciones bloqueadas, presente cuando el juego bloqueó una función protegida |

Cada entrada de `blocked` es `{"event": "ADDON_ACTION_FORBIDDEN" | "ADDON_ACTION_BLOCKED", "function": "<función>"}`.

Usa `results` en código nuevo. `result` mantiene la forma que tenía antes de SDK 2.2.

## Rutas

`lua.call`, `lua.getGlobal`, `lua.setGlobal`, `lua.createNamespace`, `lua.getRef` y los métodos de hooks reciben una ruta en lugar de código Lua:

| Ruta | Significado |
| --- | --- |
| `UnitName` | El global `UnitName` |
| `MyAddonDB.options.volume` | Campo `volume` de `options` del global `MyAddonDB` |
| `RAID_CLASS_COLORS.MAGE` | Campo `MAGE` de una tabla global |
| `MyAddonDB.list.3` | Índice `3` de una tabla |
| `PlayerFrame:Hide` | Método `Hide` de `PlayerFrame`, llamado con `PlayerFrame` como `self`; solo en `lua.call` y en los métodos de hooks |

Una ruta empieza con un identificador de Lua, sigue con identificadores o índices de hasta 9 dígitos separados por `.` y tiene como máximo 256 caracteres. Las rutas nunca se ejecutan como código.

## lua.execute

Ejecuta código Lua y descarta los valores que devuelve.

### Parámetros

| Nombre | Tipo | Obligatorio | Por defecto | Descripción |
| --- | --- | --- | --- | --- |
| `code` | string | Sí | | Código Lua que se va a ejecutar |
| `chunkName` | string | No | `sdk_exec` | Nombre que aparece en los mensajes de error |
| `secure` | boolean | No | `false` | Ejecuta sin taint. Consulta [Taint y modo seguro](#taint-y-modo-seguro) |

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | `true` cuando el código se ejecutó y no se bloqueó nada |
| `error` | string | `code is required`, o la función bloqueada |
| `blocked` | array | Acciones bloqueadas |

Los errores de Lua no se devuelven: el juego los muestra como cualquier otro error de script. Usa `lua.evaluate` para recibirlos.

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.execute", "params": {"code": "CastSpellByName('Attack')"}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": false, "error": "blocked by the game: CastSpellByName()", "blocked": [{"event": "ADDON_ACTION_FORBIDDEN", "function": "CastSpellByName()"}]}}
```

## lua.evaluate

Ejecuta código Lua y devuelve todos los valores que retorna.

### Parámetros

| Nombre | Tipo | Obligatorio | Por defecto | Descripción |
| --- | --- | --- | --- | --- |
| `code` | string | Sí | | Código Lua; usa `return` para devolver valores |
| `chunkName` | string | No | `sdk_eval` | Nombre que aparece en los errores y en `stack` |
| `secure` | boolean | No | `false` | Ejecuta sin taint |
| `maxDepth` | integer | No | `4` | Consulta [Valores](#valores) |
| `maxEntries` | integer | No | `500` | Consulta [Valores](#valores) |

### Respuesta

Consulta [Campos del resultado](#campos-del-resultado).

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.evaluate", "params": {"code": "return UnitName('player'), UnitLevel('player'), {UnitClass('player')}"}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "result": ["PlayerName", 80, ["Mage", "MAGE"]], "results": ["PlayerName", 80, ["Mage", "MAGE"]]}}
```

Un error de Lua devuelve el mensaje y la pila:

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": false, "error": "sdk_eval:1: attempt to call global 'Nope' (a nil value)", "stack": "sdk_eval:1: in main chunk\n"}}
```

## lua.call

Llama una función de Lua por ruta o por ref sin escribir código Lua. Los argumentos se pasan como valores, así que nunca se ejecutan como código.

### Parámetros

| Nombre | Tipo | Obligatorio | Por defecto | Descripción |
| --- | --- | --- | --- | --- |
| `function` | string | Sí, sin `ref` | | Ruta de la función, como `GetItemInfo` o `PlayerFrame:Hide` |
| `ref` | integer | Sí, sin `function` | | Ref de una función, o de un objeto cuando se indica `method` |
| `method` | string | No | | Con `ref`, nombre del método que se llama en el objeto, que se pasa como `self` |
| `args` | array | No | `[]` | Argumentos, convertidos como se describe en [Valores](#valores) |
| `secure` | boolean | No | `false` | Ejecuta sin taint |
| `maxDepth` | integer | No | `4` | Consulta [Valores](#valores) |
| `maxEntries` | integer | No | `500` | Consulta [Valores](#valores) |

### Respuesta

Consulta [Campos del resultado](#campos-del-resultado).

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.call", "params": {"function": "UnitClass", "args": ["player"]}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "result": ["Mage", "MAGE"], "results": ["Mage", "MAGE"]}}
```

## lua.getGlobal

Lee el valor de una ruta.

### Parámetros

| Nombre | Tipo | Obligatorio | Por defecto | Descripción |
| --- | --- | --- | --- | --- |
| `name` | string | Sí | | Ruta que se lee, como `MyAddonDB.options` |
| `maxDepth` | integer | No | `4` | Consulta [Valores](#valores) |
| `maxEntries` | integer | No | `500` | Consulta [Valores](#valores) |

### Respuesta

Consulta [Campos del resultado](#campos-del-resultado). Un valor que no existe devuelve `ok: true` y `result: null`. Un valor intermedio de la ruta que no es una tabla devuelve un error como `MyAddonDB.options is not a table`.

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.getGlobal", "params": {"name": "RAID_CLASS_COLORS.MAGE"}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "result": {"r": 0.41, "g": 0.8, "b": 0.94}, "results": [{"r": 0.41, "g": 0.8, "b": 0.94}]}}
```

## lua.setGlobal

Escribe un valor en una ruta. Las tablas de la ruta ya deben existir.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `name` | string | Sí | Ruta que se escribe, como `MyAddonDB.options.volume` |
| `value` | any | No | Valor que se escribe, convertido como se describe en [Valores](#valores). Si se omite o es `null`, escribe `nil` |

### Respuesta

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true}}
```

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.setGlobal", "params": {"name": "MyAddonDB.options", "value": {"volume": 0.5, "channels": ["GUILD", "PARTY"]}}}
```

!!! note
    El valor escrito queda contaminado. No sobrescribas globales que lee la interfaz de Blizzard, como frames o tablas de configuración.

## lua.createNamespace

Escribe una tabla vacía en una ruta. Úsalo para crear una tabla antes de escribir campos con `lua.setGlobal`.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `name` | string | Sí | Ruta de la tabla nueva, como `MyTool` o `MyTool.cache` |

### Respuesta

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true}}
```

!!! warning
    Si ya existe un valor en la ruta, se reemplaza.

## lua.getRef

Guarda el valor de una ruta y devuelve un número que lo identifica. Usa las refs para llamar métodos de frames, o para pasar la misma tabla a varias llamadas sin buscarla de nuevo.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `path` | string | Sí | Ruta del valor que se guarda, como `PlayerFrame` |

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | `true` cuando se creó la ref |
| `ref` | integer | Número que se pasa como `ref` o como `{"$ref": n}` |
| `type` | string | Tipo de Lua del valor, como `table` o `function` |

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.getRef", "params": {"path": "PlayerFrame"}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "ref": 1, "type": "table"}}
```

```json
{"jsonrpc": "2.0", "id": 2, "method": "lua.call", "params": {"ref": 1, "method": "SetAlpha", "args": [0.5]}}
```

!!! note
    El SDK guarda hasta 256 refs. Una ref mantiene vivo su valor hasta `lua.releaseRef` o el siguiente `/reload`, así que libera las que ya no necesites.

## lua.releaseRef

Libera una ref creada con `lua.getRef`.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `ref` | integer | Sí | Ref que se libera |

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | `true` cuando la llamada se ejecutó |
| `released` | boolean | `false` cuando la ref no existía |

## lua.hook

Vigila una función de Lua. Cada llamada que hacen el juego o un addon se envía al pipe de eventos como un evento [`lua.hooked`](events.md#cargas-utiles-de-los-eventos-del-sdk) con sus argumentos. La función original sigue funcionando sin cambios.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `function` | string | Sí | Ruta de la función, como `AcceptQuest` o `StaticPopup1Button1:Click` |

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | `true` cuando el hook está activo |
| `function` | string | La ruta |
| `hooked` | boolean | Siempre `true` |

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.hook", "params": {"function": "AcceptQuest"}}
```

El juego no permite quitar un hook. `lua.unhook` detiene los eventos, y `lua.hook` sobre la misma ruta los vuelve a activar.

## lua.unhook

Detiene los eventos de una función vigilada.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `function` | string | Sí | Ruta usada en `lua.hook` |

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | `true` cuando la llamada se ejecutó |
| `function` | string | La ruta |
| `unhooked` | boolean | `false` cuando la ruta no estaba vigilada |

## lua.getHooks

Lista los hooks activos.

### Respuesta

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "hooks": ["AcceptQuest", "StaticPopup1Button1:Click"]}}
```

## lua.batch

Ejecuta varios métodos `lua.*` en el mismo frame y devuelve todos los resultados en una respuesta. El estado del juego no puede cambiar entre las operaciones.

### Parámetros

| Nombre | Tipo | Obligatorio | Por defecto | Descripción |
| --- | --- | --- | --- | --- |
| `operations` | array | Sí | | De 1 a 32 objetos con `method` y `params`. Cualquier método `lua.*` excepto `lua.batch` |
| `stopOnError` | boolean | No | `false` | Se detiene en la primera operación cuyo resultado tiene `error` |

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | `true` cuando el lote se ejecutó, aunque falle alguna operación |
| `results` | array | El resultado de cada operación, en orden |
| `completed` | integer | Número de operaciones ejecutadas |
| `stopped` | boolean | Presente y `true` cuando `stopOnError` detuvo el lote |

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.batch", "params": {"operations": [
  {"method": "lua.call", "params": {"function": "UnitName", "args": ["player"]}},
  {"method": "lua.call", "params": {"function": "GetMoney"}},
  {"method": "lua.getGlobal", "params": {"name": "MyAddonDB.options"}}
]}}
```

## lua.setAllowedAddons

Limita el [puente de addons](../addons.md) a una lista de addons. Los addons que no están en la lista no reciben el objeto `SDK` la próxima vez que cargan, y sus eventos se descartan desde ese momento.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `addons` | array[string] o `null` | Sí | Nombres de carpeta de los addons, o `null` para permitir todos los addons que lo activan |

### Respuesta

Igual que [`lua.getAddons`](#luagetaddons).

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.setAllowedAddons", "params": {"addons": ["WoWControlSDKExample"]}}
```

!!! tip
    Llámalo justo después de inyectar, antes de que el juego cargue los addons, para que solo los addons de la lista reciban el objeto.

## lua.getAddons

Lista los addons que recibieron el objeto `SDK` y la lista de permitidos actual.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | Siempre `true` |
| `granted` | array[string] | Addons que recibieron el objeto desde el último `/reload` |
| `allowed` | array[string] o `null` | La lista de permitidos, o `null` cuando se permiten todos los addons que lo activan |

## lua.setBlockedNotice

Elige si el juego muestra su aviso cuando bloquea una función protegida llamada por el SDK.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `show` | boolean | Sí | `true` para mostrar el aviso, `false` para ocultarlo |

### Respuesta

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "show": true}}
```

El aviso está oculto por defecto. La respuesta y el evento `lua.blocked` informan del bloqueo en ambos casos.

## Cambios en SDK 2.2

| Cambio | Antes | Ahora |
| --- | --- | --- |
| Taint | Lua se ejecutaba como código de Blizzard | Lua se ejecuta contaminado salvo con `secure: true` |
| `lua.execute` | Siempre `ok: true` | `ok: false` cuando el juego bloquea una acción |
| Tablas, funciones y userdata | `null` | Se convierten como se describe en [Valores](#valores) |
| `lua.getGlobal`, `lua.setGlobal`, `lua.createNamespace` | `name` era el nombre de un global | `name` es una [ruta](#rutas); los nombres que no son rutas válidas se rechazan |
| `lua.setGlobal` | Solo booleanos, números y strings | Cualquier valor JSON |
| Hilo | Hilo del pipe | Hilo del juego, en el siguiente frame |
