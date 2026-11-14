# Primeros pasos

Esta guía instala las herramientas, compila el SDK, lo inyecta en el juego y envía una primera petición RPC.

## Requisitos

| Herramienta | Versión | Uso |
| --- | --- | --- |
| Windows | 10 u 11, x64 | SDK, inyector y CLI |
| Visual Studio | 2022 con la carga de trabajo Desarrollo de escritorio con C++ | Compilador C++17 |
| CMake | 3.20 o posterior | Sistema de compilación |
| Git | 2.40 o posterior | Control de versiones |
| Python | 3.10 o posterior | CLI de pruebas y documentación |
| pywin32 | Última versión | CLI de pruebas |

## Compilar el SDK

1. Clona el repositorio y entra en él:

   ```bash
   git clone https://github.com/Zerik-Official/WoWControlSDK.git
   cd WoWControlSDK
   ```

2. Configura el proyecto con CMake:

   ```bash
   cmake -B build -A Win32
   ```

3. Compila la configuración Release:

   ```bash
   cmake --build build --config Release
   ```

La compilación genera dos artefactos:

| Archivo | Propósito |
| --- | --- |
| `bin/release/WoWControlSDK.dll` | El SDK que se inyecta en el juego |
| `bin/release/WowInjector.exe` | El inyector congelar-sincronizar incluido |

Una compilación Debug escribe en `bin/debug/`. Consulta [Compilación](operations/building.md) para la lista completa de objetivos y las opciones de compilación.

## Inyectar la DLL

Inicia el juego con el inyector incluido:

```bash
WowInjector.exe --wow "C:\World of Warcraft\Wow.exe" --dll "bin\release\WoWControlSDK.dll"
```

El inyector inicia `Wow.exe` suspendido, carga la DLL, espera hasta 15 segundos a que exista `\\.\pipe\WowGameCommand` y reanuda el juego. El SDK está completamente inicializado antes de que se ejecute el primer frame.

!!! tip
    Ejecuta el inyector desde la raíz del repositorio o usa rutas absolutas para `--wow` y `--dll`. El inyector resuelve las rutas relativas desde el directorio de trabajo actual.

El inyector acepta argumentos adicionales, como `--kill`, `--wow-args` y comandos `--pipe-cmd` repetibles. Consulta [Inyección](injection.md) para todas las opciones y el flujo alternativo.

## Enviar una primera petición

El pipe de comandos atiende una petición por conexión. Instala `pywin32` y ejecuta un cliente pequeño:

```bash
pip install pywin32
```

```python
import json

import win32file


def rpc(method, params=None, request_id=1):
    pipe = win32file.CreateFile(
        r"\\.\pipe\WowGameCommand",
        win32file.GENERIC_READ | win32file.GENERIC_WRITE,
        0,
        None,
        win32file.OPEN_EXISTING,
        0,
        None,
    )
    try:
        request = {
            "jsonrpc": "2.0",
            "id": request_id,
            "method": method,
            "params": params or {},
        }
        win32file.WriteFile(pipe, json.dumps(request).encode("utf-8"))
        _, data = win32file.ReadFile(pipe, 65536)
        return json.loads(data.decode("utf-8").rstrip("\x00"))
    finally:
        win32file.CloseHandle(pipe)


print(rpc("client.ping"))
print(rpc("client.getScreen"))
```

`client.ping` devuelve `{"status": "ok"}` cuando la DLL está cargada y el pipe escucha. `client.getScreen` indica si el cliente está en la pantalla de login, en la selección de personajes o dentro del mundo.

!!! note
    Los métodos Lua y de unidades requieren la pantalla adecuada. `lua.*`, `unit.*`, `group.*`, `player.*` y `world.*` normalmente necesitan que el jugador esté dentro del mundo. `client.login` requiere la pantalla de login y `client.enterWorld` requiere la selección de personajes.

## Usar la herramienta CLI

El repositorio incluye un CLI interactivo de Python para probar todos los dominios:

```bash
python tests/wowsdk_cli.py
```

La herramienta requiere `pywin32` y lee `WOW_PATH`, `DLL_PATH` e `INJECTOR` de la cabecera de `tests/wowsdk_cli.py`. Ajusta esas rutas antes de usar las opciones de lanzamiento e inyección. Consulta [Herramienta CLI](operations/cli.md) para el menú y el listener de eventos en vivo.

## Servir la documentación

Instala las dependencias de documentación e inicia el sitio local:

```bash
pip install -r requirements-docs.txt
mkdocs serve
```

El sitio en inglés se sirve en `http://127.0.0.1:8000/` y el español en `http://127.0.0.1:8000/es/`. El workflow `docs` valida ambos idiomas con `mkdocs build --strict` y publica el sitio en GitHub Pages desde `main`.

## Solución de problemas

| Síntoma | Causa y solución |
| --- | --- |
| `El sistema no puede encontrar el archivo especificado` al abrir el pipe | La DLL no está cargada o el inyector aún no ha terminado. Inicia el juego con el inyector y espera al pipe. |
| `Method not found` | El nombre del método está mal escrito o pertenece a otra versión del SDK. Revisa la [referencia de métodos](methods/index.md). |
| `not on login screen` | Se llamó a `client.login` después de abandonar la pantalla de login. Vuelve a la pantalla de login y reintenta. |
| `not on character select screen` | `client.enterWorld` requiere la lista de personajes. Llama a `client.login` con `realmName` o espera a la selección de personajes. |
| `player not available` o `world not available` | La caché por frame está vacía. El jugador todavía no ha entrado al mundo. |
| `unit.mapPosition` devuelve ceros | Las posiciones de mapa solo se resuelven para miembros de grupo o raid. Usa `unit.position` para otras unidades. |
| El pipe se desconecta tras cada llamada | Es el comportamiento esperado. El pipe de comandos atiende una petición por conexión; abre una conexión nueva para cada petición. |

## Cliente soportado

Solo se soporta **World of Warcraft 3.3.5a (build 12340)**. Los offsets y las estructuras son específicos de esa build. Se recomienda un cliente limpio y sin modificaciones, aunque los clientes ligeramente modificados también pueden funcionar.
