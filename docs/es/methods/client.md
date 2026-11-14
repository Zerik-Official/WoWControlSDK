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

## client.login

Realiza una secuencia de login completa. Opcionalmente define las CVars de lista de reinos y nombre de reino, envía las credenciales y espera el resultado del login.

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
{"jsonrpc": "2.0", "id": 1, "method": "client.login", "params": {"username": "myuser", "password": "mypass", "realmName": "MyServer"}}
```

!!! note
    La espera del login es de 20 segundos. Cuando se indica `realmName`, el método también espera hasta 5 segundos a la lista de reinos y hasta 15 segundos a la pantalla de selección de personajes.

!!! warning
    Este método bloquea el hilo RPC mientras sondea con `Sleep(50)`. No lo llames desde código sensible al tiempo.

## client.enterWorld

Selecciona un personaje y entra en el mundo.

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
