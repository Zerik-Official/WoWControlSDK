# Métodos client.*

Autenticación, control de sesión, gestión de personajes, configuración de reinos y diagnósticos de conexión.

---

## `client.ping`

Health check. Siempre es exitoso si la DLL está cargada y el pipe conectado.

### Parámetros

Ninguno.

### Respuesta

```json
{ "status": "ok" }
```

---

## `client.getScreen`

Devuelve la pantalla / estado UI actual del juego.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `screen` | string | nombre de la pantalla actual (`"login"`, `"charselect"` o cadena vacía si está en el mundo) |
| `inWorld` | boolean | si el jugador está en el mundo del juego |
| `loginState` | integer | valor crudo del estado de login de WoW |

---

## `client.getDebugState`

Estado detallado de autenticación y conexión de bajo nivel para depuración.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `netClientPtr` | integer | dirección de memoria del objeto NetClient |
| `authResultCode` | integer | código de resultado de autenticación crudo de WoW |
| `errorFlag` | integer | flag de error de red |
| `authStatus` | integer | estado de autenticación crudo |
| `loginState` | integer | estado de login crudo |
| `screen` | string | nombre de la pantalla |
| `inWorld` | boolean | |
| `authResultStr` | string | string del resultado de login capturado |

> [!WARNING]
> Para uso diagnóstico. Los campos son valores de memoria crudos que varían según la versión/build de WoW.

---

## `client.login`

Realiza una secuencia de login completa: establece CVars de realmlist (opcional), envía credenciales, espera el resultado, y opcionalmente espera la lista de reinos y la selección de personaje.

### Parámetros

| Nombre | Tipo | Requerido | Descripción |
|---|---|---|---|
| `username` | string | sí | nombre de usuario de la cuenta |
| `password` | string | sí | contraseña de la cuenta |
| `realmList` | string | no | si se proporciona, establece el CVar `realmList` antes del login |
| `realmName` | string | no | si se proporciona, auto-selecciona el reino y espera la selección de personaje |

### Respuesta

En éxito:
```json
{ "ok": true }
```

En fallo:
```json
{ "error": "<razón>" }
```

### Posibles errores

| Error | Descripción |
|---|---|
| `"not on login screen"` | Debe llamarse en la pantalla de login |
| `"username and password required"` | Faltan credenciales |
| `"SERVER_DOWN"` | El servidor está offline |
| `"BANNED"` | Cuenta baneada |
| `"SUSPENDED"` | Cuenta suspendida |
| `"INCORRECT_PASSWORD"` | Contraseña incorrecta |
| `"UNKNOWN_ACCOUNT"` | Cuenta no encontrada |
| `"ALREADYONLINE"` | Personaje ya conectado |
| `"BADVERSION"` | Versión del cliente incorrecta |
| `"NO_TIME"` | Suscripción expirada |
| `"realm list timeout"` | Lista de reinos no lista en 5s |
| `"realm select timeout"` | Selección de reino tomó más de 15s |

### Ejemplo

```json
{"id": 1, "method": "client.login", "params": {"username": "myuser", "password": "mypass", "realmName": "MyServer"}}
```

> [!NOTE]
> El timeout de login es de 20 segundos. Si se proporciona `realmName`, además espera hasta 5 segundos por la lista de reinos y hasta 15 segundos por la pantalla de selección de personaje.

> [!CAUTION]
> Este método bloquea el hilo RPC con sondeo (`Sleep(50)`). Evita llamarlo desde código sensible al tiempo.

---

## `client.enterWorld`

Selecciona un personaje y entra al mundo. Opcionalmente espera a que la carga del mundo se complete.

### Parámetros

| Nombre | Tipo | Requerido | Por defecto | Descripción |
|---|---|---|---|---|
| `index` | integer | no | 0 | índice del slot del personaje (usado si no se proporciona `name`) |
| `name` | string | no | | nombre del personaje (sobrescribe `index`) |
| `wait` | boolean | no | true | si es true, bloquea hasta que el mundo esté completamente cargado (hasta 30s) |

### Respuesta

```json
{ "ok": true }
```

### Errores

| Condición | Error |
|---|---|
| No está en selección de personaje | `"not on character select screen"` |
| Nombre no encontrado | `"character not found"` |
| Índice fuera de rango | `"index out of range"` |
| Timeout de carga del mundo (30s) | `"enter world timeout"` |

---

## `client.logout`

Cierra la sesión del mundo a la pantalla de selección de personaje.

### Parámetros

Ninguno.

### Respuesta

```json
{ "ok": true }
```

> [!NOTE]
> Se encola asincrónicamente al hilo del juego. Devuelve inmediatamente sin esperar a que el logout se complete.

---

## `client.quit`

Cierra el juego completamente.

### Parámetros

Ninguno.

### Respuesta

```json
{ "ok": true }
```

> [!WARNING]
> La respuesta se envía antes de que el juego salga. La conexión del pipe se terminará poco después.

---

## `client.getCharacters`

Devuelve la lista de personajes en el reino actualmente seleccionado. Debe llamarse en la pantalla de selección de personaje.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `count` | integer | número de personajes |
| `characters` | array | objetos de personaje |

**Campos de personaje:**

| Campo | Tipo | Descripción |
|---|---|---|
| `index` | integer | índice del slot (0-based) |
| `name` | string | |
| `level` | integer | |
| `race` | integer | ID de raza |
| `class` | integer | ID de clase |
| `gender` | integer | ID de género |
| `map` | integer | último ID de mapa conocido |
| `zone` | integer | último ID de zona conocido |

---

## `client.refreshCharacters`

Solicita al cliente que vuelva a obtener la lista de personajes del servidor.

### Parámetros

Ninguno.

### Respuesta

```json
{ "ok": true }
```

> [!NOTE]
> Esto se encola al hilo del juego y NO espera a que se complete. Llama a `client.getCharacters` después de una pequeña demora para obtener la lista actualizada.

---

## `client.getRealmlist`

Devuelve el valor actual del CVar `realmList`.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `realmList` | string | dirección de realmlist actual |

---

## `client.setRealmlist`

Establece el CVar `realmList` a un nuevo valor.

### Parámetros

| Nombre | Tipo | Requerido | Descripción |
|---|---|---|---|
| `realmList` | string | sí | nueva dirección de realmlist |

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `ok` | boolean | si el CVar se estableció exitosamente |

> [!NOTE]
> Requiere que el juego esté en la pantalla de login. Se ejecuta en el hilo del juego.

---

## `client.acceptEULA`

Acepta el diálogo de Acuerdo de Licencia de Usuario Final.

## `client.acceptTOS`

Acepta el diálogo de Términos de Servicio.

## `client.acceptTermination`

Acepta el acuerdo de Terminación Sin Causa.

## `client.acceptScanning`

Acepta el acuerdo de Escaneo.

## `client.acceptContest`

Acepta el Acuerdo de Concurso.

## `client.acceptAll`

Acepta todos los diálogos de acuerdos legales a la vez (llama a todos los anteriores).

### Parámetros

Ninguno (los seis métodos).

### Respuesta

```json
{ "ok": true }
```

> [!NOTE]
> Estos están diseñados para el flujo de primer inicio de sesión. Llamarlos cuando no se muestra ningún acuerdo es inofensivo. Cada método llama a la función del juego respectiva y cierra el frame de UI del acuerdo via Lua.
