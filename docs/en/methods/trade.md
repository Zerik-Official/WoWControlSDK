# trade.* methods

Read trade requests and the trade window, and answer them natively with the same functions as the game's buttons. Trade activity is also pushed as [SDK events](events.md#sdk-event-payloads): `trade.requested`, `trade.requestClosed`, `trade.opened`, `trade.updated` and `trade.closed`.

## trade.get

Returns the pending trade request and the open trade window with both sides.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `open` | boolean | Whether the trade window is open. Every field after `request` is omitted when `false` |
| `request` | object | `pending`, `requesterName` and `receivedAtMs` of a request waiting for an answer |
| `traderGuid` | array[2] | `[high, low]` GUID of the other player |
| `traderName` | string | Name of the other player |
| `playerAccepted` | boolean | Whether you pressed **Trade** |
| `targetAccepted` | boolean | Whether the other player pressed **Trade** |
| `playerMoney` | integer | Copper you offer (`GetPlayerTradeMoney`) |
| `targetMoney` | integer | Copper the other player offers (`GetTargetTradeMoney`) |
| `playerItems` | array | Your seven slots |
| `targetItems` | array | The other player's seven slots |

**Item fields** (`GetTradePlayerItemInfo`, `GetTradeTargetItemInfo` and their item links):

| Field | Type | Description |
| --- | --- | --- |
| `slot` | integer | Slot `1` to `7` |
| `exists` | boolean | Whether the slot holds an item. Only `slot` and `willNotBeTraded` are present when `false` |
| `willNotBeTraded` | boolean | `true` for slot `7`, the "Will not be traded" slot used to enchant the other player's item |
| `guid` | array[2] | `[high, low]` item GUID; `[0, 0]` for the other player's items |
| `itemId` | integer | Item ID |
| `cached` | boolean | Whether the item is in the client's item cache. When `false`, `name` and `link` are empty and `quality` is `-1` |
| `name` | string | Localized item name |
| `link` | string | Item link as returned by `GetTradePlayerItemLink` and `GetTradeTargetItemLink` |
| `quality` | integer | `0` poor to `7` heirloom, `-1` when not cached |
| `count` | integer | Stack size |
| `enchantId` | integer | Permanent enchant ID |
| `gemIds` | array[3] | Gem item IDs in the sockets |
| `suffixId` | integer | Random property or suffix ID |
| `uniqueId` | integer | Unique ID from the item link |

### Example

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
        "name": "Frostweave Cloth",
        "link": "|cffffffff|Hitem:33470:0:0:0:0:0:0:0:80|h[Frostweave Cloth]|h|r",
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
    The example shortens both item lists to one slot. The response always contains the seven slots of each side.

## trade.acceptRequest

Accepts a trade request like the **Yes** button of the request dialog (`BeginTrade`) and closes the dialog. The trade window opens when the server confirms, which also sends `trade.opened`.

### Parameters

None.

### Response

```json
{"ok": true}
```

### Errors

| Condition | Response |
| --- | --- |
| The player is not in the world | `{"error": "not in world"}` |
| No request is pending | `{"error": "no pending trade request"}` |

## trade.declineRequest

Declines a trade request like the **No** button of the request dialog (`CancelTrade`) and closes the dialog.

### Parameters

None.

### Response

```json
{"ok": true}
```

### Errors

| Condition | Response |
| --- | --- |
| The player is not in the world | `{"error": "not in world"}` |
| No request is pending | `{"error": "no pending trade request"}` |

## trade.accept

Presses **Trade** in the trade window (`AcceptTrade`). The trade completes when both players have accepted. Changing an item or the money on either side clears both accept states, as in the game.

### Parameters

None.

### Response

```json
{"ok": true}
```

### Errors

| Condition | Response |
| --- | --- |
| The player is not in the world | `{"error": "not in world"}` |
| The trade window is not open | `{"error": "trade window is not open"}` |

## trade.unaccept

Withdraws your acceptance (`CancelTradeAccept`).

### Parameters

None.

### Response

```json
{"ok": true}
```

### Errors

| Condition | Response |
| --- | --- |
| The player is not in the world | `{"error": "not in world"}` |
| The trade window is not open | `{"error": "trade window is not open"}` |

## trade.cancel

Closes the trade window without trading, like the **Cancel** button (`CancelTrade`).

### Parameters

None.

### Response

```json
{"ok": true}
```

### Errors

| Condition | Response |
| --- | --- |
| The player is not in the world | `{"error": "not in world"}` |
| The trade window is not open | `{"error": "trade window is not open"}` |
