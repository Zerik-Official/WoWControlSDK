# Métodos battlefield.*

Leen las colas de campos de batalla y arenas y responden de forma nativa a la invitación "Ya puedes entrar". Los cambios de las colas también se envían como [eventos del SDK](events.md#cargas-utiles-de-los-eventos-del-sdk): `battlefield.statusChanged` y `battlefield.ready`.

## battlefield.getQueues

Devuelve los dos huecos de cola con los datos de `GetBattlefieldStatus`, `GetBattlefieldPortExpiration`, `GetBattlefieldTimeWaited` y `GetBattlefieldEstimatedWaitTime`.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `queues` | array | Dos objetos de cola, índice `1` y `2` |

**Campos de cada cola:**

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `index` | integer | Hueco de cola, `1` o `2` |
| `status` | string | `"none"`, `"queued"`, `"confirm"` (la invitación está abierta), `"active"` (dentro) o `"error"` |
| `statusId` | integer | Estado sin procesar, de `0` a `3` en el orden anterior |
| `typeId` | integer | ID de `BattlemasterList.dbc`, `32` para un campo de batalla aleatorio |
| `typeName` | string | Nombre localizado del campo de batalla o la arena |
| `random` | boolean | Cola de campo de batalla aleatorio |
| `arena` | boolean | Cola de arena (`teamSize > 0`) |
| `rated` | boolean | Partida puntuada (`registeredMatch`) |
| `teamSize` | integer | `2`, `3` o `5` en arenas, `0` en campos de batalla |
| `mapId` | integer | ID de `Map.dbc`; en una invitación aleatoria abierta, el campo de batalla que eligió el servidor |
| `mapName` | string | Nombre localizado del mapa cuando la invitación está abierta o estás dentro; si no, `typeName` |
| `instanceId` | integer | Número de instancia del campo de batalla que muestra el juego |
| `minLevel`, `maxLevel` | integer | Rango de niveles |
| `queueGuid` | array[2] | Identificador `[high, low]` de la cola que se devuelve al servidor |
| `instanceGuid` | array[2] | Identificador `[high, low]` de la instancia sin procesar |
| `estimatedWaitMs` | integer | Espera estimada en milisegundos |
| `timeWaitedMs` | integer | Tiempo en la cola en milisegundos |
| `portExpiresInMs` | integer | Milisegundos que quedan para entrar mientras `status` es `"confirm"` |
| `canLeave` | boolean | Indica si la invitación permite abandonar la cola; las invitaciones de arena no se pueden rechazar |

### Ejemplo

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "battlefield.getQueues"}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "queues": [
      {
        "index": 1,
        "status": "confirm",
        "statusId": 2,
        "typeId": 32,
        "typeName": "Campo de batalla aleatorio",
        "random": true,
        "arena": false,
        "rated": false,
        "teamSize": 0,
        "mapId": 489,
        "mapName": "Garganta Grito de Guerra",
        "instanceId": 12,
        "minLevel": 80,
        "maxLevel": 80,
        "queueGuid": [503316480, 32],
        "instanceGuid": [0, 131072],
        "estimatedWaitMs": 120000,
        "timeWaitedMs": 95000,
        "portExpiresInMs": 78000,
        "canLeave": true
      },
      {
        "index": 2,
        "status": "none",
        "statusId": 0,
        "typeId": 0,
        "typeName": "",
        "random": false,
        "arena": false,
        "rated": false,
        "teamSize": 0,
        "mapId": 0,
        "mapName": "",
        "instanceId": 0,
        "minLevel": 0,
        "maxLevel": 0,
        "queueGuid": [0, 0],
        "instanceGuid": [0, 0],
        "estimatedWaitMs": 0,
        "timeWaitedMs": 0,
        "portExpiresInMs": 0,
        "canLeave": true
      }
    ]
  }
}
```

## battlefield.acceptPort

Entra en el campo de batalla o la arena igual que el botón **Entrar en batalla** (`AcceptBattlefieldPort(index, 1)`, `CMSG_BATTLEFIELD_PORT`).

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `index` | integer | Sí | Hueco de cola, `1` o `2` |

### Respuesta

```json
{"ok": true}
```

### Errores

| Condición | Respuesta |
| --- | --- |
| `index` no es `1` ni `2` | `{"error": "index must be 1 or 2"}` |
| El jugador no está en el mundo | `{"error": "not in world"}` |
| La cola no tiene una invitación abierta | `{"error": "queue is not in the required status"}` |

## battlefield.leaveQueue

Abandona una cola, o rechaza su invitación, igual que el botón **Abandonar cola** (`AcceptBattlefieldPort(index, nil)`).

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `index` | integer | Sí | Hueco de cola, `1` o `2` |

### Respuesta

```json
{"ok": true}
```

### Errores

| Condición | Respuesta |
| --- | --- |
| `index` no es `1` ni `2` | `{"error": "index must be 1 or 2"}` |
| El jugador no está en el mundo | `{"error": "not in world"}` |
| La cola no está en cola ni esperando respuesta | `{"error": "queue is not in the required status"}` |
| La invitación es de una partida de arena | `{"error": "this queue cannot be left from the invitation"}` |

!!! note
    El SDK construye `CMSG_BATTLEFIELD_PORT` con las funciones de paquetes del propio cliente porque `AcceptBattlefieldPort` lee sus argumentos desde Lua. Las comprobaciones del botón en el cliente (por ejemplo, estar en combate) quedan en manos del servidor.
