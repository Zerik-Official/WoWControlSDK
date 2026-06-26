# Métodos world.*

Lee el estado actual del mundo, zona y mapa. Los datos vienen de un caché por frame — sin ida y vuelta al hilo del juego.

---

## `world.getState`

Devuelve la información actual del mundo y zona para el jugador.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `mapId` | integer | ID del mapa/instancia actual |
| `zoneId` | integer | ID de la zona actual |
| `continentId` | integer | ID del continente |
| `zoneName` | string | nombre de zona localizado |
| `subZoneName` | string | nombre de subzona localizado |
| `inWorld` | boolean | si el jugador está en el mundo del juego |
| `loading` | boolean | si el mundo aún está cargando |
| `valid` | boolean | si la entrada de caché está poblada |

### Ejemplo

```json
{"id": 1, "method": "world.getState"}
```

```json
{
  "id": 1,
  "result": {
    "mapId": 1,
    "zoneId": 1519,
    "continentId": 0,
    "zoneName": "Stormwind City",
    "subZoneName": "Dwarven District",
    "inWorld": true,
    "loading": false,
    "valid": true
  }
}
```

### Errores

| Condición | Error |
|---|---|
| No está en el mundo del juego | `{ "error": "world not available" }` |
