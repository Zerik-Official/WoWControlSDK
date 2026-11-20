# Métodos duel.*

Leen una solicitud de duelo y la aceptan o rechazan de forma nativa. Los duelos también se envían como [eventos del SDK](events.md#cargas-utiles-de-los-eventos-del-sdk): `duel.requested`, `duel.accepted`, `duel.closed` y `duel.boundsChanged`.

## duel.get

Devuelve la solicitud de duelo o el duelo actual.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `pending` | boolean | Hay una solicitud de duelo esperando respuesta |
| `active` | boolean | Aceptaste un duelo que todavía no ha terminado |
| `challengerName` | string | Nombre del jugador que te retó |
| `challengerGuid` | array[2] | GUID `[high, low]` de quien te retó |
| `arbiterGuid` | array[2] | GUID `[high, low]` de la bandera del duelo |
| `requestedAtMs` | integer | Momento de `GetTickCount64()` de la solicitud, `0` si no hay ninguna |

### Ejemplo

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "duel.get"}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "pending": true,
    "active": false,
    "challengerName": "Arthas",
    "challengerGuid": [0, 1234],
    "arbiterGuid": [4026597376, 81],
    "requestedAtMs": 81234567
  }
}
```

!!! note
    `challengerGuid` y `arbiterGuid` se leen del último duelo del cliente y conservan su valor cuando el duelo termina. Usa `pending` y `active` para saber si hay un duelo en curso.

## duel.accept

Acepta el duelo igual que el botón **Aceptar** del diálogo (`AcceptDuel`, `CMSG_DUEL_ACCEPTED`) y cierra el diálogo.

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
| No hay ninguna solicitud de duelo pendiente | `{"error": "no pending duel"}` |

## duel.decline

Rechaza la solicitud igual que el botón **Rechazar** del diálogo (`CancelDuel`, `CMSG_DUEL_CANCELLED`) y cierra el diálogo. Durante un duelo aceptado te rinde, igual que llamar a `CancelDuel` en el juego.

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
| No hay solicitud de duelo ni duelo activo | `{"error": "no pending duel"}` |
