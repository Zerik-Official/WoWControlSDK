# WoWControlSDK

WoWControlSDK es una DLL inyectable ligera y un SDK para **World of Warcraft 3.3.5a (build 12340)**. Expone los métodos internos del juego mediante **JSON-RPC 2.0** sobre named pipes de Windows, de modo que launchers, overlays, herramientas de automatización y otras aplicaciones externas pueden leer el estado del juego y controlar el cliente desde cualquier lenguaje.

La DLL se ejecuta dentro de `Wow.exe`. Un cliente se conecta a `\\.\pipe\WowGameCommand`, envía una petición JSON-RPC y recibe una respuesta JSON. Un segundo pipe, `\\.\pipe\WowGameEvent`, transmite eventos como cambios de pantalla, progreso de carga y eventos UI de WoW suscritos.

## De un vistazo

| Elemento | Valor |
| --- | --- |
| Cliente soportado | World of Warcraft 3.3.5a, build 12340 |
| Plataforma | Windows 10 u 11, x64 |
| Transporte | Named pipes de Windows |
| Protocolo | JSON-RPC 2.0, una petición por conexión |
| Pipe de comandos | `\\.\pipe\WowGameCommand` |
| Pipe de eventos | `\\.\pipe\WowGameEvent` |
| Dominios RPC | `lua`, `unit`, `group`, `player`, `world`, `object`, `client`, `events`, `dll`, `window`, `friends`, `summon`, `lfg`, `trade`, `duel`, `battlefield`, `chat` |
| Lenguajes | Cualquier lenguaje capaz de abrir un named pipe |

## Por dónde empezar

| Si quieres | Lee |
| --- | --- |
| Compilar la DLL, inyectarla y enviar una primera petición | [Primeros pasos](getting-started.md) |
| Entender cómo encajan las capas y los hilos | [Arquitectura](architecture.md) |
| Conocer el proceso de inyección en detalle | [Inyección](injection.md) |
| Conectarte desde Python, C# u otro lenguaje | [Protocolo RPC](protocol.md) |
| Conectar un addon con tu aplicación | [Integración con addons](addons.md) |
| Consultar un método y su respuesta | [Métodos](methods/index.md) |
| Preparar una versión o revisar el CI | [Operación](operations/building.md) |
| Buscar un término | [Glosario](glossary.md) |

## Contenido

**Guías**

- [Primeros pasos](getting-started.md): requisitos, compilación, inyección, primera petición y documentación local.
- [Arquitectura](architecture.md): componentes, diseño por capas, flujo de peticiones y eventos, hilos y offsets.
- [Inyección](injection.md): el inyector congelar-sincronizar, sus argumentos, inyectores alternativos y comandos de arranque.
- [Protocolo RPC](protocol.md): comportamiento de los pipes, sobre JSON-RPC, errores, catálogo de métodos y ejemplos de cliente.
- [Integración con addons](addons.md): el puente de addons, `SDK.Emit`, llamar funciones de un addon y el addon de ejemplo.

**Referencia**

- [Métodos](methods/index.md): convenciones comunes a todos los dominios y enlaces a cada página.
- [lua.*](methods/lua.md), [unit.*](methods/unit.md), [group.*](methods/group.md), [player.*](methods/player.md), [world.*](methods/world.md), [object.*](methods/object.md), [client.*](methods/client.md), [events.*](methods/events.md), [dll.*](methods/dll.md), [window.*](methods/window.md), [friends.*](methods/friends.md), [summon.*](methods/summon.md), [lfg.*](methods/lfg.md), [trade.*](methods/trade.md), [duel.*](methods/duel.md), [battlefield.*](methods/battlefield.md) y [chat.*](methods/chat.md).
- [Glosario](glossary.md): términos de transporte, del juego y de ingeniería.

**Operación**

- [Compilación](operations/building.md): requisitos, objetivos de CMake y estructura de salida.
- [Cliente de escritorio](operations/cli.md): la aplicación de escritorio para llamar a todos los métodos y ver los eventos en vivo.
- [Versiones](operations/releases.md): etiquetas, artefactos, manifiesto y GitHub Releases.
- [Sitio de documentación](operations/documentation.md): MkDocs, traducciones, validación y despliegue.

## Alcance

El SDK está pensado para integraciones legítimas como launchers, overlays de stream, Discord Rich Presence y automatización de interfaz. No es una herramienta de hacking, trampas o bots, aunque muchos métodos también pueden facilitar el botting. Úsalo de forma responsable.

Solo se soporta un cliente **3.3.5a (build 12340)** sin modificaciones o con modificaciones ligeras. Otras builds, incluidas expansiones posteriores, no comparten el mismo diseño de memoria ni el mismo entorno Lua.
