# Métodos object.*

Inspeccionan objetos del juego. Estos métodos comprueban si una entidad existe en el gestor de objetos del cliente y de qué tipo de objeto de WoW se trata.

Todos los métodos aceptan un parámetro `token`:

| Nombre | Tipo | Obligatorio | Por defecto | Descripción |
| --- | --- | --- | --- | --- |
| `token` | string | No | `"player"` | Token de unidad (`player`, `target`, `mouseover`, `focus`, `partyN`, `raidN`) o una cadena GUID |

## object.exists

Comprueba si la entidad existe en el gestor de objetos.

### Parámetros

El parámetro común `token`.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `exists` | boolean | Indica si el objeto existe |

### Ejemplo

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "object.exists", "params": {"token": "target"}}
```

## object.getType

Devuelve el tipo de objeto de WoW de la entidad.

### Parámetros

El parámetro común `token`.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `exists` | boolean | Indica si el objeto existe |
| `type` | string | `object`, `item`, `container`, `unit`, `player`, `gameobject`, `dynamicobject`, `corpse` o `unknown` |
| `typeId` | integer | Valor bruto del enum `ObjectType` |

!!! note
    Cuando el objeto no existe, la respuesta es `{"exists": false, "type": "none", "typeId": 0}`.

## object.isUnit

Comprueba si la entidad es una unidad de WoW, como un PNJ, una mascota o un jugador.

### Parámetros

El parámetro común `token`.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `isUnit` | boolean | Indica si el objeto es una unidad |

## object.isPlayer

Comprueba si la entidad es un jugador de WoW.

### Parámetros

El parámetro común `token`.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `isPlayer` | boolean | Indica si el objeto es un jugador |

### Ejemplo

```json
{"jsonrpc": "2.0", "auth": "<session token>", "id": 1, "method": "object.isPlayer", "params": {"token": "player"}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"isPlayer": true}}
```
