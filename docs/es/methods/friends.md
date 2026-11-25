# Métodos friends.*

Leen la lista de amigos que el cliente guarda en memoria. Los cambios de conexión también se envían como [eventos del SDK](events.md#cargas-utiles-de-los-eventos-del-sdk) (`friends.online`, `friends.offline`, `friends.statusChanged`).

## friends.list

Devuelve cada entrada de la lista de amigos con los mismos datos que `GetFriendInfo`.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `loaded` | boolean | Indica si el cliente ya recibió la lista de amigos. Es `false` antes de entrar al mundo |
| `count` | integer | Número de amigos |
| `onlineCount` | integer | Número de amigos conectados |
| `friends` | array | Objetos de amigo |

**Campos de cada amigo:**

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `index` | integer | Posición en la lista, el índice que usa `GetFriendInfo(index)` |
| `guid` | array[2] | GUID `[high, low]` del amigo |
| `name` | string | Nombre del personaje |
| `online` | boolean | Indica si el amigo está conectado |
| `afk` | boolean | Ausente (solo mientras está conectado) |
| `dnd` | boolean | No molestar (solo mientras está conectado) |
| `referAFriend` | boolean | Vinculado mediante Recluta a un amigo |
| `flags` | integer | Flags de estado sin procesar: `0x02` ausente, `0x04` no molestar, `0x08` Recluta a un amigo |
| `note` | string | Nota escrita en la lista de amigos |
| `level` | integer | Nivel, `0` si está desconectado |
| `classId` | integer | ID de clase, `0` si está desconectado |
| `className` | string | Nombre localizado de la clase |
| `classFileName` | string | Token de la clase, por ejemplo `"MAGE"` |
| `areaId` | integer | ID de `AreaTable.dbc` de la zona del amigo, `0` si está desconectado |
| `areaName` | string | Nombre localizado de la zona |
| `player` | objeto o null | Raza, sexo y clase del amigo según la caché de nombres; ver el [objeto de jugador](events.md#objeto-de-jugador) |

### Ejemplo

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "friends.list"}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "loaded": true,
    "count": 2,
    "onlineCount": 1,
    "friends": [
      {
        "index": 1,
        "guid": [0, 4321],
        "name": "Jaina",
        "online": true,
        "afk": false,
        "dnd": false,
        "referAFriend": false,
        "flags": 0,
        "note": "Raid on Thursdays",
        "level": 80,
        "classId": 8,
        "className": "Mage",
        "classFileName": "MAGE",
        "areaId": 4395,
        "areaName": "Dalaran"
      },
      {
        "index": 2,
        "guid": [0, 8765],
        "name": "Thrall",
        "online": false,
        "afk": false,
        "dnd": false,
        "referAFriend": false,
        "flags": 0,
        "note": "",
        "level": 0,
        "classId": 0,
        "className": "",
        "classFileName": "",
        "areaId": 0,
        "areaName": ""
      }
    ]
  }
}
```

!!! note
    La lista admite hasta 100 entradas. El servidor solo envía nivel, clase y zona de los amigos conectados.
