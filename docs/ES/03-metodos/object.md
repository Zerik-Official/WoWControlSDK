# Métodos object.*

Introspección de tipos de objetos del juego. Verifica si una entidad existe y qué tipo de objeto de WoW es.

Todos los métodos aceptan un parámetro `token` para identificar la entidad objetivo:

| Nombre | Tipo | Requerido | Por defecto | Descripción |
|---|---|---|---|---|
| `token` | string | no | `"player"` | token de unidad (`"player"`, `"target"`, `"mouseover"`, `"focus"`, `"partyN"`, `"raidN"` o un string GUID) |

---

## `object.exists`

Verifica si un objeto del juego existe en el object manager del cliente.

### Parámetros

`token` (opcional)

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `exists` | boolean | si el objeto existe |

### Ejemplo

```json
{"id": 1, "method": "object.exists", "params": {"token": "target"}}
```

---

## `object.getType`

Devuelve el tipo de objeto de WoW para la entidad identificada por el token.

### Parámetros

`token` (opcional)

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `exists` | boolean | si el objeto existe |
| `type` | string | uno de: `"object"`, `"item"`, `"container"`, `"unit"`, `"player"`, `"gameobject"`, `"dynamicobject"`, `"corpse"`, `"unknown"` |
| `typeId` | integer | valor del enum ObjectType |

> [!NOTE]
> Si el objeto no existe, devuelve `{ "exists": false, "type": "none", "typeId": 0 }`.

---

## `object.isUnit`

Verifica si la entidad es una Unidad de WoW (NPC, mascota o jugador).

### Parámetros

`token` (opcional)

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `isUnit` | boolean | |

---

## `object.isPlayer`

Verifica si la entidad es un Jugador de WoW.

### Parámetros

`token` (opcional)

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `isPlayer` | boolean | |

---

### Ejemplo

```json
{"id": 1, "method": "object.isPlayer", "params": {"token": "player"}}
```

```json
{"id": 1, "result": {"isPlayer": true}}
```
