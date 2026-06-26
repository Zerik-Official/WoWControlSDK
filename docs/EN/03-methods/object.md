# object.* Methods

Game object type introspection. Check whether an entity exists and what type of WoW object it is.

All methods accept a `token` parameter to identify the target entity:

| Name | Type | Required | Default | Description |
|---|---|---|---|---|
| `token` | string | no | `"player"` | unit token (`"player"`, `"target"`, `"mouseover"`, `"focus"`, `"partyN"`, `"raidN"`, or a GUID string) |

---

## `object.exists`

Check whether a game object exists in the client's object manager.

### Parameters

`token` (optional)

### Response

| Field | Type | Description |
|---|---|---|
| `exists` | boolean | whether the object exists |

### Example

```json
{"id": 1, "method": "object.exists", "params": {"token": "target"}}
```

---

## `object.getType`

Returns the WoW object type for the entity identified by the token.

### Parameters

`token` (optional)

### Response

| Field | Type | Description |
|---|---|---|
| `exists` | boolean | whether the object exists |
| `type` | string | one of: `"object"`, `"item"`, `"container"`, `"unit"`, `"player"`, `"gameobject"`, `"dynamicobject"`, `"corpse"`, `"unknown"` |
| `typeId` | integer | raw ObjectType enum value |

> [!NOTE]
> If the object does not exist, returns `{ "exists": false, "type": "none", "typeId": 0 }`.

---

## `object.isUnit`

Check whether the entity is a WoW Unit (NPC, pet, or player).

### Parameters

`token` (optional)

### Response

| Field | Type | Description |
|---|---|---|
| `isUnit` | boolean | |

---

## `object.isPlayer`

Check whether the entity is a WoW Player.

### Parameters

`token` (optional)

### Response

| Field | Type | Description |
|---|---|---|
| `isPlayer` | boolean | |

---

### Example

```json
{"id": 1, "method": "object.isPlayer", "params": {"token": "player"}}
```

```json
{"id": 1, "result": {"isPlayer": true}}
```
