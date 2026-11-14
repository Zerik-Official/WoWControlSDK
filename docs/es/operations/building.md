# Compilación

El proyecto usa CMake para producir dos artefactos: la DLL que se inyecta y el inyector independiente. Visual Studio 2022 aporta la cadena de herramientas de C++17.

## Requisitos

| Herramienta | Versión | Función |
| --- | --- | --- |
| Windows | 10 u 11, x64 | Plataforma objetivo |
| Visual Studio | 2022 con Desarrollo para escritorio con C++ | Compilador y Windows SDK |
| CMake | 3.20 o posterior | Sistema de compilación |
| Git | 2.40 o posterior | Control de versiones |

Las fuentes de terceros están incluidas en `deps/`, entre ellas Microsoft Detours, así que la compilación no necesita ningún gestor de paquetes ni acceso a red.

## Configurar y compilar

Desde la raíz del repositorio:

```bash
cmake -B build -A Win32
cmake --build build --config Release
```

`cmake -B build -A Win32` genera la solución de Visual Studio en `build/`. La opción `-A Win32` selecciona la cadena de herramientas de 32 bits, necesaria porque `Wow.exe` es un proceso de 32 bits. El segundo comando compila la configuración Release.

Para una compilación Debug:

```bash
cmake --build build --config Debug
```

## Objetivos y salidas

| Objetivo | Salida | Descripción |
| --- | --- | --- |
| `WoWControlSDK` | `bin/release/WoWControlSDK.dll` | El SDK que se inyecta |
| `WowInjector` | `bin/release/WowInjector.exe` | El inyector congelar-sincronizar |

Las compilaciones Debug escriben en `bin/debug/`. La configuración de CMake define explícitamente los directorios de salida, de modo que ambos objetivos acaban en la misma carpeta.

## Ajustes de compilación

El `CMakeLists.txt` raíz define lo siguiente:

| Ajuste | Valor | Efecto |
| --- | --- | --- |
| `CMAKE_CXX_STANDARD` | `17` | Requiere un compilador de C++17 |
| `_WIN32_WINNT` | `0x0601` | Apunta a las API de Windows 7 y posteriores |
| `WIN32_LEAN_AND_MEAN` | Definido | Excluye cabeceras de Windows de uso poco frecuente |
| `/arch:IA32` | Opción del compilador | Genera código x86 compatible con el cliente de 32 bits |
| `/GS-` | Opción del compilador | Desactiva las comprobaciones de seguridad del búfer de pila en la DLL inyectada |
| `/utf-8` | Opción del compilador | Trata los archivos fuente como UTF-8 |

La DLL enlaza con Detours. El inyector solo enlaza con `kernel32`.

## Recompilación limpia

Elimina la carpeta de compilación generada y vuelve a configurar:

```powershell
Remove-Item -Recurse -Force build
cmake -B build -A Win32
cmake --build build --config Release
```

Las carpetas `build/`, `bin/` y `obj/` están ignoradas por Git.

## Integración continua

El flujo `CI` se ejecuta en cada pull request y push a `dev` y `main`. Compila la DLL y el inyector con la cadena de herramientas de 32 bits y valida ambos idiomas de la documentación con `mkdocs build --strict`.

El flujo `Build and Release` se ejecuta con cada etiqueta `v*`. Configura CMake, compila la configuración Release, verifica las salidas esperadas y empaqueta la versión. Consulta [Versiones](releases.md) para la lista de artefactos y el manifiesto.

No hay una suite de pruebas unitarias en el repositorio. La validación es manual: compilar la DLL, inyectarla en el cliente soportado y ejercitar los métodos con la [herramienta CLI](cli.md).
