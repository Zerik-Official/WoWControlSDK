# Herramienta CLI

El repositorio incluye `tests/wowsdk_cli.py`, un cliente interactivo de Python que ejercita todos los dominios RPC. Resulta útil para pruebas manuales, para inspeccionar respuestas y como referencia para una integración propia.

## Requisitos

| Herramienta | Versión | Función |
| --- | --- | --- |
| Python | 3.10 o posterior | Ejecuta el CLI |
| pywin32 | Última | Acceso a named pipes en Windows |

```bash
pip install pywin32
```

## Configuración

La parte superior de `tests/wowsdk_cli.py` define las rutas que usan sus acciones de arranque e inyección:

```python
RPC_PIPE   = r"\\.\pipe\WowGameCommand"
EVENT_PIPE = r"\\.\pipe\WowGameEvent"

WOW_PATH  = r"D:\Juegos\World Of Warcraft\Wow.exe"
DLL_PATH  = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "bin", "release", "WoWControlSDK.dll"))
INJECTOR  = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "bin", "release", "WowInjector.exe"))
```

Ajusta `WOW_PATH` antes de usar las opciones de arranque. `DLL_PATH` e `INJECTOR` ya apuntan a la salida Release. La CLI lanza el juego con `--no-auth`, así que sus peticiones y su escucha de eventos no necesitan token.

## Ejecución

```bash
python tests/wowsdk_cli.py
```

El CLI comprueba si el pipe de comandos está disponible y muestra el menú. Cada acción envía una petición, muestra la respuesta JSON sin procesar y vuelve al menú.

## Menú

| Sección | Acciones |
| --- | --- |
| Auth | Login, aceptar EULA, TOS, terminación, escaneo y concurso, aceptar todo, obtener y refrescar personajes, entrar en el mundo, cerrar sesión, salir, obtener y definir la lista de reinos |
| Player | Estado, salud, posición y objetivo del jugador |
| Unit | `unit.get`, `unit.state`, `unit.position` |
| Object | Inspección de `object.*` |
| World y grupo | `world.getState`, `group.get`, `group.combatSummary` |
| Eventos | Listar, suscribirse, suscribirse a los predeterminados, cancelar o limpiar, ver historial, vaciar historial, definir búfer, comprobar el sistema de hooks y escuchar en vivo |
| Ventana | Todos los métodos `window.*`, incluidos `setIcon` y `restoreIcon` |
| Lua | `lua.execute`, `lua.evaluate`, `lua.getGlobal`, `lua.setGlobal` y `lua.createNamespace` |

## Comportamiento del cliente

`send()` abre una nueva conexión al pipe de comandos por cada petición, escribe el objeto JSON, lee la respuesta y cierra el handle. Esto coincide con el comportamiento de una petición por conexión de `\\.\pipe\WowGameCommand`.

```python
def send(method, params={}):
    req = {"jsonrpc": "2.0", "method": method, "params": params, "id": next_id()}
    h = win32file.CreateFile(RPC_PIPE, win32file.GENERIC_READ | win32file.GENERIC_WRITE, 0, None, win32file.OPEN_EXISTING, 0, None)
    win32file.WriteFile(h, json.dumps(req).encode("utf-8"))
    _, data = win32file.ReadFile(h, 65536)
    win32file.CloseHandle(h)
    return json.loads(data.decode("utf-8").rstrip("\x00"))
```

Si el pipe no está disponible, `send()` devuelve `{"error": "<error de Windows>"}` en lugar de lanzar una excepción.

## Escucha de eventos en vivo

**Events → Listen live** se conecta a `\\.\pipe\WowGameEvent` y muestra cada evento a medida que llega. El escucha lee un flujo de bytes, lo divide por saltos de línea y analiza cada línea como JSON.

El pipe de eventos acepta un cliente a la vez. Cierra otros escuchas antes de iniciar esta opción.

!!! tip
    Suscríbete a los eventos que necesites antes de abrir el escucha. Los eventos del SDK, como `client.screenChanged` y los eventos de carga, se transmiten siempre, pero los eventos de la UI de WoW solo aparecen tras `events.subscribe`.
