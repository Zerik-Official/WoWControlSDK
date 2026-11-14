# Métodos group.*

Leen información del grupo y de la raid. Los datos de grupo se basan en checksums y solo se reconstruyen cuando cambia la composición, por lo que las llamadas repetidas son baratas.

## group.get

Devuelve la lista completa del grupo o de la raid.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `valid` | boolean | Indica si existe un grupo |
| `type` | string | `"party"`, `"raid"` o `"none"` |
| `leaderGuid` | array[2] | GUID del líder del grupo `[high, low]` |
| `memberCount` | integer | Número de miembros |
| `members` | array | Objetos de miembro |

**Campos de miembro:**

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `name` | string | Nombre del miembro |
| `level` | integer | Nivel del miembro |
| `race` | integer | ID de raza |
| `class` | integer | ID de clase |
| `health` | integer | Salud actual |
| `maxHealth` | integer | Salud máxima |
| `power` | integer | Poder actual |
| `powerMax` | integer | Poder máximo |
| `targetGuid` | array[2] | GUID del objetivo `[high, low]` |
| `inCombat` | boolean | Indica si el miembro está en combate |
| `dead` | boolean | Indica si el miembro está muerto |
| `ghost` | boolean | Indica si el miembro es un fantasma |
| `casting` | boolean | Indica si el miembro está lanzando un hechizo |
| `channeling` | boolean | Indica si el miembro está canalizando |
| `autoAttacking` | boolean | Indica si el miembro está atacando automáticamente |

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
    "leaderGuid": [1234, 5678],
    "memberCount": 2,
    "members": [
      {
        "name": "PlayerName",
        "level": 80,
        "race": 1,
        "class": 2,
        "health": 25000,
        "maxHealth": 25000,
        "power": 100,
        "powerMax": 100,
        "targetGuid": [0, 0],
        "inCombat": false,
        "dead": false,
        "ghost": false,
        "casting": false,
        "channeling": false,
        "autoAttacking": false
      }
    ]
  }
}
```

!!! note
    Cuando no existe ningún grupo, la respuesta es `{"valid": false}` y no contiene más campos.

## group.combatSummary

Devuelve un resumen de combate de todos los miembros del grupo.

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
