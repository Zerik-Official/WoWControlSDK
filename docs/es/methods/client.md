# Métodos client.*

Controlan la sesión de juego: login, selección de reino, gestión de personajes, detección de pantalla, diagnóstico y automatización de los acuerdos legales.

La mayoría de las llamadas que cambian estado se encolan en el hilo del juego. Las llamadas que necesitan un resultado, como `client.login` y `client.enterWorld`, bloquean el hilo RPC mientras hacen sondeos.

## client.ping

Comprobación de estado. Responde bien siempre que la DLL esté cargada y el pipe de comandos esté escuchando.

### Parámetros

Ninguno.

### Respuesta

```json
{"status": "ok"}
```

## client.getScreen

Devuelve la pantalla actual del juego y el estado de login.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `screen` | string | `"login"`, `"charselect"`, `"world"` o el nombre bruto de la pantalla glue |
| `inWorld` | boolean | Indica si el jugador está en el mundo del juego |
| `loginState` | integer | Valor bruto del estado de login de WoW |

## client.getDebugState

Devuelve el estado de autenticación y conexión de bajo nivel para diagnóstico.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `netClientPtr` | integer | Dirección de memoria del objeto `NetClient` |
| `authResultCode` | integer | Código bruto del resultado de autenticación de WoW |
| `errorFlag` | integer | Flag de error de red |
| `authStatus` | integer | Estado bruto de autenticación |
| `loginState` | integer | Estado bruto de login |
| `screen` | string | Nombre de la pantalla glue actual |
| `inWorld` | boolean | Indica si el jugador está en el mundo del juego |
| `authResultStr` | string | Cadena del resultado de login capturada |

!!! warning
    Los campos son valores brutos de memoria que dependen de la build del cliente. Usa este método para diagnóstico, no para lógica de la aplicación.

## client.getNetStats

Devuelve la latencia y el ancho de banda que el juego mide en su conexión con el servidor, los mismos valores que `GetNetStats()` en Lua. El SDK los lee del cliente de red del juego cada 500 milisegundos y responde desde esa caché, así que la llamada es barata.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `connected` | boolean | `true` dentro del mundo; fuera del mundo los demás valores son `0` |
| `latencyMs` | integer | Tiempo medio de ida y vuelta al servidor en milisegundos |
| `bandwidthIn` | number | Ancho de banda de entrada en KB/s |
| `bandwidthOut` | number | Ancho de banda de salida en KB/s |
| `ageMs` | integer | Milisegundos desde que se leyeron los valores |

```json
{"connected": true, "latencyMs": 48, "bandwidthIn": 2.418, "bandwidthOut": 0.351, "ageMs": 212}
```

!!! note
    El juego promedia la latencia de sus últimos mensajes, así que el valor cambia despacio. Consultar más de una vez por segundo no aporta información.

## client.getFramerate

Devuelve los fotogramas por segundo, promediados por el juego en sus últimos 30 frames, el mismo valor que `GetFramerate()` en Lua. Usa la misma caché de 500 milisegundos que `client.getNetStats`.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `fps` | number | Fotogramas por segundo, con un decimal |
| `ageMs` | integer | Milisegundos desde que se leyó el valor |

```json
{"fps": 59.8, "ageMs": 140}
```

## client.getAddonMemory

Devuelve la memoria que usa cada addon cargado y toda la Lua VM, los mismos valores que `GetAddOnMemoryUsage()` y `collectgarbage("count")`. La llamada se ejecuta en el hilo del juego.

Medir la memoria recorre todos los objetos de Lua, lo que puede tardar unos milisegundos con muchos addons. El SDK mide como mucho una vez por segundo; una llamada anterior responde con la última medición, y `ageMs` indica su antigüedad.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `addons` | array | Addons que usan memoria, de mayor a menor, como `{"name", "kilobytes"}` |
| `scripts` | array | Scripts cargados con [`lua.loadScript`](lua.md#lualoadscript), como `{"name", "kilobytes"}` |
| `addonsKilobytes` | number | Suma de los addons |
| `totalKilobytes` | number | Memoria de toda la Lua VM, incluida la interfaz de Blizzard |
| `ageMs` | integer | Milisegundos desde la medición |

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

Los addons que no están cargados no aparecen.

## client.getLastCrash

Devuelve el último error fatal del juego, como `ERROR #132 (0x85100084) Fatal Exception`, con el log que el juego escribió en su carpeta `Errors`.

El SDK registra un manejador en el propio sistema de errores del juego. Se ejecuta después de que el juego escribe el log y antes de que muestre la ventana de error, así que los datos están listos mientras la ventana está abierta. Los mismos datos se envían al pipe de eventos como [`client.crashed`](events.md#cargas-utiles-de-los-eventos-del-sdk).

### Parámetros

Ninguno.

### Respuesta

Sin errores en esta sesión:

```json
{"crashed": false}
```

Después de un error:

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `crashed` | boolean | `true` |
| `kind` | string | `"crash"` para una excepción, `"error"` para otros errores fatales |
| `errorNumber` | integer | El número que muestra el juego, por ejemplo `132` |
| `code` | string | Código de error completo, por ejemplo `"0x85100084"` |
| `title` | string | Título del error, por ejemplo `"Fatal Exception"` |
| `description` | string | Descripción corta, como la excepción y su dirección |
| `details` | string | Explicación más larga escrita por el juego |
| `exception` | object o `null` | `{"code", "address", "module", "offset"}` de la excepción; `null` en otros errores |
| `logPath` | string o `null` | Ruta del log en la carpeta `Errors`. `null` cuando el juego no escribió ningún log. Un crash que dañe la memoria del proceso también puede dejar la ruta ilegible |
| `log` | string o `null` | Contenido del log, hasta 64 KB. `null` cuando no se puede leer el log, por ejemplo si `logPath` es `null` o está dañada |
| `logTruncated` | boolean | Presente y `true` cuando el log ocupaba más de 64 KB |
| `time` | integer | Milisegundos desde que se inició Windows |

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
    Un crash puede dejar al juego sin poder responder. Un launcher también debería leer el archivo más reciente de la carpeta `Errors` cuando termina el proceso del juego, por si no llegó ni el evento ni este método.

## client.login

Realiza una secuencia de login completa. Opcionalmente define las CVars de lista de reinos y nombre de reino, envía las credenciales y espera el resultado del login.

La pantalla de login ignora las credenciales hasta que el cliente termina sus comprobaciones de inicio y acepta los acuerdos de usuario, así que un login enviado en cuanto aparece la pantalla se perdería. El método espera hasta 30 segundos a que el cliente acepte un login antes de enviar las credenciales, así que se puede llamar en cuanto se informa de la pantalla de login.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `username` | string | Sí | Usuario de la cuenta |
| `password` | string | Sí | Contraseña de la cuenta |
| `realmList` | string | No | Si se indica, define la CVar `realmList` antes del login |
| `realmName` | string | No | Si se indica, selecciona el reino y espera a la pantalla de selección de personajes |

### Respuesta

En caso de éxito:

```json
{"ok": true}
```

En caso de fallo:

```json
{"error": "incorrect password"}
```

Cuando el login se completa pero falla una espera posterior, la respuesta también incluye `ok: false`:

```json
{"ok": false, "error": "realm list timeout"}
```

### Cadenas de error

| Error | Significado |
| --- | --- |
| `not on login screen` | El cliente no está en la pantalla de login |
| `login screen not ready` | El cliente no aceptó un login en 30 segundos |
| `username and password required` | Una de las credenciales está vacía |
| `login failed` | Fallo genérico de autenticación |
| `server down` | El servidor de login está caído |
| `unknown account` | La cuenta no existe |
| `incorrect password` | Contraseña incorrecta |
| `disconnected` | La conexión se cerró |
| `parental control` | El control parental bloquea la cuenta |
| `chargeback` | La cuenta tiene un contracargo |
| `conversion required` | La cuenta debe convertirse |
| `account banned` | La cuenta está baneada |
| `account suspended` | La cuenta está suspendida |
| `account locked` | La cuenta está bloqueada |
| `game account locked` | La cuenta de juego está bloqueada |
| `unlockable lock` | La cuenta tiene un bloqueo desbloqueable |
| `already online` | La cuenta ya tiene una sesión iniciada |
| `bad version` | La versión del cliente no se acepta |
| `no time remaining` | La suscripción ha caducado |
| `database busy` | La base de datos de login está ocupada |
| `trial expired` | El periodo de prueba ha caducado |
| `account converted` | La cuenta fue convertida |
| `login timeout` | No llegó ningún resultado de login en 20 segundos |
| `realm list timeout` | La lista de reinos no estuvo lista en 5 segundos |
| `realm select timeout` | La pantalla de selección de personajes no apareció en 15 segundos |
| `unknown error` | Un resultado de login sin asignar |

### Ejemplo

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "client.login", "params": {"username": "myuser", "password": "mypass", "realmName": "MyServer"}}
```

!!! note
    El método espera primero hasta 30 segundos a que la pantalla de login acepte un login y después hasta 20 segundos al resultado. Cuando se indica `realmName`, el método también espera hasta 5 segundos a la lista de reinos y hasta 15 segundos a la pantalla de selección de personajes.

!!! warning
    Este método bloquea el hilo RPC mientras sondea con `Sleep(50)`. No lo llames desde código sensible al tiempo.

## client.enterWorld

Selecciona un personaje y entra en el mundo.

Justo después del login el cliente todavía está recibiendo la lista de personajes, y entrar en ese momento lo devuelve a la lista. El método espera a la lista: entra en cuanto la pantalla de selección de personajes está lista o, si la lista aún no llegó, en el momento en que el cliente la recibe. Con `wait`, comprueba después que el personaje sigue en el mundo durante 1,5 segundos y vuelve a entrar, hasta dos veces más, si el cliente regresó a la lista de personajes.

### Parámetros

| Nombre | Tipo | Obligatorio | Por defecto | Descripción |
| --- | --- | --- | --- | --- |
| `index` | integer | No | `0` | Índice de la ranura del personaje, usado cuando no se indica `name` |
| `name` | string | No | | Nombre del personaje, que tiene prioridad sobre `index` |
| `wait` | boolean | No | `true` | Espera hasta que el mundo esté completamente cargado, como máximo 30 segundos |

### Respuesta

```json
{"ok": true}
```

### Errores

| Condición | Error |
| --- | --- |
| No se está en la pantalla de selección de personajes | `not on character select screen` |
| El nombre no existe | `character not found` |
| El índice está fuera de rango | `index out of range` |
| El mundo no se cargó en 30 segundos | `enter world timeout` |
| El cliente volvió a la lista de personajes tras tres intentos | `the game returned to the character list after entering the world` |

## client.logout

Cierra la sesión del mundo y vuelve a la pantalla de selección de personajes.

### Parámetros

Ninguno.

### Respuesta

```json
{"ok": true}
```

!!! note
    La llamada se encola en el hilo del juego y responde de inmediato. No espera a que el cierre de sesión termine.

## client.quit

Cierra el cliente del juego.

### Parámetros

Ninguno.

### Respuesta

```json
{"ok": true}
```

!!! warning
    La respuesta se envía antes de que el proceso termine. La conexión del pipe se cierra poco después.

## client.getCharacters

Devuelve los personajes del reino seleccionado.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `count` | integer | Número de personajes |
| `characters` | array | Objetos de personaje |

**Campos de personaje:**

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `index` | integer | Índice de la ranura, empezando en 0 |
| `name` | string | Nombre del personaje |
| `level` | integer | Nivel del personaje |
| `race` | integer | ID de raza |
| `class` | integer | ID de clase |
| `gender` | integer | ID de género |
| `map` | integer | Último mapa conocido |
| `zone` | integer | Última zona conocida |

!!! note
    Llama a este método en la pantalla de selección de personajes, después de `client.login` o `client.refreshCharacters`.

## client.refreshCharacters

Solicita al servidor una lista de personajes actualizada.

### Parámetros

Ninguno.

### Respuesta

```json
{"ok": true}
```

!!! note
    La petición se encola en el hilo del juego y no espera a que termine. Llama a `client.getCharacters` tras una breve espera.

## client.getRealmlist

Devuelve la CVar `realmList` actual.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `realmList` | string | Dirección actual de la lista de reinos |

## client.setRealmlist

Define la CVar `realmList`.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `realmList` | string | Sí | Nueva dirección de la lista de reinos |

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | Indica si la CVar se definió |

!!! note
    Define la lista de reinos mientras el cliente está en la pantalla de login. El valor se usa en el siguiente intento de login.

## Métodos EULA y TOS

Los siguientes métodos descartan el cuadro de acuerdo legal correspondiente:

| Método | Acuerdo |
| --- | --- |
| `client.acceptEULA` | Contrato de licencia de usuario final |
| `client.acceptTOS` | Términos de servicio |
| `client.acceptTermination` | Terminación sin causa |
| `client.acceptScanning` | Acuerdo de escaneo |
| `client.acceptContest` | Acuerdo de concurso |
| `client.acceptAll` | Todos los anteriores |

### Parámetros

Ninguno, para los seis métodos.

### Respuesta

```json
{"ok": true}
```

!!! note
    Estas llamadas están pensadas para el flujo de primer login. Llamarlas cuando no se muestra ningún acuerdo no tiene efecto. Cada llamada invoca la función del juego y oculta el marco del acuerdo mediante Lua.
