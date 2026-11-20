# Métodos dll.*

Gestionan la propia DLL del SDK: la consola superpuesta del juego, la captura de logs y el diagnóstico.

El SDK lee la lista de líneas de la consola de WoW en cada frame para capturar la salida de consola. La mayoría de las entradas provienen del propio juego, como errores de Lua, mensajes de depuración y cambios de CVar.

## dll.toggleConsole

Alterna la visibilidad de la consola superpuesta del juego.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | Siempre `true` |
| `visible` | boolean | Indica si la consola está visible después de la llamada |

## dll.getInfo

Devuelve información general de la DLL, incluido el commit con el que se compiló. Es el único método que también funciona sin el juego: consulta [Leer la DLL sin el juego](#leer-la-dll-sin-el-juego).

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `version` | string | Cadena de versión del SDK |
| `commit` | object o `null` | Commit con el que se compiló la DLL. `null` cuando se compiló fuera de un repositorio Git |
| `inGame` | boolean | `true` cuando la DLL se ejecuta dentro del juego |
| `uptimeMs` | integer | Milisegundos desde la inicialización de la DLL |
| `console` | boolean | Indica si el sistema de consola está inicializado |
| `consoleVisible` | boolean | Indica si la consola superpuesta está visible |
| `logCount` | integer | Entradas de log en el búfer actualmente |
| `logMaxEntries` | integer | Tamaño máximo del búfer de logs |
| `logShowInConsole` | boolean | Indica si los logs capturados se muestran también en la consola |

`uptimeMs`, `console`, `consoleVisible` y los campos `log*` solo aparecen dentro del juego.

**Campos de `commit`:**

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `hash` | string | Hash completo del commit |
| `shortHash` | string | Hash abreviado del commit |
| `date` | string | Fecha del commit en ISO 8601 con segundos y desfase horario, por ejemplo `2026-11-19T12:55:26-05:00` |
| `timestamp` | integer | Fecha del commit en segundos desde el 1970-01-01 UTC |
| `dirty` | boolean | `true` cuando `src/`, `cmake/` o `CMakeLists.txt` tenían cambios sin commitear, así que la DLL no coincide exactamente con el commit |

El commit y su fecha se graban en la DLL al compilar, así que dos builds del mismo commit devuelven los mismos valores. La fecha es la del commit, no la de la compilación.

### Ejemplo

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "dll.getInfo"}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "version": "2.3.0",
    "commit": {
      "hash": "4f09723ebef967c9c66e5ed16a19eb7a42a14d97",
      "shortHash": "4f09723",
      "date": "2026-11-19T12:55:26-05:00",
      "timestamp": 1795110926,
      "dirty": false
    },
    "inGame": true,
    "uptimeMs": 123456,
    "console": true,
    "consoleVisible": false,
    "logCount": 42,
    "logMaxEntries": 1000,
    "logShowInConsole": false
  }
}
```

### Leer la DLL sin el juego

La DLL solo arranca el SDK cuando la carga el juego. Si la carga cualquier otro programa, no inicia hooks, pipes ni hilos, y solo responde a través de su función exportada:

```cpp
extern "C" int __cdecl WoWControlSDK_Invoke(const char* request, char* response, int responseSize);
```

`request` es una petición JSON-RPC. La función escribe la respuesta JSON-RPC en `response`, terminada en un carácter nulo, y devuelve su tamaño contando ese carácter. Cuando `responseSize` es demasiado pequeño, no escribe nada y devuelve el tamaño que necesita, así que una primera llamada con `nullptr` y `0` da el tamaño del búfer. Solo está disponible `dll.getInfo`; cualquier otro método devuelve el error `-32601`.

La DLL es de 32 bits, así que solo un proceso de 32 bits puede cargarla. El inyector lo hace con [`--call`](../injection.md#leer-la-dll-sin-el-juego):

```powershell
.\WowInjector.exe --dll .\WoWControlSDK.dll --call dll.getInfo
```

```json
{"id":1,"jsonrpc":"2.0","result":{"commit":{"date":"2026-11-19T12:55:26-05:00","dirty":false,"hash":"4f09723ebef967c9c66e5ed16a19eb7a42a14d97","shortHash":"4f09723","timestamp":1795110926},"inGame":false,"version":"2.3.0"}}
```

Un programa de 64 bits puede leer la versión sin cargar la DLL: el recurso de versión de Windows tiene `FileVersion`, `ProductVersion` (versión, commit abreviado, `-dirty` cuando tenía cambios sin commitear, y fecha, por ejemplo `2.3.0 (4f09723, 2026-11-19T12:55:26-05:00)`), `Commit` y `CommitDate`. Se leen con `GetFileVersionInfo` y `VerQueryValue`, y `ProductVersion` aparece en **Propiedades → Detalles** del archivo.

## dll.getLogs

Devuelve las entradas de log almacenadas en el búfer.

### Parámetros

| Nombre | Tipo | Obligatorio | Por defecto | Descripción |
| --- | --- | --- | --- | --- |
| `offset` | integer | No | `0` | Posición del búfer desde la que empezar |
| `count` | integer | No | `50` | Número de entradas que se devuelven |

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `total` | integer | Entradas de log en el búfer |
| `returned` | integer | Entradas devueltas en esta respuesta |
| `offset` | integer | Desplazamiento usado por la petición |
| `entries` | array | Objetos de entrada de log |

**Campos de entrada:**

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `text` | string | Mensaje del log |
| `style` | integer | Gravedad: `0` información, `1` advertencia, `2` error |
| `timestampMs` | integer | Marca de tiempo en milisegundos |

## dll.setLogConfig

Configura la captura de logs.

### Parámetros

| Nombre | Tipo | Obligatorio | Por defecto | Descripción |
| --- | --- | --- | --- | --- |
| `maxEntries` | integer | No | Valor actual, `1000` al iniciar | Número máximo de entradas almacenadas |
| `showInConsole` | boolean | No | Valor actual, `false` al iniciar | Muestra los logs capturados en la consola del juego |

Los parámetros que no se envían mantienen su valor actual. Cuando `showInConsole` se activa, o la consola se abre más tarde, primero se muestran los mensajes guardados, así que no se pierden los mensajes del inicio del juego. La consola se escribe desde un hilo aparte, así que mostrar los logs nunca ralentiza el juego.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | Siempre `true` |
| `maxEntries` | integer | Máximo configurado |
| `showInConsole` | boolean | Ajuste de eco configurado |

!!! note
    Reducir `maxEntries` recorta de inmediato las entradas más antiguas.
