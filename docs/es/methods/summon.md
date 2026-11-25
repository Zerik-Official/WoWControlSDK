# Métodos summon.*

Leen una invocación pendiente (por ejemplo de una piedra de encuentro o del Ritual de invocación de un brujo) y la aceptan o rechazan de forma nativa. El evento [`summon.requested`](events.md#cargas-utiles-de-los-eventos-del-sdk) avisa de una invocación nueva.

## summon.get

Devuelve la invocación pendiente, los mismos datos que muestra el diálogo `CONFIRM_SUMMON` del juego.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `pending` | boolean | Indica si hay una invocación esperando y sin caducar |
| `summonerGuid` | array[2] | GUID `[high, low]` de quien invoca |
| `summonerName` | string | Nombre de quien invoca (`GetSummonConfirmSummoner`) |
| `areaId` | integer | ID de `AreaTable.dbc` del destino |
| `areaName` | string | Destino localizado (`GetSummonConfirmAreaName`) |
| `timeLeftMs` | integer | Milisegundos hasta que caduque (`GetSummonConfirmTimeLeft`), `0` si caducó |
| `player` | objeto o null | Raza, sexo y clase de quien invoca; ver el [objeto de jugador](events.md#objeto-de-jugador) |
| `canTeleport` | boolean | Indica si el juego permitiría aceptar ahora (fuera de combate, sin aturdimiento, miedo ni otros bloqueos) |

### Ejemplo

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "summon.get"}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "pending": true,
    "summonerGuid": [0, 1234],
    "summonerName": "Arthas",
    "areaId": 4395,
    "areaName": "Dalaran",
    "timeLeftMs": 108000,
    "canTeleport": true
  }
}
```

## summon.accept

Acepta la invocación igual que el botón **Aceptar** del diálogo (`ConfirmSummon`, `CMSG_SUMMON_RESPONSE` con `true`) y cierra el diálogo.

### Parámetros

Ninguno.

### Respuesta

```json
{"ok": true}
```

### Errores

| Condición | Respuesta |
| --- | --- |
| El jugador no está en el mundo | `{"error": "not in world"}` |
| No hay invocación o caducó | `{"error": "no pending summon"}` |
| El jugador está en combate o no puede teletransportarse | `{"error": "cannot teleport now"}` (la invocación sigue pendiente) |

## summon.decline

Rechaza la invocación igual que el botón **Cancelar** del diálogo (`CancelSummon`, `CMSG_SUMMON_RESPONSE` con `false`) y cierra el diálogo.

### Parámetros

Ninguno.

### Respuesta

```json
{"ok": true}
```

### Errores

| Condición | Respuesta |
| --- | --- |
| El jugador no está en el mundo | `{"error": "not in world"}` |
| No hay invocación o caducó | `{"error": "no pending summon"}` |
