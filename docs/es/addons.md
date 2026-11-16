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

Expón funciones en una tabla global y llámalas desde la aplicación. Todos los valores de retorno vuelven como JSON:

```lua
WoWControlSDKExample = {}

function WoWControlSDKExample.Ping(text)
    return "pong", text
end
```

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.call", "params": {"function": "WoWControlSDKExample.Ping", "args": ["hi"]}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "result": ["pong", "hi"], "results": ["pong", "hi"]}}
```

El ejemplo expone:

| Función | Descripción |
| --- | --- |
| `WoWControlSDKExample.Ping(text)` | Devuelve `"pong"` y el texto |
| `WoWControlSDKExample.GetStatus()` | Devuelve el jugador, el nivel, la zona y las opciones como una tabla |
| `WoWControlSDKExample.ShowMessage(text)` | Muestra el texto en un panel y emite `example.messageShown` |
| `WoWControlSDKExample.HideMessage()` | Oculta el panel |
| `WoWControlSDKExample.SetOption(key, value)` | Cambia una opción conocida y emite `example.optionChanged` |
| `WoWControlSDKExample.Request(id, action)` | Responde con un evento `example.response` que lleva el mismo `id` |

### Petición y respuesta

`Request` muestra cómo responder trabajo que termina más tarde, por ejemplo después de un evento del juego. La aplicación envía un id y espera el evento con ese mismo id:

```json
{"jsonrpc": "2.0", "id": 1, "method": "lua.call", "params": {"function": "WoWControlSDKExample.Request", "args": [7, "status"]}}
```

```json
{"event": "lua.custom", "args": {"name": "example.response", "addon": "WoWControlSDKExample", "args": [{"id": 7, "ok": true, "data": {"addon": "WoWControlSDKExample", "version": "1.0.0", "player": "PlayerName", "level": 80, "zone": "Dalaran", "options": {"forwardLevelUp": true, "greeting": "Hello from WoWControlSDK"}}}]}}
```

## Otras funciones para addons

| Función | Ejemplo |
| --- | --- |
| Leer variables guardadas | [`lua.getGlobal`](methods/lua.md#luagetglobal) con `{"name": "WoWControlSDKExampleDB.options"}` |
| Cambiar variables guardadas | [`lua.setGlobal`](methods/lua.md#luasetglobal) con `{"name": "WoWControlSDKExampleDB.options.greeting", "value": "Hola"}` |
| Vigilar llamadas | [`lua.hook`](methods/lua.md#luahook) con `{"function": "WoWControlSDKExample.ShowMessage"}` envía cada llamada como `lua.hooked` |
| Guardar una referencia a un frame | [`lua.getRef`](methods/lua.md#luagetref) con `{"path": "WoWControlSDKExampleFrame"}`, y después `lua.call` con `{"ref": 1, "method": "Hide"}` |
| Varias llamadas en un frame | [`lua.batch`](methods/lua.md#luabatch) con una lista de operaciones |
| Addons con acceso | [`lua.getAddons`](methods/lua.md#luagetaddons) lista los addons que recibieron el objeto |

Las funciones que llama la aplicación se ejecutan contaminadas, igual que el propio addon. No pueden llamar funciones protegidas salvo que la aplicación use `secure: true`, que es [peligroso](methods/lua.md#taint-y-modo-seguro).

## Comandos de chat del ejemplo

| Comando | Descripción |
| --- | --- |
| `/wcs status` | Muestra el nombre del addon, que no existe un `SDK` global y que el objeto es de solo lectura |
| `/wcs ping [texto]` | Envía `example.ping` |
| `/wcs types` | Envía un evento con todos los tipos de valor admitidos |
| `/wcs flood` | Envía 300 eventos para mostrar el límite por frame y `lua.dropped` |

## Seguridad

- Trata cada argumento que llega desde la aplicación como datos no confiables. Comprueba su tipo y tamaño antes de usarlo.
- Nunca ejecutes texto recibido como código con `loadstring` o `RunScript`.
- No expongas funciones que lancen hechizos, seleccionen unidades, muevan al personaje o actúen sobre otros jugadores.
- La aplicación también debe tratar los eventos `lua.custom` como datos no confiables, porque vienen del juego.
