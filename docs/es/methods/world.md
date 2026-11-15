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
| `mapAreaId` | integer | ID de `WorldMapArea.dbc` del mapa actual, el valor de `GetCurrentMapAreaID()` |
| `instance` | object | Información de instancia y dificultad, ver abajo |
| `inWorld` | boolean | Indica si el jugador está en el mundo del juego |
| `loading` | boolean | Indica si el mundo todavía se está cargando |
| `valid` | boolean | Indica si la entrada de caché está rellena |

**Campos de `instance`** (los datos detrás de `GetInstanceInfo`, `GetInstanceDifficulty`, `GetDungeonDifficulty` y `GetRaidDifficulty`):

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `inInstance` | boolean | Indica si el mapa actual es una instancia (`type` no es `"none"`) |
| `mapId` | integer | ID de `Map.dbc` que usa la información de instancia |
| `mapName` | string | Nombre localizado del mapa |
| `mapDirectory` | string | Directorio interno del mapa, por ejemplo `"Northrend"` |
| `type` | string | `"none"`, `"party"`, `"raid"`, `"pvp"` (campo de batalla) o `"arena"` |
| `typeId` | integer | Tipo de instancia sin procesar, de `0` a `4` en el orden anterior |
| `dynamic` | boolean | Indica si la dificultad de la banda se puede cambiar dentro (Ciudadela de la Corona de Hielo y similares) |
| `difficulty` | integer | Dificultad actual, empezando en `1`: mazmorras `1` normal y `2` heroica; bandas `1` 10 normal, `2` 25 normal, `3` 10 heroica y `4` 25 heroica |
| `difficultyIndex` | integer | `difficulty - 1`, el valor sin procesar que usa `MapDifficulty.dbc` |
| `difficultyKey` | string | Clave de texto global de la dificultad, por ejemplo `"RAID_DIFFICULTY_10PLAYER"`; vacía fuera de instancias |
| `mapDifficultyId` | integer | ID de `MapDifficulty.dbc`, `0` si el mapa no tiene |
| `maxPlayers` | integer | Número máximo de jugadores, `0` fuera de instancias |
| `resetSeconds` | integer | Duración del bloqueo en segundos, `0` si la instancia no tiene |
| `requirementMessage` | string | Texto de requisito de `MapDifficulty.dbc`, normalmente vacío |
| `playerDifficulty` | integer | Valor `playerDifficulty` sin procesar de `GetInstanceInfo` |
| `dungeonDifficulty` | integer | Dificultad de mazmorra elegida fuera de la instancia, `1` o `2` |
| `raidDifficulty` | integer | Dificultad de banda elegida fuera de la instancia, de `1` a `4` |

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
    "mapAreaId": 302,
    "instance": {
      "inInstance": false,
      "mapId": 0,
      "mapName": "Eastern Kingdoms",
      "mapDirectory": "Azeroth",
      "type": "none",
      "typeId": 0,
      "dynamic": false,
      "difficulty": 1,
      "difficultyIndex": 0,
      "difficultyKey": "",
      "mapDifficultyId": 0,
      "maxPlayers": 0,
      "resetSeconds": 0,
      "requirementMessage": "",
      "playerDifficulty": 0,
      "dungeonDifficulty": 1,
      "raidDifficulty": 1
    },
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
