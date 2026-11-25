# friends.* methods

Read the friend list that the client keeps in memory. Connection changes are also pushed as [SDK events](events.md#sdk-event-payloads) (`friends.online`, `friends.offline`, `friends.statusChanged`).

## friends.list

Returns every entry of the friend list with the same data as `GetFriendInfo`.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `loaded` | boolean | Whether the client has received the friend list. `false` before entering the world |
| `count` | integer | Number of friends |
| `onlineCount` | integer | Number of friends online |
| `friends` | array | Friend objects |

**Friend fields:**

| Field | Type | Description |
| --- | --- | --- |
| `index` | integer | Slot in the list, the index used by `GetFriendInfo(index)` |
| `guid` | array[2] | `[high, low]` GUID of the friend |
| `name` | string | Character name |
| `online` | boolean | Whether the friend is connected |
| `afk` | boolean | Away (only while online) |
| `dnd` | boolean | Do not disturb (only while online) |
| `referAFriend` | boolean | Linked through Recruit-a-Friend |
| `flags` | integer | Raw status flags: `0x02` AFK, `0x04` DND, `0x08` Recruit-a-Friend |
| `note` | string | Note written in the friend list |
| `level` | integer | Level, `0` when offline |
| `classId` | integer | Class ID, `0` when offline |
| `className` | string | Localized class name |
| `classFileName` | string | Class token, for example `"MAGE"` |
| `areaId` | integer | `AreaTable.dbc` ID of the friend's zone, `0` when offline |
| `areaName` | string | Localized zone name |
| `player` | object or null | Race, gender and class of the friend from the name cache; see the [Player object](events.md#player-object) |

### Example

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
    The list holds up to 100 entries. The server sends level, class and zone only for friends that are online.
