# object.* methods

Inspect game objects. These methods check whether an entity exists in the client's object manager and what type of WoW object it is.

All methods accept a `token` parameter:

| Name | Type | Required | Default | Description |
| --- | --- | --- | --- | --- |
| `token` | string | No | `"player"` | Unit token (`player`, `target`, `mouseover`, `focus`, `partyN`, `raidN`) or a GUID string |

## object.exists

Checks whether the entity exists in the object manager.

### Parameters

The common `token` parameter.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `exists` | boolean | Whether the object exists |

### Example

```json
{"jsonrpc": "2.0", "id": 1, "method": "object.exists", "params": {"token": "target"}}
```

## object.getType

Returns the WoW object type of the entity.

### Parameters

The common `token` parameter.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `exists` | boolean | Whether the object exists |
| `type` | string | `object`, `item`, `container`, `unit`, `player`, `gameobject`, `dynamicobject`, `corpse` or `unknown` |
| `typeId` | integer | Raw `ObjectType` enum value |

!!! note
    When the object does not exist, the response is `{"exists": false, "type": "none", "typeId": 0}`.

## object.isUnit

Checks whether the entity is a WoW unit, such as an NPC, pet or player.

### Parameters

The common `token` parameter.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `isUnit` | boolean | Whether the object is a unit |

## object.isPlayer

Checks whether the entity is a WoW player.

### Parameters

The common `token` parameter.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `isPlayer` | boolean | Whether the object is a player |

### Example

```json
{"jsonrpc": "2.0", "id": 1, "method": "object.isPlayer", "params": {"token": "player"}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"isPlayer": true}}
```
