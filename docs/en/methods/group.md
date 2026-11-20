# group.* methods

Read the party or raid roster, a combat summary and pending group invitations, and answer invitations natively. The roster is read from the client's group tables on every call, so members that are far away or in another zone are included with the data the server keeps sending for them.

## group.get

Returns the full party or raid roster. The local player is not part of `members`.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `valid` | boolean | Whether a group exists |
| `type` | string | `"party"`, `"raid"` or `"none"` |
| `leaderGuid` | array[2] | `[high, low]` GUID of the group leader |
| `memberCount` | integer | Number of entries in `members` (every member except the player) |
| `size` | integer | Group size including the player: `GetNumPartyMembers() + 1` in a party, `GetNumRaidMembers()` in a raid |
| `inRangeCount` | integer | Members whose unit is loaded in the object manager (`inRange: true`) |
| `partyMemberCount` | integer | Same value as `GetNumPartyMembers()`: filled party slots, excluding the player |
| `raidMemberCount` | integer | Same value as `GetNumRaidMembers()`: raid members including the player, `0` outside a raid |
| `realPartyMemberCount` | integer | Same value as `GetRealNumPartyMembers()` (ignores battleground groups) |
| `realRaidMemberCount` | integer | Same value as `GetRealNumRaidMembers()` (ignores battleground raids) |
| `lootMethod` | string | `"freeforall"`, `"roundrobin"`, `"master"`, `"group"`, `"needbeforegreed"` or `"unknown"` |
| `lootMethodId` | integer | Raw loot method, `0` to `4` in the order above |
| `masterLooterGuid` | array[2] | `[high, low]` GUID of the master looter, `[0, 0]` when not used |
| `dungeonDifficulty` | integer | Dungeon difficulty selected by the group: `1` normal, `2` heroic |
| `raidDifficulty` | integer | Raid difficulty selected by the group: `1` 10 normal, `2` 25 normal, `3` 10 heroic, `4` 25 heroic |
| `playerSubgroup` | integer | Raid subgroup of the player, `1` to `8`; `0` outside a raid |
| `playerRank` | integer | Raid rank of the player: `0` member, `1` assistant, `2` leader; `0` outside a raid |
| `members` | array | Member objects |

**Member fields:**

| Field | Type | Description |
| --- | --- | --- |
| `guid` | array[2] | `[high, low]` member GUID |
| `inRange` | boolean | Whether the unit is loaded nearby. Live fields such as `targetGuid`, `inCombat` and `casting` are only meaningful when `true` |
| `nameKnown` | boolean | Whether the name is already in the client's name cache. When `false`, the SDK asks the server and the name appears on a later call |
| `name` | string | Member name |
| `level` | integer | Level |
| `race` | integer | Race ID |
| `gender` | integer | `0` male, `1` female |
| `class` | integer | Class ID |
| `className` | string | Localized class name |
| `classFileName` | string | Class token, for example `"DEATHKNIGHT"` |
| `online` | boolean | Whether the member is connected |
| `health` | integer | Current health (live when in range, otherwise the last value sent by the server) |
| `maxHealth` | integer | Maximum health |
| `power` | integer | Current power |
| `powerMax` | integer | Maximum power |
| `targetGuid` | array[2] | `[high, low]` target GUID, `[0, 0]` out of range |
| `inCombat` | boolean | In combat (in range only) |
| `dead` | boolean | Dead |
| `ghost` | boolean | Released as a ghost |
| `casting` | boolean | Casting (in range only) |
| `channeling` | boolean | Channeling (in range only) |
| `autoAttacking` | boolean | Auto-attacking (in range only) |
| `partyIndex` | integer | Party slot `1` to `4` (the `partyN` token), `-1` in a raid |
| `raidIndex` | integer | Raid slot `1` to `40` (the `raidN` token), `-1` in a party |
| `subgroup` | integer | Raid subgroup `1` to `8`, `0` in a party |
| `rank` | integer | Raid rank: `0` member, `1` assistant, `2` leader |
| `isLeader` | boolean | Whether the member leads the group |
| `isAssistant` | boolean | Raid assistant |
| `isMainTank` | boolean | Marked as main tank |
| `isMainAssist` | boolean | Marked as main assist |
| `isMasterLooter` | boolean | Master looter when the loot method is `"master"` |
| `lfgRoles` | object | Roles chosen in the Dungeon Finder, see [roles](#roles) |
| `stats` | object | Data the server sends for every group member, see below |

**`stats` fields** (the `SMSG_PARTY_MEMBER_STATS` data kept by the client):

| Field | Type | Description |
| --- | --- | --- |
| `available` | boolean | Whether the entry exists. The other fields are omitted when `false` |
| `statusFlags` | integer | Raw status flags: `0x01` online, `0x02` PvP, `0x04` dead, `0x08` ghost, `0x10` free-for-all PvP, `0x40` AFK, `0x80` DND |
| `online`, `pvp`, `dead`, `ghost`, `pvpFreeForAll`, `afk`, `dnd` | boolean | The status flags decoded |
| `powerType` | integer | `0` mana, `1` rage, `2` focus, `3` energy, `6` runic power |
| `health`, `maxHealth` | integer | Health |
| `power`, `maxPower` | integer | Power |
| `level` | integer | Level |
| `zoneId` | integer | `AreaTable.dbc` ID of the member's zone |
| `zoneName` | string | Localized zone name |
| `position` | object | `{"x", "y"}` world position rounded to integers |
| `auras` | array | `{"spellId", "flags"}` for each of up to 64 aura slots in use |
| `vehicleSeat` | integer | Vehicle seat ID, `0` outside a vehicle |
| `pet` | object or null | `guid`, `name`, `modelId`, `powerType`, `health`, `maxHealth`, `power`, `maxPower`, `auras` of the member's pet |

### Example

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "group.get"}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "valid": true,
    "type": "party",
    "leaderGuid": [0, 1234],
    "memberCount": 2,
    "size": 3,
    "inRangeCount": 1,
    "partyMemberCount": 2,
    "raidMemberCount": 0,
    "realPartyMemberCount": 2,
    "realRaidMemberCount": 0,
    "lootMethod": "group",
    "lootMethodId": 3,
    "masterLooterGuid": [0, 0],
    "dungeonDifficulty": 2,
    "raidDifficulty": 1,
    "playerSubgroup": 0,
    "playerRank": 0,
    "members": [
      {
        "guid": [0, 1234],
        "inRange": false,
        "nameKnown": true,
        "name": "Arthas",
        "level": 80,
        "race": 1,
        "gender": 0,
        "class": 6,
        "className": "Death Knight",
        "classFileName": "DEATHKNIGHT",
        "online": true,
        "health": 31000,
        "maxHealth": 32000,
        "power": 0,
        "powerMax": 1000,
        "targetGuid": [0, 0],
        "inCombat": false,
        "dead": false,
        "ghost": false,
        "casting": false,
        "channeling": false,
        "autoAttacking": false,
        "partyIndex": 1,
        "raidIndex": -1,
        "subgroup": 0,
        "rank": 0,
        "isLeader": true,
        "isAssistant": false,
        "isMainTank": false,
        "isMainAssist": false,
        "isMasterLooter": false,
        "lfgRoles": {"flags": 0, "tank": false, "healer": false, "damage": false},
        "stats": {
          "available": true,
          "statusFlags": 1,
          "online": true,
          "pvp": false,
          "dead": false,
          "ghost": false,
          "pvpFreeForAll": false,
          "afk": false,
          "dnd": false,
          "powerType": 6,
          "health": 31000,
          "maxHealth": 32000,
          "power": 0,
          "maxPower": 1000,
          "level": 80,
          "zoneId": 4395,
          "zoneName": "Dalaran",
          "position": {"x": 5804, "y": 624},
          "auras": [{"spellId": 48263, "flags": 9}],
          "vehicleSeat": 0,
          "pet": null
        }
      }
    ]
  }
}
```

!!! note
    When no group exists, the response is `{"valid": false}` and contains no other fields.

!!! note
    Since 2.1.0 `members` also contains members that are out of range, and `memberCount` counts them. Before 2.1.0 members that were not loaded nearby were left out. Use `size` for the size of the group shown in the game.

## group.combatSummary

Returns a combat summary across the group members that are in range.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `anyInCombat` | boolean | Whether at least one member is in combat |
| `anyDead` | boolean | Whether at least one member is dead |
| `anyCasting` | boolean | Whether at least one member is casting |
| `anyEngaging` | boolean | Whether at least one member is engaging |
| `countInCombat` | integer | Number of members in combat |
| `countDead` | integer | Number of dead members |

### Example

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "group.combatSummary"}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "anyInCombat": true,
    "anyDead": false,
    "anyCasting": true,
    "anyEngaging": false,
    "countInCombat": 3,
    "countDead": 0
  }
}
```

## group.getInvite

Returns the pending group invitation. The SDK records it when the game fires `PARTY_INVITE_REQUEST`; the inviter's name is not kept anywhere else in memory.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `pending` | boolean | Whether an invitation is waiting for an answer |
| `inviterName` | string | Name of the player who invited you, empty when none |
| `receivedAtMs` | integer | `GetTickCount64()` time when the invitation arrived, `0` when none |

### Example

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "group.getInvite"}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"pending": true, "inviterName": "Jaina", "receivedAtMs": 81234567}}
```

## group.acceptInvite

Accepts the pending invitation the same way as the dialog's **Accept** button (`AcceptGroup`): it sends `CMSG_GROUP_ACCEPT` with the selected roles and closes the in-game dialog. No Lua is executed.

### Parameters

| Name | Type | Required | Default | Description |
| --- | --- | --- | --- | --- |
| `tank` | boolean | No | `false` | Offer the tank role |
| `healer` | boolean | No | `false` | Offer the healer role |
| `damage` | boolean | No | `false` | Offer the damage role |

### Response

```json
{"ok": true}
```

### Example

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "group.acceptInvite", "params": {"damage": true}}
```

### Errors

| Condition | Response |
| --- | --- |
| The player is not in the world | `{"error": "not in world"}` |

## group.declineInvite

Declines the pending invitation like the dialog's **Decline** button (`DeclineGroup`, `CMSG_GROUP_DECLINE`) and closes the dialog.

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

!!! note
    The server ignores an answer when there is no invitation, so both methods are safe to call at any time. The [`group.inviteClosed`](events.md#sdk-event-payloads) event reports how the invitation ended, also when it was answered from the game.

## Roles

Role objects (`lfgRoles`, and `roles` in events) decode the role flags used by the group and Dungeon Finder packets:

| Field | Type | Description |
| --- | --- | --- |
| `flags` | integer | Raw flags: `0x02` tank, `0x04` healer, `0x08` damage |
| `tank` | boolean | Tank role |
| `healer` | boolean | Healer role |
| `damage` | boolean | Damage role |
