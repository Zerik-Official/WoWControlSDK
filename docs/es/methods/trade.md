# Métodos trade.*

Leen las solicitudes de comercio y la ventana de comercio, y las responden de forma nativa con las mismas funciones que los botones del juego. La actividad de comercio también se envía como [eventos del SDK](events.md#cargas-utiles-de-los-eventos-del-sdk): `trade.requested`, `trade.requestClosed`, `trade.opened`, `trade.updated` y `trade.closed`.

## trade.get

Devuelve la solicitud de comercio pendiente y la ventana de comercio abierta con los dos lados.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `open` | boolean | Indica si la ventana de comercio está abierta. Si es `false` se omiten todos los campos posteriores a `request` |
| `request` | object | `pending`, `requesterName` y `receivedAtMs` de una solicitud que espera respuesta |
| `traderGuid` | array[2] | GUID `[high, low]` del otro jugador |
| `traderName` | string | Nombre del otro jugador |
| `playerAccepted` | boolean | Indica si pulsaste **Comerciar** |
| `targetAccepted` | boolean | Indica si el otro jugador pulsó **Comerciar** |
| `playerMoney` | integer | Cobre que ofreces (`GetPlayerTradeMoney`) |
| `targetMoney` | integer | Cobre que ofrece el otro jugador (`GetTargetTradeMoney`) |
| `playerItems` | array | Tus siete huecos |
| `targetItems` | array | Los siete huecos del otro jugador |

**Campos de cada objeto** (`GetTradePlayerItemInfo`, `GetTradeTargetItemInfo` y sus enlaces de objeto):

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `slot` | integer | Hueco de `1` a `7` |
| `exists` | boolean | Indica si el hueco tiene un objeto. Si es `false` solo están `slot` y `willNotBeTraded` |
| `willNotBeTraded` | boolean | `true` en el hueco `7`, el de "No se comerciará" que sirve para encantar el objeto del otro jugador |
| `guid` | array[2] | GUID `[high, low]` del objeto; `[0, 0]` en los objetos del otro jugador |
| `itemId` | integer | ID del objeto |
| `cached` | boolean | Indica si el objeto está en la caché de objetos del cliente. Si es `false`, `name` y `link` están vacíos y `quality` vale `-1` |
| `name` | string | Nombre localizado del objeto |
| `link` | string | Enlace del objeto tal como lo devuelven `GetTradePlayerItemLink` y `GetTradeTargetItemLink` |
| `quality` | integer | De `0` pobre a `7` reliquia, `-1` si no está en caché |
| `count` | integer | Tamaño de la pila |
| `enchantId` | integer | ID del encantamiento permanente |
| `gemIds` | array[3] | IDs de las gemas engarzadas |
| `suffixId` | integer | ID de la propiedad aleatoria o sufijo |
| `uniqueId` | integer | ID único del enlace del objeto |

### Ejemplo

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "trade.get"}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "open": true,
    "request": {"pending": false, "requesterName": "", "receivedAtMs": 0},
    "traderGuid": [0, 4321],
    "traderName": "Jaina",
    "playerAccepted": false,
    "targetAccepted": true,
    "playerMoney": 0,
    "targetMoney": 150000,
    "playerItems": [
      {"slot": 1, "exists": false, "willNotBeTraded": false}
    ],
    "targetItems": [
      {
        "slot": 1,
        "exists": true,
        "willNotBeTraded": false,
        "guid": [0, 0],
        "itemId": 33470,
        "cached": true,
        "name": "Paño de tejido de escarcha",
        "link": "|cffffffff|Hitem:33470:0:0:0:0:0:0:0:80|h[Paño de tejido de escarcha]|h|r",
        "quality": 1,
        "count": 20,
        "enchantId": 0,
        "gemIds": [0, 0, 0],
        "suffixId": 0,
        "uniqueId": 0
      }
    ]
  }
}
```

!!! note
    El ejemplo acorta las dos listas de objetos a un hueco. La respuesta siempre contiene los siete huecos de cada lado.

## trade.acceptRequest

Acepta una solicitud de comercio igual que el botón **Sí** del diálogo de solicitud (`BeginTrade`) y cierra el diálogo. La ventana de comercio se abre cuando el servidor lo confirma, y entonces también se envía `trade.opened`.

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
| No hay ninguna solicitud pendiente | `{"error": "no pending trade request"}` |

## trade.declineRequest

Rechaza una solicitud de comercio igual que el botón **No** del diálogo de solicitud (`CancelTrade`) y cierra el diálogo.

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
| No hay ninguna solicitud pendiente | `{"error": "no pending trade request"}` |

## trade.accept

Pulsa **Comerciar** en la ventana de comercio (`AcceptTrade`). El comercio se completa cuando los dos jugadores han aceptado. Cambiar un objeto o el dinero en cualquiera de los lados borra las dos aceptaciones, igual que en el juego.

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
| La ventana de comercio no está abierta | `{"error": "trade window is not open"}` |

## trade.unaccept

Retira tu aceptación (`CancelTradeAccept`).

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
| La ventana de comercio no está abierta | `{"error": "trade window is not open"}` |

## trade.cancel

Cierra la ventana de comercio sin comerciar, igual que el botón **Cancelar** (`CancelTrade`).

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
| La ventana de comercio no está abierta | `{"error": "trade window is not open"}` |
