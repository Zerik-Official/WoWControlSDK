# Métodos group.*

Leen la lista del grupo o la banda, un resumen de combate y las invitaciones de grupo pendientes, y responden a las invitaciones de forma nativa. La lista se lee de las tablas de grupo del cliente en cada llamada, así que incluye a los miembros lejanos o en otra zona con los datos que el servidor sigue enviando de ellos.

## group.get

Devuelve la lista completa del grupo o la banda. El jugador local no forma parte de `members`.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `valid` | boolean | Indica si existe un grupo |
| `type` | string | `"party"`, `"raid"` o `"none"` |
| `leaderGuid` | array[2] | GUID `[high, low]` del líder del grupo |
| `memberCount` | integer | Número de entradas de `members` (todos los miembros menos el jugador) |
| `size` | integer | Tamaño del grupo contando al jugador: `GetNumPartyMembers() + 1` en grupo, `GetNumRaidMembers()` en banda |
| `inRangeCount` | integer | Miembros cuya unidad está cargada en el object manager (`inRange: true`) |
| `partyMemberCount` | integer | Mismo valor que `GetNumPartyMembers()`: huecos de grupo ocupados, sin contar al jugador |
| `raidMemberCount` | integer | Mismo valor que `GetNumRaidMembers()`: miembros de la banda contando al jugador, `0` fuera de una banda |
| `realPartyMemberCount` | integer | Mismo valor que `GetRealNumPartyMembers()` (ignora los grupos de campo de batalla) |
| `realRaidMemberCount` | integer | Mismo valor que `GetRealNumRaidMembers()` (ignora las bandas de campo de batalla) |
| `lootMethod` | string | `"freeforall"`, `"roundrobin"`, `"master"`, `"group"`, `"needbeforegreed"` o `"unknown"` |
| `lootMethodId` | integer | Método de botín sin procesar, de `0` a `4` en el orden anterior |
| `masterLooterGuid` | array[2] | GUID `[high, low]` del maestro despojador, `[0, 0]` si no se usa |
| `dungeonDifficulty` | integer | Dificultad de mazmorra elegida por el grupo: `1` normal, `2` heroica |
| `raidDifficulty` | integer | Dificultad de banda elegida por el grupo: `1` 10 normal, `2` 25 normal, `3` 10 heroica, `4` 25 heroica |
| `playerSubgroup` | integer | Subgrupo de banda del jugador, de `1` a `8`; `0` fuera de una banda |
| `playerRank` | integer | Rango de banda del jugador: `0` miembro, `1` ayudante, `2` líder; `0` fuera de una banda |
| `members` | array | Objetos de miembro |

**Campos de cada miembro:**

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `guid` | array[2] | GUID `[high, low]` del miembro |
| `inRange` | boolean | Indica si la unidad está cargada cerca. Los campos en vivo como `targetGuid`, `inCombat` y `casting` solo tienen sentido cuando es `true` |
| `nameKnown` | boolean | Indica si el nombre ya está en la caché de nombres del cliente. Si es `false`, el SDK lo pide al servidor y el nombre aparece en una llamada posterior |
| `name` | string | Nombre del miembro |
| `level` | integer | Nivel |
| `race` | integer | ID de raza |
| `gender` | integer | `0` masculino, `1` femenino |
| `class` | integer | ID de clase |
| `className` | string | Nombre localizado de la clase |
| `classFileName` | string | Token de la clase, por ejemplo `"DEATHKNIGHT"` |
| `online` | boolean | Indica si el miembro está conectado |
| `health` | integer | Salud actual (en vivo si está cerca, si no el último valor enviado por el servidor) |
| `maxHealth` | integer | Salud máxima |
| `power` | integer | Poder actual |
| `powerMax` | integer | Poder máximo |
| `targetGuid` | array[2] | GUID `[high, low]` del objetivo, `[0, 0]` fuera de alcance |
| `inCombat` | boolean | En combate (solo cerca) |
| `dead` | boolean | Muerto |
| `ghost` | boolean | Liberado como fantasma |
| `casting` | boolean | Lanzando un hechizo (solo cerca) |
| `channeling` | boolean | Canalizando (solo cerca) |
| `autoAttacking` | boolean | Atacando automáticamente (solo cerca) |
| `partyIndex` | integer | Hueco de grupo de `1` a `4` (el token `partyN`), `-1` en banda |
| `raidIndex` | integer | Hueco de banda de `1` a `40` (el token `raidN`), `-1` en grupo |
| `subgroup` | integer | Subgrupo de banda de `1` a `8`, `0` en grupo |
| `rank` | integer | Rango de banda: `0` miembro, `1` ayudante, `2` líder |
| `isLeader` | boolean | Indica si el miembro lidera el grupo |
| `isAssistant` | boolean | Ayudante de banda |
| `isMainTank` | boolean | Marcado como tanque principal |
| `isMainAssist` | boolean | Marcado como asistente principal |
| `isMasterLooter` | boolean | Maestro despojador cuando el método de botín es `"master"` |
| `lfgRoles` | object | Roles elegidos en el Buscador de mazmorras, ver [roles](#roles) |
| `stats` | object | Datos que el servidor envía de cada miembro del grupo, ver abajo |

**Campos de `stats`** (los datos de `SMSG_PARTY_MEMBER_STATS` que guarda el cliente):

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `available` | boolean | Indica si la entrada existe. Si es `false` se omiten los demás campos |
| `statusFlags` | integer | Flags de estado sin procesar: `0x01` conectado, `0x02` JcJ, `0x04` muerto, `0x08` fantasma, `0x10` JcJ libre, `0x40` ausente, `0x80` no molestar |
| `online`, `pvp`, `dead`, `ghost`, `pvpFreeForAll`, `afk`, `dnd` | boolean | Los flags de estado decodificados |
| `powerType` | integer | `0` maná, `1` ira, `2` enfoque, `3` energía, `6` poder rúnico |
| `health`, `maxHealth` | integer | Salud |
| `power`, `maxPower` | integer | Poder |
| `level` | integer | Nivel |
| `zoneId` | integer | ID de `AreaTable.dbc` de la zona del miembro |
| `zoneName` | string | Nombre localizado de la zona |
| `position` | object | Posición `{"x", "y"}` en el mundo redondeada a enteros |
| `auras` | array | `{"spellId", "flags"}` por cada uno de los hasta 64 huecos de aura usados |
| `vehicleSeat` | integer | ID del asiento de vehículo, `0` fuera de un vehículo |
| `pet` | object o null | `guid`, `name`, `modelId`, `powerType`, `health`, `maxHealth`, `power`, `maxPower` y `auras` de la mascota del miembro |

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "group.get"}
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
    Si no existe un grupo, la respuesta es `{"valid": false}` y no contiene otros campos.

!!! note
    Desde la 2.1.0 `members` también incluye a los miembros fuera de alcance y `memberCount` los cuenta. Antes de la 2.1.0 se omitían los miembros que no estaban cargados cerca. Usa `size` para el tamaño del grupo que muestra el juego.

## group.combatSummary

Devuelve un resumen de combate de los miembros del grupo que están cerca.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `anyInCombat` | boolean | Indica si al menos un miembro está en combate |
| `anyDead` | boolean | Indica si al menos un miembro está muerto |
| `anyCasting` | boolean | Indica si al menos un miembro está lanzando un hechizo |
| `anyEngaging` | boolean | Indica si al menos un miembro está entrando en combate |
| `countInCombat` | integer | Número de miembros en combate |
| `countDead` | integer | Número de miembros muertos |

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "group.combatSummary"}
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

Devuelve la invitación de grupo pendiente. El SDK la registra cuando el juego dispara `PARTY_INVITE_REQUEST`; el nombre de quien invita no se guarda en ningún otro sitio de la memoria.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `pending` | boolean | Indica si hay una invitación esperando respuesta |
| `inviterName` | string | Nombre del jugador que te invitó, vacío si no hay ninguna |
| `receivedAtMs` | integer | Momento de `GetTickCount64()` en que llegó la invitación, `0` si no hay ninguna |

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "group.getInvite"}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"pending": true, "inviterName": "Jaina", "receivedAtMs": 81234567}}
```

## group.acceptInvite

Acepta la invitación pendiente igual que el botón **Aceptar** del diálogo (`AcceptGroup`): envía `CMSG_GROUP_ACCEPT` con los roles elegidos y cierra el diálogo del juego. No ejecuta Lua.

### Parámetros

| Nombre | Tipo | Obligatorio | Por defecto | Descripción |
| --- | --- | --- | --- | --- |
| `tank` | boolean | No | `false` | Ofrecer el rol de tanque |
| `healer` | boolean | No | `false` | Ofrecer el rol de sanador |
| `damage` | boolean | No | `false` | Ofrecer el rol de daño |

### Respuesta

```json
{"ok": true}
```

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "group.acceptInvite", "params": {"damage": true}}
```

### Errores

| Condición | Respuesta |
| --- | --- |
| El jugador no está en el mundo | `{"error": "not in world"}` |

## group.declineInvite

Rechaza la invitación pendiente igual que el botón **Rechazar** del diálogo (`DeclineGroup`, `CMSG_GROUP_DECLINE`) y cierra el diálogo.

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

!!! note
    El servidor ignora la respuesta si no hay invitación, así que ambos métodos se pueden llamar en cualquier momento. El evento [`group.inviteClosed`](events.md#cargas-utiles-de-los-eventos-del-sdk) indica cómo terminó la invitación, también si se respondió desde el juego.

## Roles

Los objetos de roles (`lfgRoles`, y `roles` en los eventos) decodifican los flags de rol que usan los paquetes de grupo y del Buscador de mazmorras:

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `flags` | integer | Flags sin procesar: `0x02` tanque, `0x04` sanador, `0x08` daño |
| `tank` | boolean | Rol de tanque |
| `healer` | boolean | Rol de sanador |
| `damage` | boolean | Rol de daño |
