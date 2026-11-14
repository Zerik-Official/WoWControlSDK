# Métodos world.*

Leen el mundo, la zona y el estado de carga actuales. Los datos vienen de la caché por frame, así que estas llamadas no bloquean el hilo del juego.

## world.getState

Devuelve la información de mundo y zona del jugador.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `mapId` | integer | ID del mapa o de la instancia actual |
| `zoneId` | integer | ID de la zona actual |
| `continentId` | integer | ID del mapa del continente |
| `zoneName` | string | Nombre localizado de la zona |
| `subZoneName` | string | Nombre localizado de la subzona |
| `inWorld` | boolean | Indica si el jugador está en el mundo del juego |
| `loading` | boolean | Indica si el mundo todavía se está cargando |
| `valid` | boolean | Indica si la entrada de caché está rellena |

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "world.getState"}
```

```json
{
  "jsonrpc": "2.0",
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

| Condición | Respuesta |
| --- | --- |
| El jugador no está en el mundo del juego | `{"error": "world not available"}` |
