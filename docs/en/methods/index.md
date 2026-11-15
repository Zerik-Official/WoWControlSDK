# Methods

Every RPC method belongs to a domain. Each domain has its own page with the parameters, response fields, examples and edge cases of each method.

## Conventions

- **Method names** use `domain.action`, for example `unit.get` or `window.setBounds`.
- **Parameters** are passed as a JSON object in `params`. Every parameter is optional unless the table marks it as required.
- **Responses** are JSON objects. Unless a page says otherwise, fields are always present, even when their value is `null`.
- **Unit tokens** accept `"player"`, `"target"`, `"focus"`, `"mouseover"`, `"partyN"`, `"raidN"` or a GUID string. The default token is `"player"`.
- **GUIDs** are returned as `[high, low]` arrays of two 32-bit unsigned integers.
- **Positions** are objects with `x`, `y` and `z` as floating-point values. World positions are in game coordinates; map positions are normalized.
- **Errors** are usually returned inside `result` as `{"error": "<message>"}`. A successful JSON-RPC envelope does not guarantee that the operation succeeded.
- **Cached methods** (`player.*` and `world.*`) read the per-frame cache and return immediately. The other methods run on the game thread.
- **Native actions** (`group.acceptInvite`, `summon.accept`, `lfg.acceptProposal`, `trade.accept`, `duel.accept`, `battlefield.acceptPort` and their counterparts) call the same client functions as the game's buttons. They never run Lua.

## Domains

| Domain | Methods | Description |
| --- | --- | --- |
| [lua.*](lua.md) | 5 | Execute, evaluate and manage Lua globals in WoW's Lua VM |
| [unit.*](unit.md) | 4 | Read health, power, position, rotation and state flags from a unit |
| [group.*](group.md) | 5 | Read the party or raid roster, a combat summary and group invitations, and answer invitations |
| [player.*](player.md) | 4 | Read cached player health, position, target and full state |
| [world.*](world.md) | 1 | Read the current map, zone, instance, difficulty and loading state |
| [friends.*](friends.md) | 1 | Read the friend list |
| [summon.*](summon.md) | 3 | Read, accept and decline summons |
| [lfg.*](lfg.md) | 4 | Read and answer Dungeon Finder proposals |
| [trade.*](trade.md) | 6 | Read and answer trade requests and the trade window |
| [duel.*](duel.md) | 3 | Read, accept and decline duels |
| [battlefield.*](battlefield.md) | 3 | Read battleground and arena queues and answer their invitations |
| [object.*](object.md) | 4 | Inspect whether an entity exists and what type it is |
| [client.*](client.md) | 17 | Login, realm selection, character list, screen state and EULA/TOS automation |
| [events.*](events.md) | 8 | Subscribe to WoW events and manage the event history |
| [dll.*](dll.md) | 4 | Control the console overlay and read SDK diagnostics and logs |
| [window.*](window.md) | 18 | Inspect and control the main WoW window |

## Full method list

| Domain | Methods |
| --- | --- |
| `lua` | `execute`, `evaluate`, `getGlobal`, `setGlobal`, `createNamespace` |
| `unit` | `get`, `state`, `position`, `mapPosition` |
| `group` | `get`, `combatSummary`, `getInvite`, `acceptInvite`, `declineInvite` |
| `player` | `getHealth`, `getPosition`, `getTarget`, `getState` |
| `world` | `getState` |
| `friends` | `list` |
| `summon` | `get`, `accept`, `decline` |
| `lfg` | `getProposal`, `getDungeon`, `acceptProposal`, `rejectProposal` |
| `trade` | `get`, `acceptRequest`, `declineRequest`, `accept`, `unaccept`, `cancel` |
| `duel` | `get`, `accept`, `decline` |
| `battlefield` | `getQueues`, `acceptPort`, `leaveQueue` |
| `object` | `exists`, `getType`, `isUnit`, `isPlayer` |
| `client` | `ping`, `getScreen`, `getDebugState`, `login`, `enterWorld`, `logout`, `quit`, `getCharacters`, `refreshCharacters`, `getRealmlist`, `setRealmlist`, `acceptEULA`, `acceptTOS`, `acceptTermination`, `acceptScanning`, `acceptContest`, `acceptAll` |
| `events` | `subscribe`, `unsubscribe`, `list`, `clear`, `getHistory`, `clearHistory`, `setMaxBuffer`, `isReady` |
| `dll` | `toggleConsole`, `getInfo`, `getLogs`, `setLogConfig` |
| `window` | `getHandle`, `getInfo`, `show`, `hide`, `minimize`, `restore`, `maximize`, `focus`, `getPosition`, `setPosition`, `getSize`, `setSize`, `setBounds`, `setTitle`, `setTopMost`, `setOpacity`, `flashTaskbar`, `setIcon` |

## Anatomy of a method page

Each method is documented with the same sections:

| Section | Content |
| --- | --- |
| Description | What the method does and when to use it |
| Parameters | Name, type, required flag, default and meaning |
| Response | Field, type and meaning |
| Example | A request and, when useful, a response |
| Notes | Screen requirements, caching, limits or edge cases |

## Request example

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "unit.get",
  "params": {
    "token": "player"
  }
}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "exists": true,
    "name": "PlayerName",
    "health": 25000,
    "maxHealth": 25000,
    "position": {
      "x": 1234.5,
      "y": 2345.6,
      "z": 300.1
    }
  }
}
```
