# Métodos

Cada método RPC pertenece a un dominio. Cada dominio tiene su propia página con los parámetros, los campos de respuesta, ejemplos y casos límite de cada método.

## Convenciones

- **Nombres de método** con el formato `dominio.acción`, por ejemplo `unit.get` o `window.setBounds`.
- **Parámetros** en un objeto JSON dentro de `params`. Todos son opcionales salvo que la tabla indique lo contrario.
- **Respuestas** como objetos JSON. Salvo que una página indique otra cosa, los campos siempre están presentes, incluso con valor `null`.
- **Tokens de unidad** que aceptan `"player"`, `"target"`, `"focus"`, `"mouseover"`, `"partyN"`, `"raidN"` o una cadena GUID. El token por defecto es `"player"`.
- **GUIDs** devueltos como arreglos `[high, low]` de dos enteros sin signo de 32 bits.
- **Posiciones** como objetos con `x`, `y` y `z` en coma flotante. Las posiciones del mundo usan coordenadas del juego; las de mapa están normalizadas.
- **Errores** normalmente dentro de `result` como `{"error": "<mensaje>"}`. Un sobre JSON-RPC correcto no garantiza que la operación haya funcionado.
- **Métodos cacheados** (`player.*`, `world.*` y `group.*`) que leen la caché por frame o por checksum y responden de inmediato. Los métodos no cacheados llaman al hilo del juego.

## Dominios

| Dominio | Métodos | Descripción |
| --- | --- | --- |
| [lua.*](lua.md) | 5 | Ejecutar, evaluar y gestionar globales en la Lua VM de WoW |
| [unit.*](unit.md) | 4 | Leer salud, poder, posición, rotación y flags de estado de una unidad |
| [group.*](group.md) | 2 | Leer la lista del grupo o raid y un resumen de combate |
| [player.*](player.md) | 4 | Leer salud, posición, objetivo y estado completo cacheados del jugador |
| [world.*](world.md) | 1 | Leer el mapa, la zona y el estado de carga actuales |
| [object.*](object.md) | 4 | Inspeccionar si una entidad existe y de qué tipo es |
| [client.*](client.md) | 17 | Login, selección de reino, lista de personajes, estado de pantalla y automatización de EULA/TOS |
| [events.*](events.md) | 8 | Suscribirse a eventos de WoW y gestionar el historial |
| [dll.*](dll.md) | 4 | Controlar la consola y leer diagnóstico y logs del SDK |
| [window.*](window.md) | 18 | Inspeccionar y controlar la ventana principal de WoW |

## Lista completa de métodos

| Dominio | Métodos |
| --- | --- |
| `lua` | `execute`, `evaluate`, `getGlobal`, `setGlobal`, `createNamespace` |
| `unit` | `get`, `state`, `position`, `mapPosition` |
| `group` | `get`, `combatSummary` |
| `player` | `getHealth`, `getPosition`, `getTarget`, `getState` |
| `world` | `getState` |
| `object` | `exists`, `getType`, `isUnit`, `isPlayer` |
| `client` | `ping`, `getScreen`, `getDebugState`, `login`, `enterWorld`, `logout`, `quit`, `getCharacters`, `refreshCharacters`, `getRealmlist`, `setRealmlist`, `acceptEULA`, `acceptTOS`, `acceptTermination`, `acceptScanning`, `acceptContest`, `acceptAll` |
| `events` | `subscribe`, `unsubscribe`, `list`, `clear`, `getHistory`, `clearHistory`, `setMaxBuffer`, `isReady` |
| `dll` | `toggleConsole`, `getInfo`, `getLogs`, `setLogConfig` |
| `window` | `getHandle`, `getInfo`, `show`, `hide`, `minimize`, `restore`, `maximize`, `focus`, `getPosition`, `setPosition`, `getSize`, `setSize`, `setBounds`, `setTitle`, `setTopMost`, `setOpacity`, `flashTaskbar`, `setIcon` |

## Anatomía de una página de método

Cada método se documenta con las mismas secciones:

| Sección | Contenido |
| --- | --- |
| Descripción | Qué hace el método y cuándo usarlo |
| Parámetros | Nombre, tipo, obligatoriedad, valor por defecto y significado |
| Respuesta | Campo, tipo y significado |
| Ejemplo | Una petición y, cuando aporta, una respuesta |
| Notas | Requisitos de pantalla, caché, límites o casos límite |

## Ejemplo de petición

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
