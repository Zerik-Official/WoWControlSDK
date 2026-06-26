# Métodos group.*

Información de grupo y raid con caché basada en checksum para rendimiento. Los datos de grupo se reconstruyen solo cuando cambia la composición del grupo.

---

## `group.get`

Devuelve información completa del grupo o raid incluyendo todos los miembros.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `valid` | boolean | si existe un grupo |
| `type` | string | `"party"`, `"raid"` o `"none"` |
| `leaderGuid` | array[2] | `[high, low]` GUID del líder |
| `memberCount` | integer | |
| `members` | array | objetos de miembro |

**Campos de miembro:**

| Campo | Tipo | Descripción |
|---|---|---|
| `name` | string | |
| `level` | integer | |
| `race` | integer | |
| `class` | integer | |
| `health` | integer | |
| `maxHealth` | integer | |
| `power` | integer | |
| `powerMax` | integer | |
| `targetGuid` | array[2] | `[high, low]` |
| `inCombat` | boolean | |
| `dead` | boolean | |
| `ghost` | boolean | |
| `casting` | boolean | |
| `channeling` | boolean | |
| `autoAttacking` | boolean | |

### Ejemplo

```json
{"id": 1, "method": "group.get"}
```

```json
{
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

> [!NOTE]
> Si no existe grupo, devuelve `{ "valid": false }` sin campos adicionales.

---

## `group.combatSummary`

Resumen rápido del estado de combate entre todos los miembros del grupo. Útil para visión general de raid.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `anyInCombat` | boolean | |
| `anyDead` | boolean | |
| `anyCasting` | boolean | |
| `anyEngaging` | boolean | |
| `countInCombat` | integer | |
| `countDead` | integer | |

### Ejemplo

```json
{"id": 1, "method": "group.combatSummary"}
```

```json
{
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
