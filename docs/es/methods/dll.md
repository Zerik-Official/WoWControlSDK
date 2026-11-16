# Métodos dll.*

Gestionan la propia DLL del SDK: la consola superpuesta del juego, la captura de logs y el diagnóstico.

El SDK engancha la función interna `PrintMessage` de WoW en `0x00765270` para capturar la salida de consola. La mayoría de las entradas provienen del propio juego, como errores de Lua, mensajes de depuración y cambios de CVar.

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

Devuelve información general de la DLL.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `version` | string | Cadena de versión del SDK |
| `uptimeMs` | integer | Milisegundos desde la inicialización de la DLL |
| `console` | boolean | Indica si el sistema de consola está inicializado |
| `consoleVisible` | boolean | Indica si la consola superpuesta está visible |
| `logCount` | integer | Entradas de log en el búfer actualmente |
| `logMaxEntries` | integer | Tamaño máximo del búfer de logs |
| `logShowInConsole` | boolean | Indica si los logs capturados se muestran también en la consola |

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "dll.getInfo"}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "version": "2.2.0",
    "uptimeMs": 123456,
    "console": true,
    "consoleVisible": false,
    "logCount": 42,
    "logMaxEntries": 1000,
    "logShowInConsole": false
  }
}
```

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
    Reducir `maxEntries` no recorta el búfer de inmediato. Las entradas antiguas se sobrescriben a medida que llegan nuevas.
