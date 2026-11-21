# Integración con addons

Un addon puede comunicarse con la aplicación que controla el juego a través del SDK. El addon envía eventos con `SDK.Emit`, y la aplicación llama las funciones del addon con [`lua.call`](methods/lua.md#luacall).

El repositorio incluye un ejemplo completo en [`tools/addons/WoWControlSDKExample`](https://github.com/Zerik-Official/WoWControlSDK/tree/main/tools/addons/WoWControlSDKExample). Usa todas las funciones de esta página y se puede copiar como punto de partida.

## Instalar el ejemplo

1. Copia la carpeta `WoWControlSDKExample` en `World of Warcraft\Interface\AddOns\`.
2. Inicia el juego con el SDK inyectado, por ejemplo desde el [cliente de escritorio](operations/cli.md).
3. Entra al mundo. El chat muestra `Connected as WoWControlSDKExample` y el pipe de eventos recibe `lua.custom` con `name: example.ready`.

## Obtener el objeto SDK

Añade esta línea al archivo `.toc` del addon:

```text
## X-WoWControlSDK: 1
```

Cuando el juego carga el addon desde `Interface\AddOns`, el SDK coloca un objeto `SDK` de solo lectura en la tabla privada que recibe cada archivo del addon:

```lua
local addonName, ns = ...
local SDK = ns.SDK

if not SDK then
    return
end
```

`ns.SDK` es `nil` cuando el SDK no está inyectado, así que el addon sigue funcionando sin él.

| Campo | Descripción |
| --- | --- |
| `SDK.addon` | Nombre del addon que recibió el objeto |
| `SDK.Emit(name, ...)` | Envía un evento a la aplicación |
| `SDK.Print(text, r, g, b)` | Escribe una línea en el chat; consulta [Mensajes en el chat](#mensajes-en-el-chat) |
| `SDK.RegisterCommand(names, handler)` | Añade un comando de chat sin globales; consulta [Comandos de chat](#comandos-de-chat) |
| `SDK.UnregisterCommand(name)` | Quita un comando registrado por el mismo addon |
| `SDK.Expose(name, function)` | Permite que la aplicación llame a la función con `lua.call`; consulta [De la aplicación al addon](#de-la-aplicacion-al-addon-luacall) |
| `SDK.Save(key, value)`, `SDK.Load(key, default)`, `SDK.Delete(key)`, `SDK.Keys()` | Guardan datos en disco sin SavedVariables; consulta [Datos guardados](#datos-guardados) |
| `SDK.OnDataChanged(function)` | Se ejecuta cuando la aplicación cambia una clave guardada |

### Quién puede acceder al objeto

No existe un `SDK` global. Solo los archivos que el juego carga desde la carpeta del addon reciben el objeto. El código de otros orígenes no puede acceder a él:

- `/run`, `/script`, `RunScript` y `loadstring`.
- El Lua que envía el servidor, incluidos los paquetes de Warden.
- Los mensajes de addon de otros jugadores (`SendAddonMessage`) y los enlaces de chat.

Guarda el objeto en una variable local y nunca lo guardes en un global ni en una tabla que otro código pueda leer, porque ese código podría enviar eventos en nombre del addon.

La aplicación también puede limitar el puente a addons concretos con [`lua.setAllowedAddons`](methods/lua.md#luasetallowedaddons). Los addons que no están en la lista no reciben el objeto y sus eventos se descartan.

## Del addon a la aplicación: `SDK.Emit`

```lua
SDK.Emit("example.ready", { player = UnitName("player"), level = UnitLevel("player") })
```

La aplicación recibe un evento [`lua.custom`](methods/events.md#cargas-utiles-de-los-eventos-del-sdk):

```json
{"event": "lua.custom", "args": {"name": "example.ready", "addon": "WoWControlSDKExample", "args": [{"player": "PlayerName", "level": 80}]}}
```

| Regla | Valor |
| --- | --- |
| Nombre del evento | String de 1 a 64 caracteres. Cualquier otro valor lanza un error de Lua, así que envuelve `SDK.Emit` en `pcall` cuando el nombre no es fijo |
| Argumentos | Strings, números, booleanos, `nil` y tablas, convertidos como se describe en [Valores](methods/lua.md#valores). Las funciones y userdata pasan a ser marcadores `{"$type": ...}` |
| Tamaño | Hasta 64 argumentos, 3 niveles de tablas anidadas y 200 entradas de tablas por evento. El evento incluye `truncated: true` cuando un límite lo cortó |
| Frecuencia | Hasta 256 eventos por frame. El exceso se descarta y se informa una vez como `lua.dropped` con un `count` |
| Entrega | En el siguiente frame, en el orden en que se emitieron |

Pon el nombre del addon como prefijo de los eventos, como `example.ready`, para que la aplicación distinga los addons. El SDK siempre rellena `addon` y el addon no puede falsificarlo.

## De la aplicación al addon: `lua.call`

Expón funciones con `SDK.Expose` y llámalas desde la aplicación con `lua.call` y el nombre del addon. Todos los valores de retorno vuelven como JSON:

```lua
SDK.Expose("Ping", function(text)
    return "pong", text
end)
```

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.call", "params": {"addon": "WoWControlSDKExample", "function": "Ping", "args": ["hi"]}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "result": ["pong", "hi"], "results": ["pong", "hi"]}}
```

Las funciones expuestas se quedan dentro del SDK. Una tabla global también funciona con `lua.call` y una ruta, pero entonces cualquier addon o `/run` puede encontrar las funciones y llamarlas; `SDK.Expose` no deja nada en `_G`. Los nombres son identificadores de Lua de hasta 64 caracteres, y exponer otra vez un nombre reemplaza la función.

El ejemplo expone:

| Función | Descripción |
| --- | --- |
| `Ping(text)` | Devuelve `"pong"` y el texto |
| `GetStatus()` | Devuelve el jugador, el nivel, la zona y las opciones como una tabla |
| `ShowMessage(text)` | Muestra el texto en un panel y emite `example.messageShown` |
| `HideMessage()` | Oculta el panel |
| `SetOption(key, value)` | Cambia una opción conocida, la guarda y emite `example.optionChanged` |
| `Request(id, action)` | Responde con un evento `example.response` que lleva el mismo `id` |

El ejemplo no deja globales: su frame no tiene nombre, sus opciones se guardan con `SDK.Save` y `/wcs` se registra con `SDK.RegisterCommand`. `/wcs status` lo comprueba.

### Petición y respuesta

`Request` muestra cómo responder trabajo que termina más tarde, por ejemplo después de un evento del juego. La aplicación envía un id y espera el evento con ese mismo id:

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.call", "params": {"addon": "WoWControlSDKExample", "function": "Request", "args": [7, "status"]}}
```

```json
{"event": "lua.custom", "args": {"name": "example.response", "addon": "WoWControlSDKExample", "args": [{"id": 7, "ok": true, "data": {"addon": "WoWControlSDKExample", "version": "1.0.0", "player": "PlayerName", "level": 80, "zone": "Dalaran", "options": {"forwardLevelUp": true, "greeting": "Hello from WoWControlSDK"}}}]}}
```

## Otras funciones para addons

| Función | Ejemplo |
| --- | --- |
| Leer los datos guardados | [`lua.getStorage`](methods/lua.md#luagetstorage) con `{"addon": "WoWControlSDKExample"}` |
| Cambiar los datos guardados | [`lua.setStorage`](methods/lua.md#luasetstorage) con `{"addon": "WoWControlSDKExample", "key": "options", "value": {"forwardLevelUp": false, "greeting": "Hola"}}` |
| Vigilar llamadas de una función global | [`lua.hook`](methods/lua.md#luahook) con `{"function": "ToggleCharacter"}` envía cada llamada como `lua.hooked` |
| Guardar una referencia a un frame | [`lua.getRef`](methods/lua.md#luagetref) con `{"path": "PlayerFrame"}`, y después `lua.call` con `{"ref": 1, "method": "Hide"}` |
| Varias llamadas en un frame | [`lua.batch`](methods/lua.md#luabatch) con una lista de operaciones |
| Addons con acceso | [`lua.getAddons`](methods/lua.md#luagetaddons) lista los addons que recibieron el objeto |

Las funciones que llama la aplicación se ejecutan contaminadas, igual que el propio addon. No pueden llamar funciones protegidas salvo que la aplicación use `secure: true`, que es [peligroso](methods/lua.md#taint-y-modo-seguro).

## Mensajes en el chat

```lua
SDK.Print("|cff33ccff[MyAddon]|r Conectado")
SDK.Print("Algo falló", 1, 0.2, 0.2)
```

`SDK.Print` escribe en el frame de chat por defecto con el método `AddMessage` del tipo de frame de chat, que el SDK guarda al iniciarse. Muchos addons de chat sustituyen `ChatFrame1.AddMessage` para leer, registrar o cambiar cada línea; esas sustituciones no ven los mensajes de `SDK.Print`. Los valores de color van de `0` a `1` y por defecto son blanco, y el texto se corta en 4096 caracteres. Devuelve `false` cuando no hay frame de chat, por ejemplo en la pantalla de login.

## Comandos de chat

```lua
SDK.RegisterCommand({ "/mytool", "/mt" }, function(message)
    SDK.Emit("mytool.command", message)
end)
```

Un comando registrado de la forma clásica crea `SLASH_MYTOOL1` y una entrada `SlashCmdList.MYTOOL`, así que cualquier código, incluido `/run`, puede encontrar el handler y llamarlo con cualquier texto. `SDK.RegisterCommand` no crea ninguna de las dos: el handler queda dentro del SDK y el juego solo llega a él a través de la lista de comandos escritos que ya mantiene (`hash_SlashCmdList`). El SDK ejecuta el handler solo cuando el texto de la caja de chat que lo envió empieza de verdad por el comando y contiene el mismo mensaje, así que llamar a esa entrada de la lista con otro texto no hace nada.

| Regla | Detalle |
| --- | --- |
| Nombres | Un nombre o una lista de hasta 8, cada uno empezando por `/` y con letras, dígitos o `_`, de hasta 32 caracteres. No distingue mayúsculas |
| Conflictos | Los comandos seguros como `/cast`, los comandos del juego o de otros addons y los comandos de otro addon registrados con el SDK lanzan un error |
| Handler | Recibe el texto que sigue al comando, sin espacios alrededor |
| Macros | `/mytool` en una macro también ejecuta el handler, porque las macros envían sus líneas a través del chat |
| Quitar | `SDK.UnregisterCommand("/mytool")` quita un nombre; devuelve `false` cuando el nombre no lo registró el mismo addon |

Cualquier cosa que pueda hacer que el chat envíe una línea, como una macro o código que rellena una caja de chat y la envía, puede seguir ejecutando el comando, como si el usuario lo escribiera. Trata el mensaje como texto no confiable.

## Datos guardados

El juego solo guarda las SavedVariables en globales, así que cualquier addon puede leerlas y cambiarlas, y solo se escriben en disco cuando el jugador sale o recarga. El SDK guarda los datos de un addon fuera de `_G` y los escribe en disco en cuanto cambian:

```lua
local options = SDK.Load("options", { volume = 1 })
options.volume = 0.5
SDK.Save("options", options)

SDK.OnDataChanged(function(key, value)
    if key == "options" then
        options = value
    end
end)
```

| Función | Descripción |
| --- | --- |
| `SDK.Save(key, value)` | Guarda una copia del valor. `nil` borra la clave |
| `SDK.Load(key, default)` | Devuelve una copia del valor guardado, o `default` cuando la clave no está guardada |
| `SDK.Delete(key)` | Borra la clave |
| `SDK.Keys()` | Las claves guardadas, ordenadas |
| `SDK.OnDataChanged(function(key, value))` | Se ejecuta cuando la aplicación cambia una clave con [`lua.setStorage`](methods/lua.md#luasetstorage); `value` es `nil` cuando la clave se borró. Lo que guarda el propio addon no la llama |

| Regla | Detalle |
| --- | --- |
| Valores | Cadenas, números finitos, booleanos y tablas de ellos, con claves de texto o numéricas. Las funciones, los frames, los ciclos, más de 12 niveles o más de 2000 entradas en un valor lanzan un error |
| Claves | Cadenas de 1 a 128 caracteres; hasta 1000 claves por addon |
| Copias | `Save` y `Load` copian el valor, así que cambiar una tabla después de guardarla no cambia los datos guardados hasta que se vuelve a llamar a `Save` |
| Disponibilidad | Los datos están listos cuando se ejecuta el primer archivo del addon, así que se pueden leer al cargar sin esperar a `ADDON_LOADED` |
| Archivo | `<carpeta del juego>\WTF\WoWControlSDK\AddonData\<addon>.json`, que se escribe menos de un cuarto de segundo después de un cambio. El archivo lo comparten todas las cuentas y personajes de esa carpeta del juego |
| Claves numéricas | Las tablas se guardan como JSON, así que una tabla con claves `1` a `n` vuelve como lista y las demás claves numéricas vuelven como texto |

Solo los addons tienen datos guardados; los scripts cargados con `lua.loadScript` no.

## Comandos de chat del ejemplo

| Comando | Descripción |
| --- | --- |
| `/wcs status` | Muestra el nombre del addon, que no existe un `SDK` global y que el objeto es de solo lectura |
| `/wcs ping [texto]` | Envía `example.ping` |
| `/wcs types` | Envía un evento con todos los tipos de valor admitidos |
| `/wcs flood` | Envía 300 eventos para mostrar el límite por frame y `lua.dropped` |

## Scripts sin carpeta de addon

Una aplicación también puede enviar código Lua por el pipe con [`lua.loadScript`](methods/lua.md#lualoadscript). El SDK lo ejecuta como un addon virtual llamado `@nombre`, con su propio objeto `SDK`, así que el usuario no tiene que copiar una carpeta en `Interface\AddOns` ni reiniciar el juego. Un script admite hasta 1 MiB de código, el límite de petición del pipe de comandos.

| | Addon | Script |
| --- | --- | --- |
| Instalación | Una carpeta en `Interface\AddOns` con un `.toc` | `lua.loadScript` desde la aplicación |
| Nombre | El nombre de la carpeta | `@nombre`; los nombres que empiezan por `@` están reservados para scripts |
| Objeto `SDK` | `ns.SDK` | `ns.SDK`, con `SDK.script = true` |
| Tras `/reload` | El juego lo vuelve a cargar | El SDK lo vuelve a ejecutar cuando `persist` es `true` |
| SavedVariables, XML y plantillas de un `.toc` | Sí | No |
| Datos guardados con `SDK.Save` | Sí | No |
| Se puede quitar sin `/reload` | No | Sí, con `lua.unloadScript` |

Un script recibe los mismos argumentos que un archivo de addon:

```lua
local name, ns = ...
local SDK = ns.SDK

local frame = CreateFrame("Frame", "MyToolFrame", UIParent)
frame:RegisterEvent("PLAYER_TARGET_CHANGED")
frame:SetScript("OnEvent", function()
    SDK.Emit("tool.target", UnitName("target"))
end)

SDK.RegisterCommand("/mytool", function(text)
    SDK.Emit("tool.command", text)
end)

SDK.OnUnload(function()
    print(name .. " unloaded")
end)
```

### Funciones extra del SDK para scripts

| Función | Descripción |
| --- | --- |
| `SDK.OnUnload(function)` | Ejecuta la función cuando se descarga el script, antes de que el SDK quite sus recursos. La última registrada se ejecuta primero |
| `SDK.HookScript(frame, handler, function)` | Igual que `frame:HookScript`, pero el hook deja de ejecutarse al descargar el script. Úsala con frames que el script no creó |
| `SDK.IsActive()` | `false` cuando el script ya se descargó |

Después de descargarlo, `SDK.Emit`, `SDK.Print`, `SDK.RegisterCommand` y `SDK.Expose` lanzan un error, así que los temporizadores o callbacks antiguos no pueden actuar en nombre del script. Las funciones que expuso el script se quitan.

### Qué quita la descarga

El SDK ejecuta cada script en su propio entorno y registra lo que crea:

| Recurso | Al descargar |
| --- | --- |
| Frames creados con `CreateFrame` y sus hijos | Se anulan sus eventos, se quitan sus handlers como `OnEvent` y `OnUpdate` y se ocultan; se borra el nombre global del frame |
| Hooks creados con `hooksecurefunc` o `SDK.HookScript` | Dejan de ejecutarse |
| Globales escritos por el script, incluidas funciones y nombres `SLASH_*` | Recuperan el valor que tenían antes del script |
| Entradas añadidas a `SlashCmdList` | Se quitan, junto con la caché de comandos escritos del juego |
| Comandos registrados con `SDK.RegisterCommand` | Se quitan |

El SDK no ve los cambios hechos a través de otras tablas, como campos nuevos en `StaticPopupDialogs`, `frame:HookScript` sobre un frame que el script no creó o `rawset(_G, ...)`. Deshazlos en una función de `SDK.OnUnload`.

### Momento de ejecución y taint

- El script se ejecuta cuando el jugador ya está en el mundo, después de `PLAYER_LOGIN`, también tras cada `/reload`. Ese evento ya se disparó cuando el script empieza, así que no lo esperes: la interfaz, los addons y los datos del jugador están listos.
- Un script se ejecuta contaminado, con `@nombre` como origen, exactamente igual que un addon. Las funciones protegidas se bloquean y el aviso del juego nombra al script.
- `client.getAddonMemory` informa de la memoria de cada script en ejecución en `scripts`.

## Seguridad

- Trata cada argumento que llega desde la aplicación como datos no confiables. Comprueba su tipo y tamaño antes de usarlo.
- Nunca ejecutes texto recibido como código con `loadstring` o `RunScript`.
- No expongas funciones que lancen hechizos, seleccionen unidades, muevan al personaje o actúen sobre otros jugadores.
- La aplicación también debe tratar los eventos `lua.custom` como datos no confiables, porque vienen del juego.
