# Arquitectura

## Diseño en Capas

WoWControlSDK sigue una arquitectura estricta por capas. Cada capa tiene responsabilidades definidas y **no debe** cruzar límites hacia los dominios de otras capas.

```
┌─────────────────────────────────────────────────────┐
│                     injector/                        │
│          (EXE independiente, lanzador de proceso)    │
└──────────────────────┬──────────────────────────────┘
                       │ Inyección DLL
┌──────────────────────▼──────────────────────────────┐
│                       ipc/                           │
│           (transporte named pipe, sin acceso a juego) │
└──────────────────────┬──────────────────────────────┘
                       │ JSON-RPC 2.0
┌──────────────────────▼──────────────────────────────┐
│                       rpc/                           │
│           (protocolo, dispatcher, handlers)           │
│          Sin acceso a memoria del juego — llama core/ │
└──────────────────────┬──────────────────────────────┘
                       │ Llamadas a core/ API
┌──────────────────────▼──────────────────────────────┐
│                    runtime/                           │
│      (consola, logging, event bus, state cache)       │
│          Sin acceso a memoria del juego — llama core/ │
└──────────────────────┬──────────────────────────────┘
                       │ Llamadas a core/ API
┌──────────────────────▼──────────────────────────────┐
│                     hooks/                            │
│    (detours en funciones del juego, captura eventos)  │
│          Sin direcciones hardcodeadas — usa offsets/  │
└──────────────────────┬──────────────────────────────┘
                       │ llama a core/ + datos de hooks
┌──────────────────────▼──────────────────────────────┐
│                      core/                            │
│   ÚNICA capa autorizada a leer/escribir memoria juego│
│   (wrappers nativos, API layer, offsets, resolvers)   │
└──────────────────────┬──────────────────────────────┘
                       │ lee/escribe
┌──────────────────────▼──────────────────────────────┐
│              World of Warcraft 3.3.5a                 │
│              (memoria del proceso, Lua VM)             │
└─────────────────────────────────────────────────────┘
```

### Responsabilidades de Capas

| Capa | Rol | Acceso a Memoria |
|---|---|---|
| `injector/` | EXE independiente, lanza WoW e inyecta DLL | Ninguno |
| `ipc/` | Transporte named pipe (leer/escribir JSON) | Ninguno |
| `rpc/` | Protocolo JSON-RPC 2.0, handlers, serialización | Ninguno |
| `runtime/` | State cache, event bus, consola, logging, realm list | Ninguno (llama a `core/`) |
| `hooks/` | Detours en funciones del juego, captura de eventos | Solo via offsets |
| `core/` | Wrappers nativos, API layer, lecturas/escrituras memoria | Sí |
| `client/` | Wrappers finos sobre `core/` y Lua del juego | Direcciones mínimas |
| `offsets/` | Todas las direcciones hardcodeadas como `constexpr uintptr_t` | Solo constantes |

## Flujo de Datos

### Petición RPC (lectura)

```
Pipe RPC → ipc/ → rpc/dispatcher → rpc/handler → core/API → memoria juego
                                              ↘ runtime/cache
```

1. Llega petición JSON-RPC al named pipe
2. `ipc/` lee y parsea el JSON crudo
3. `rpc/` dispatcher enruta al handler correspondiente
4. Handler llama a funciones de `core/` API (o lee de `runtime/` cache)
5. `core/` lee memoria del juego a través de wrappers nativos
6. Respuesta fluye de vuelta: `handler → dispatcher → ipc/ → pipe`

### Push de Evento (escritura / notificación)

```
señal juego → hooks/ → EventBus → EventPipe → pipe de eventos
                              ↘ StateCache (sondeo por frame)
```

1. El juego dispara un evento UI (ej. `PLAYER_LOGIN`)
2. `hooks/EventHooks` lo captura via detour
3. Lo empuja al named pipe de eventos via `EventPipe`
4. `StateCache` sondea estado del jugador/mundo/pantalla cada frame
5. `client.screenChanged` se empuja a `EventPipe` (el único evento diff que llega a suscriptores externos)

## Modelo de Hilos

| Hilo | Rol |
|---|---|
| **Hilo del juego** (Main/Frame) | Ejecuta el loop de frames de WoW. Los callbacks de hooks se ejecutan aquí. `StateCache::update()` se llama cada frame. Las lecturas de memoria del juego ocurren en este hilo. |
| **Hilo del pipe RPC** | Lee peticiones JSON-RPC entrantes, las despacha a handlers. Bloquea en `ReadFile`. |
| **Hilo del pipe de eventos** | Empuja eventos al pipe secundario. Bloquea en `WriteFile`. |
| **Ejecución de handlers RPC** | Algunos handlers encolan trabajo al hilo del juego via `Hooks::Frame::Post` (async), otros bloquean el hilo RPC con sondeo. |

> [!NOTE]
> Las operaciones Lua (`lua.evaluate`, `lua.execute`) usan `lua_pcall` con `errfunc=0` y son seguras de llamar desde el hilo RPC. Las operaciones que cambian estado (login, logout, enter world) se encolan al hilo del juego para evitar condiciones de carrera.

## Aislamiento de Offsets

Todas las direcciones hardcodeadas del juego, punteros a función y offsets de estructuras se definen como `constexpr uintptr_t` en `offsets/Offsets*.h`. No aparecen literales `0x...` fuera de esta carpeta.

```
offsets/
├── OffsetsClient.h      Estado del cliente, net client, flag in-world
├── OffsetsConsole.h     Funciones CVar
├── OffsetsFrame.h       Render frame, D3D
├── OffsetsGroup.h       Estructuras de grupo/raid
├── OffsetsLoading.h     Flag de carga y progreso
├── OffsetsLogin.h       Autenticación, logout, lista de personajes
├── OffsetsLua.h         Estado de Lua, signal event handler
├── OffsetsMap.h         Conversión de posición en mapa
├── OffsetsObjectMgr.h   Object manager
├── OffsetsPlayer.h      Offsets específicos del jugador
└── OffsetsUnit.h        Estructura de unidad, movimiento, auras
```

Los archivos fuente referencia estas como `Offsets::Namespace::CONSTANT`, nunca como literales hex crudos.

## Sistema de Eventos

El sistema de eventos tiene dos capas:

1. **EventBus** (`runtime/events/EventBus.h`) — pub/sub en proceso. Usado por `StateCache` para emitir eventos (`player.healthChanged`, `world.zoneChanged`, etc.). Actualmente ningún componente interno se suscribe a estos eventos. Disponible para uso futuro en proceso.

2. **EventPipe** (`runtime/events/EventPipe.h`) — named pipe secundario en `\\.\pipe\WowGameEvent`. Empuja eventos como JSON para consumidores externos. Escrito por `StateCache` (solo `client.screenChanged`) y `LoadingHooks` (eventos de carga).

Eventos empujados al EventPipe:
- Eventos SDK: `client.screenChanged`, `loading.startup`, `loading.progress`, `loading.finished`
- Eventos UI de WoW capturados por `EventHooks` (si se suscriben via `events.subscribe`)
