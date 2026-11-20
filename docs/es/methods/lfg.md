# Métodos lfg.*

Leen la propuesta del Buscador de mazmorras (el diálogo "Tu grupo está listo") y permiten entrar o salir de ella de forma nativa. Los cambios de la propuesta se envían como [eventos del SDK](events.md#cargas-utiles-de-los-eventos-del-sdk): `lfg.proposalShown`, `lfg.proposalUpdated`, `lfg.proposalFailed` y `lfg.proposalSucceeded`.

## lfg.getProposal

Devuelve la propuesta activa con todo lo que exponen `GetLFGProposal`, `GetLFGProposalMember` y `GetLFGProposalEncounter`.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `exists` | boolean | Indica si hay una propuesta activa. Si es `false` se omiten los demás campos |
| `proposalId` | integer | ID de la propuesta que se devuelve al servidor al responder |
| `dungeonId` | integer | ID de `LfgDungeons.dbc` |
| `proposalType` | integer | Tipo de mazmorra que el servidor codifica junto al ID (`typeID` de `GetLFGProposal`) |
| `state` | integer | Estado de la propuesta sin procesar |
| `dungeon` | object | Fila de la mazmorra, ver [campos de la mazmorra](#campos-de-la-mazmorra) |
| `role` | string | Rol asignado: `"TANK"`, `"HEALER"`, `"DAMAGER"` o `"UNKNOWN"` |
| `isLeader` | boolean | Indica si eres el líder del grupo en la propuesta |
| `responded` | boolean | Indica si ya respondiste |
| `accepted` | boolean | Indica si aceptaste |
| `bossesKilledMask` | integer | Máscara de bits de los encuentros ya derrotados en la instancia a la que entrarías |
| `totalEncounters` | integer | Encuentros de la mazmorra (`0` en mazmorras aleatorias, como en el juego) |
| `completedEncounters` | integer | Encuentros ya derrotados |
| `members` | array | Los cinco huecos de la propuesta, ver abajo |
| `encounters` | array | Encuentros de la mazmorra, ver abajo |

**Campos de cada miembro:**

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `index` | integer | Hueco de `1` a `5` |
| `isPlayer` | boolean | Indica si el hueco es el tuyo |
| `isLeader` | boolean | Líder del grupo |
| `role` | string | `"TANK"`, `"HEALER"`, `"DAMAGER"` o `"UNKNOWN"` |
| `roles` | object | Flags de rol, ver [roles](group.md#roles) |
| `responded` | boolean | Indica si el miembro respondió |
| `accepted` | boolean | Indica si el miembro aceptó |

**Campos de cada encuentro** (`DungeonEncounter.dbc`):

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `id` | integer | ID del encuentro |
| `name` | string | Nombre localizado del jefe |
| `order` | integer | Orden dentro de la mazmorra |
| `bit` | integer | Bit del encuentro en `bossesKilledMask` |
| `iconTexture` | string | Ruta del icono de `SpellIcon.dbc`, vacía si no tiene |
| `completed` | boolean | Indica si ya fue derrotado |

### Ejemplo

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

Devuelve una fila de `LfgDungeons.dbc`.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `id` | integer | Sí | ID de `LfgDungeons.dbc` |

### Respuesta

Los [campos de la mazmorra](#campos-de-la-mazmorra).

### Errores

| Condición | Respuesta |
| --- | --- |
| Falta `id` o no es un entero | `{"error": "id must be an integer"}` |
| El ID no existe | `{"error": "dungeon not found"}` |

## lfg.acceptProposal

Entra en la mazmorra igual que el botón **Entrar en la mazmorra** del diálogo (`AcceptProposal`, `CMSG_LFG_PROPOSAL_RESULT` con `true`). El juego actualiza su propio diálogo.

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
| No hay propuesta activa | `{"error": "no pending proposal"}` |

## lfg.rejectProposal

Sale de la cola igual que el botón **Abandonar cola** del diálogo (`RejectProposal`, `CMSG_LFG_PROPOSAL_RESULT` con `false`).

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
| No hay propuesta activa | `{"error": "no pending proposal"}` |

!!! note
    El juego limita las respuestas a tres cada diez segundos. El cliente descarta las respuestas de más dentro de esa ventana, igual que con los botones.

## Campos de la mazmorra

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `id` | integer | ID de `LfgDungeons.dbc` |
| `name` | string | Nombre localizado |
| `description` | string | Descripción localizada |
| `typeId` | integer | `1` mazmorra, `2` banda, `4` zona, `5` heroica, `6` aleatoria |
| `random` | boolean | `typeId` es aleatoria |
| `holiday` | boolean | Mazmorra de evento de temporada |
| `flags` | integer | Flags de `LfgDungeons` sin procesar |
| `mapId` | integer | ID de `Map.dbc` |
| `difficulty` | integer | `0` normal, `1` heroica |
| `minLevel`, `maxLevel` | integer | Rango de niveles permitido |
| `targetLevel`, `targetLevelMin`, `targetLevelMax` | integer | Niveles recomendados |
| `faction` | integer | `-1` ambas, `0` Horda, `1` Alianza |
| `texture` | string | Nombre de la textura que usa el Buscador de mazmorras |
| `expansion` | integer | `0` clásico, `1` The Burning Crusade, `2` Wrath of the Lich King |
| `order` | integer | Orden de la lista |
| `groupId` | integer | Grupo de mazmorras en la lista del Buscador |
