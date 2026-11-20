# lfg.* methods

Read the Dungeon Finder proposal (the "Your group is ready" dialog) and enter or leave it natively. Proposal changes are pushed as [SDK events](events.md#sdk-event-payloads): `lfg.proposalShown`, `lfg.proposalUpdated`, `lfg.proposalFailed` and `lfg.proposalSucceeded`.

## lfg.getProposal

Returns the active proposal with everything `GetLFGProposal`, `GetLFGProposalMember` and `GetLFGProposalEncounter` expose.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `exists` | boolean | Whether a proposal is active. The other fields are omitted when `false` |
| `proposalId` | integer | Proposal ID sent back to the server when answering |
| `dungeonId` | integer | `LfgDungeons.dbc` ID |
| `proposalType` | integer | Dungeon type the server encodes with the dungeon ID (`typeID` of `GetLFGProposal`) |
| `state` | integer | Raw proposal state |
| `dungeon` | object | Dungeon row, see [dungeon fields](#dungeon-fields) |
| `role` | string | Role assigned to you: `"TANK"`, `"HEALER"`, `"DAMAGER"` or `"UNKNOWN"` |
| `isLeader` | boolean | Whether you are the group leader in the proposal |
| `responded` | boolean | Whether you already answered |
| `accepted` | boolean | Whether you accepted |
| `bossesKilledMask` | integer | Bit mask of the encounters already defeated in the instance you would join |
| `totalEncounters` | integer | Encounters in the dungeon (`0` for random dungeons, as in the game) |
| `completedEncounters` | integer | Encounters already defeated |
| `members` | array | The five proposal slots, see below |
| `encounters` | array | Encounters of the dungeon, see below |

**Member fields:**

| Field | Type | Description |
| --- | --- | --- |
| `index` | integer | Slot `1` to `5` |
| `isPlayer` | boolean | Whether the slot is yours |
| `isLeader` | boolean | Group leader |
| `role` | string | `"TANK"`, `"HEALER"`, `"DAMAGER"` or `"UNKNOWN"` |
| `roles` | object | Role flags, see [roles](group.md#roles) |
| `responded` | boolean | Whether the member answered |
| `accepted` | boolean | Whether the member accepted |

**Encounter fields** (`DungeonEncounter.dbc`):

| Field | Type | Description |
| --- | --- | --- |
| `id` | integer | Encounter ID |
| `name` | string | Localized boss name |
| `order` | integer | Order in the dungeon |
| `bit` | integer | Bit of the encounter in `bossesKilledMask` |
| `iconTexture` | string | Icon path from `SpellIcon.dbc`, empty when none |
| `completed` | boolean | Whether it was already defeated |

### Example

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "lfg.getProposal"}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "exists": true,
    "proposalId": 1187,
    "dungeonId": 262,
    "proposalType": 1,
    "state": 0,
    "dungeon": {
      "id": 262,
      "name": "Utgarde Pinnacle",
      "description": "",
      "typeId": 1,
      "random": false,
      "holiday": false,
      "flags": 0,
      "mapId": 575,
      "difficulty": 1,
      "minLevel": 80,
      "maxLevel": 80,
      "targetLevel": 80,
      "targetLevelMin": 80,
      "targetLevelMax": 80,
      "faction": -1,
      "texture": "UtgardePinnacle",
      "expansion": 2,
      "order": 0,
      "groupId": 0
    },
    "role": "DAMAGER",
    "isLeader": false,
    "responded": false,
    "accepted": false,
    "bossesKilledMask": 1,
    "totalEncounters": 4,
    "completedEncounters": 1,
    "members": [
      {"index": 1, "isPlayer": true, "isLeader": false, "role": "DAMAGER", "roles": {"flags": 8, "tank": false, "healer": false, "damage": true}, "responded": false, "accepted": false}
    ],
    "encounters": [
      {"id": 581, "name": "Svala Sorrowgrave", "order": 0, "bit": 0, "iconTexture": "", "completed": true}
    ]
  }
}
```

## lfg.getDungeon

Returns one row of `LfgDungeons.dbc`.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `id` | integer | Yes | `LfgDungeons.dbc` ID |

### Response

The [dungeon fields](#dungeon-fields).

### Errors

| Condition | Response |
| --- | --- |
| `id` is missing or not an integer | `{"error": "id must be an integer"}` |
| The ID does not exist | `{"error": "dungeon not found"}` |

## lfg.acceptProposal

Enters the dungeon like the dialog's **Enter Dungeon** button (`AcceptProposal`, `CMSG_LFG_PROPOSAL_RESULT` with `true`). The game updates its own dialog.

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
| No proposal is active | `{"error": "no pending proposal"}` |

## lfg.rejectProposal

Leaves the queue like the dialog's **Leave Queue** button (`RejectProposal`, `CMSG_LFG_PROPOSAL_RESULT` with `false`).

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
| No proposal is active | `{"error": "no pending proposal"}` |

!!! note
    The game limits proposal answers to three every ten seconds. Extra answers inside that window are dropped by the client, exactly as with the buttons.

## Dungeon fields

| Field | Type | Description |
| --- | --- | --- |
| `id` | integer | `LfgDungeons.dbc` ID |
| `name` | string | Localized name |
| `description` | string | Localized description |
| `typeId` | integer | `1` dungeon, `2` raid, `4` zone, `5` heroic, `6` random |
| `random` | boolean | `typeId` is random |
| `holiday` | boolean | Seasonal holiday dungeon |
| `flags` | integer | Raw `LfgDungeons` flags |
| `mapId` | integer | `Map.dbc` ID |
| `difficulty` | integer | `0` normal, `1` heroic |
| `minLevel`, `maxLevel` | integer | Allowed level range |
| `targetLevel`, `targetLevelMin`, `targetLevelMax` | integer | Recommended levels |
| `faction` | integer | `-1` both, `0` Horde, `1` Alliance |
| `texture` | string | Texture name used by the Dungeon Finder |
| `expansion` | integer | `0` classic, `1` The Burning Crusade, `2` Wrath of the Lich King |
| `order` | integer | Sort order |
| `groupId` | integer | Dungeon group in the Dungeon Finder list |
