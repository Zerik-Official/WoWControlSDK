# Métodos dll.*

Gestiona la DLL del SDK — consola, logging y diagnósticos.

---

El SDK hookea la función `PrintMessage` interna de WoW en `0x00765270` para capturar toda la salida de la consola del juego. La mayoría de los logs capturados vienen del propio WoW — errores de Lua, mensajes de depuración, cambios de CVar, etc.

---

## `dll.toggleConsole`

Alterna la visibilidad de la consola superpuesta en el juego.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `ok` | boolean | siempre `true` |
| `visible` | boolean | si la consola está ahora visible |

---

## `dll.getInfo`

Devuelve información general sobre la DLL del SDK.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `version` | string | versión del SDK |
| `uptimeMs` | integer | milisegundos desde la inicialización de la DLL |
| `console` | boolean | si el sistema de consola está inicializado |
| `consoleVisible` | boolean | si la consola está actualmente visible |
| `logCount` | integer | total de entradas de log en el búfer |
| `logMaxEntries` | integer | tamaño máximo del búfer de log |
| `logShowInConsole` | boolean | si los mensajes de log se muestran en la consola |

### Ejemplo

```json
{"id": 1, "method": "dll.getInfo"}
```

```json
{
  "id": 1,
  "result": {
    "version": "1.0.0",
    "uptimeMs": 123456,
    "console": true,
    "consoleVisible": false,
    "logCount": 42,
    "logMaxEntries": 256,
    "logShowInConsole": false
  }
}
```

---

## `dll.getLogs`

Recupera mensajes de log almacenados en el búfer interno del SDK.

### Parámetros

| Nombre | Tipo | Requerido | Por defecto | Descripción |
|---|---|---|---|---|
| `offset` | integer | no | 0 | posición en el búfer de log desde donde empezar |
| `count` | integer | no | 50 | número de entradas a devolver |

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `total` | integer | total de entradas de log en el búfer |
| `returned` | integer | número de entradas devueltas |
| `offset` | integer | el offset usado |
| `entries` | array | objetos de entrada de log |

**Campos de entrada:**

| Campo | Tipo | Descripción |
|---|---|---|
| `text` | string | texto del mensaje de log |
| `style` | integer | severidad (0=info, 1=advertencia, 2=error) |
| `timestampMs` | integer | timestamp en milisegundos |

---

## `dll.setLogConfig`

Configura el sistema de captura de logs.

### Parámetros

| Nombre | Tipo | Requerido | Por defecto | Descripción |
|---|---|---|---|---|
| `maxEntries` | integer | no | 256 | máximo de entradas de log a almacenar |
| `showInConsole` | boolean | no | false | mostrar logs en la consola del juego |

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `ok` | boolean | siempre `true` |
| `maxEntries` | integer | valor configurado de maxEntries |
| `showInConsole` | boolean | valor configurado de showInConsole |

> [!NOTE]
> Reducir `maxEntries` no elimina inmediatamente el búfer. Las entradas viejas persistirán hasta que sean sobrescritas por nuevas.
