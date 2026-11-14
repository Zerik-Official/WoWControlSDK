# Glosario

## Transporte y protocolo

**JSON-RPC 2.0**
:   Protocolo ligero de llamadas remotas que codifica peticiones y respuestas como objetos JSON. Cada petición indica un `method` y lleva `params` opcionales; cada respuesta lleva `result` o `error`.

**Named pipe**
:   Canal de comunicación entre procesos de Windows identificado por una ruta como `\\.\pipe\WowGameCommand`. El SDK crea el extremo servidor y la aplicación cliente abre el extremo cliente.

**Pipe de comandos**
:   `\\.\pipe\WowGameCommand`. Acepta una petición por conexión, la despacha y devuelve una respuesta.

**Pipe de eventos**
:   `\\.\pipe\WowGameEvent`. Flujo de solo escritura con eventos JSON delimitados por saltos de línea. Acepta un cliente a la vez.

**Modo mensaje**
:   Modo de named pipe que conserva los límites de escritura. El pipe de comandos lo usa para que una escritura sea una petición. El pipe de eventos usa modo byte.

**Notificación**
:   Petición JSON-RPC sin `id`. El SDK espera un identificador y siempre devuelve una respuesta, así que usa un `id` numérico.

## Inyección y hooks

**Inyección de DLL**
:   Carga de una biblioteca dinámica dentro de otro proceso. El inyector escribe la ruta de la DLL en `Wow.exe` y llama a `LoadLibrary` mediante un hilo remoto.

**Congelar-sincronizar**
:   Estrategia de inyección de `WowInjector.exe`: iniciar el juego suspendido, inyectar la DLL, esperar el pipe de comandos y solo entonces reanudar el juego.

**Detour**
:   Hook de función instalado con Microsoft Detours. El SDK intercepta funciones del juego para capturar eventos, salida de consola y callbacks de frame.

**Offset**
:   Dirección numérica o desplazamiento de un campo dentro de una estructura del binario del juego. Todos los offsets viven en `offsets/` como `constexpr uintptr_t`.

**Lua VM**
:   Intérprete Lua embebido en WoW. Los métodos `lua.*` ejecutan código en esta VM y solo serializan valores boolean, number y string.

## Estado del juego

**Pantalla glue**
:   Pantalla que no es de juego, como el login o la selección de personajes. `client.getScreen` informa `login`, `charselect`, `world` o el nombre glue crudo.

**Token de unidad**
:   Cadena que identifica una unidad, por ejemplo `player`, `target`, `focus`, `mouseover`, `party1` o `raid1`. También se acepta una cadena GUID.

**GUID**
:   Identificador de juego de 64 bits devuelto como arreglo `[high, low]` de dos enteros sin signo de 32 bits.

**Posición en el mundo**
:   Coordenadas del juego en el mapa actual, devueltas como `{x, y, z}`. Funciona para cualquier unidad.

**Posición de mapa**
:   Coordenadas normalizadas que usan el mapa y el minimapa. El juego solo las resuelve para miembros de grupo o raid; otras unidades devuelven ceros.

**Caché de estado**
:   Instantánea por frame del estado del jugador, el mundo y la pantalla que mantiene `runtime/state/StateCache`. Los dominios `player.*` y `world.*` la leen en lugar de bloquearse en el hilo del juego.

**EventBus**
:   Bus de publicación y suscripción en proceso de `runtime/events/EventBus.h`. Se usa internamente y no se expone a los clientes RPC.

**EventPipe**
:   Flujo de eventos externo de `runtime/events/EventPipe.h` que alimenta `\\.\pipe\WowGameEvent` y el búfer circular de historial.

## Windows y entrega

**HWND**
:   Handle de ventana de Windows. El SDK lo devuelve como entero sin signo mediante `window.getHandle`.

**Compilación estricta**
:   `mkdocs build --strict`, que falla ante cualquier advertencia, incluidas páginas ausentes de la navegación y enlaces internos rotos.

**Manifiesto**
:   Archivo `manifest.json` generado para una versión. Registra la versión del esquema, la etiqueta, el commit y el hash SHA-256 y el tamaño de cada binario.

**Herramienta CLI**
:   Cliente interactivo de Python en `tests/wowsdk_cli.py`. Prueba todos los dominios RPC e incluye un listener de eventos en vivo.
