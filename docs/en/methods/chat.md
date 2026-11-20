# chat.* methods

Send chat messages on any channel and reply to whispers. Messages are sent with the same `CMSG_MESSAGECHAT` packet the game builds for `SendChatMessage`, and Battle.net whispers go through the client's own `BNSendWhisper` functions. No Lua is executed.

Incoming whispers are pushed as the [`chat.whisperReceived`](events.md#sdk-event-payloads) event.

## chat.send

Sends a message on a channel.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `channel` | string | Yes | `SAY`, `YELL`, `EMOTE`, `PARTY`, `RAID`, `RAID_WARNING`, `BATTLEGROUND`, `GUILD`, `OFFICER`, `WHISPER`, `CHANNEL` or `BN_WHISPER`. Case does not matter |
| `target` | string | For `WHISPER`, `CHANNEL` and `BN_WHISPER` | Character name for `WHISPER`; channel number (`"1"`) or name (`"General - Dalaran"`) for `CHANNEL`; Battle.net presence ID for `BN_WHISPER` |
| `text` | string | Yes | Message, up to 255 bytes once encoded |
| `language` | integer | No | `Languages.dbc` ID. Defaults to the character's default language, as in the game |

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | Always `true` |
| `channel` | string | Channel used, in upper case |
| `target` | string | Target used; for `CHANNEL`, the resolved channel name |

### Example

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "chat.send", "params": {"channel": "WHISPER", "target": "Jaina", "text": "On my way!"}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "channel": "WHISPER", "target": "Jaina"}}
```

### Errors

| Condition | Response |
| --- | --- |
| `channel` is missing | `{"error": "channel is required"}` |
| The channel name is not in the list | `{"error": "unknown channel"}` |
| `WHISPER`, `CHANNEL` or `BN_WHISPER` without `target` | `{"error": "target is required for this channel"}` |
| `text` is empty | `{"error": "text is empty"}` |
| `text` is longer than 255 bytes | `{"error": "text is longer than 255 bytes"}` |
| `PARTY` or `RAID_WARNING` outside a group | `{"error": "not in a group"}` |
| `RAID` or `BATTLEGROUND` outside a raid | `{"error": "not in a raid"}` |
| The channel number is not joined | `{"error": "channel not joined"}` |
| `SAY`, `YELL` or `EMOTE` while the game blocks speaking | `{"error": "cannot speak now"}` |
| Battle.net is not connected (always on private servers) | `{"error": "battle.net is not available"}` |
| The Battle.net friend is offline | `{"error": "battle.net friend is not online"}` |
| The player is not in the world | `{"error": "not in world"}` |

!!! note
    As in the game, `PARTY`, `RAID` and `BATTLEGROUND` messages from the group leader are sent as their leader variants. A `|` in the text is sent literally, so links and color codes cannot be injected. The server still applies its own rules: muted players, ignored players and spam limits.

## chat.reply

Replies to a whisper, like pressing **R** in the game. Without parameters it answers the last whisper received; it picks `WHISPER` or `BN_WHISPER` from that whisper.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `text` | string | Yes | Message |
| `lineId` | integer | No | Reply to the whisper with this `lineId` instead of the last one |
| `guid` | string | No | Reply to the last whisper from this sender GUID |

### Response

Same as [`chat.send`](#chatsend).

### Example

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "chat.reply", "params": {"text": "Give me 5 minutes"}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "channel": "WHISPER", "target": "Jaina"}}
```

### Errors

The errors of [`chat.send`](#chatsend), plus:

| Condition | Response |
| --- | --- |
| No whisper was received, or none matches `lineId` or `guid` | `{"error": "no whisper to reply to"}` |

!!! note
    The SDK keeps its own list of the last 20 whispers instead of using the game's reply target, so a reply always goes to the whisper you chose, even if the chat frame changed its target.

## chat.getRecentWhispers

Returns the whispers `chat.reply` can answer, oldest first.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `whispers` | array | Up to 20 whisper objects |

**Whisper fields:**

| Field | Type | Description |
| --- | --- | --- |
| `author` | string | Sender name |
| `battleNet` | boolean | Battle.net whisper |
| `presenceId` | integer | Battle.net presence ID, `0` for normal whispers |
| `lineId` | integer | Chat line ID |
| `guid` | string | Sender GUID as sent by the game |
| `receivedAtMs` | integer | `GetTickCount64()` time when it arrived |

### Example

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "chat.getRecentWhispers"}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "whispers": [
      {"author": "Jaina", "battleNet": false, "presenceId": 0, "lineId": 412, "guid": "0x0000000000001234", "receivedAtMs": 81234567}
    ]
  }
}
```
