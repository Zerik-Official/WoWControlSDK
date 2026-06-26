# Inyección

## Visión General

WoWControlSDK incluye `WowInjector.exe`, un inyector independiente que lanza Wow.exe, inyecta la DLL y sincroniza el arranque para que el SDK esté completamente inicializado antes de que el juego comience.

## Flujo de Inyección Recomendado

El inyector incluido usa un enfoque de congelar-sincronizar:

1. **Lanzamiento suspendido** — Wow.exe se inicia con `CREATE_SUSPENDED`, congelando todos los hilos antes de que ejecute cualquier código del juego
2. **Inyección DLL** — `CreateRemoteThread` + `LoadLibrary` carga `WoWControlSDK.dll` en el proceso congelado
3. **Init DLL** — El hilo principal se reanuda momentáneamente, dejando que `DllMain` ejecute (consola, hooks, hilos del pipe se inicializan)
4. **Espera del pipe** — El juego se mantiene congelado. El servidor del pipe RPC del SDK (`\\.\pipe\WowGameCommand`) comienza a escuchar
5. **Descongelar** — Una vez que el pipe acepta conexiones (o después de `--wait` ms con `--no-pipe`), el juego se reanuda normalmente

Esto asegura que el SDK esté listo antes de que cualquier frame del juego se ejecute — sin condiciones de carrera, sin eventos perdidos.

```
WowInjector.exe
    │
    ├── CreateProcess(Wow.exe, CREATE_SUSPENDED)
    ├── VirtualAllocEx + WriteProcessMemory + CreateRemoteThread(LoadLibrary)
    ├── ResumeThread (DllMain ejecuta brevemente)
    ├── Esperar named pipe
    └── ResumeThread (juego inicia)
```

## Uso

```bash
WowInjector.exe --wow "C:\World of Warcraft\Wow.exe" --dll "bin\release\WoWControlSDK.dll"
```

### Argumentos

| Argumento | Descripción |
|---|---|
| `--wow <path>` | Ruta a `Wow.exe` |
| `--dll <path>` | Ruta a `WoWControlSDK.dll` |
| `--pipe <name>` | Pipe named a esperar antes de descongelar (por defecto: `WowGameCommand`) |
| `--no-pipe` | Descongela después de `--wait` ms en vez de esperar el pipe |
| `--wait <ms>` | Milisegundos a esperar cuando `--no-pipe` está activo (por defecto: 500) |
| `--kill` | Mata el proceso de WoW existente antes de lanzar |
| `--wow-args <str>` | Argumentos extra para Wow.exe |
| `--pipe-cmd <json>` | Envía un comando JSON-RPC después de que el pipe esté listo (repetible) |

### Ejemplo con opciones

```bash
WowInjector.exe ^
    --wow "C:\World of Warcraft\Wow.exe" ^
    --dll "C:\WoWControlSDK\bin\release\WoWControlSDK.dll" ^
    --pipe-cmd "{\"jsonrpc\":\"2.0\",\"method\":\"dll.toggleConsole\",\"id\":1}"
```

> [!TIP]
> El enfoque de congelar-sincronizar es el método recomendado — garantiza que la DLL esté completamente inicializada antes de que cualquier frame del juego se ejecute. Es la forma más segura de asegurar un funcionamiento confiable.

## Inyección Alternativa

No estás obligado a usar el inyector incluido. Puedes usar cualquier inyector DLL, como [Cheat Engine](https://www.cheatengine.org/).

Al usar un inyector de terceros:
- El juego ya debe estar ejecutándose
- La DLL se inicializará asincrónicamente (los hooks se adjuntan durante el siguiente frame)
- El pipe estará disponible poco después de la inyección
- Algunos eventos que ocurran entre el inicio del juego y la inyección podrían perderse

## Comandos RPC en Inyección

Con `--pipe-cmd`, puedes encolar comandos JSON-RPC para ejecutar inmediatamente después de que el SDK se inicialice. Útil para automatización:

```bash
WowInjector.exe --wow "..." --dll "..." ^
    --pipe-cmd "{\"jsonrpc\":\"2.0\",\"method\":\"dll.setLogConfig\",\"params\":{\"showInConsole\":true},\"id\":1}" ^
    --pipe-cmd "{\"jsonrpc\":\"2.0\",\"method\":\"dll.toggleConsole\",\"id\":2}"
```

Los comandos se envían en orden. Cada uno espera una respuesta antes de enviar el siguiente. Comandos seguros al inicio incluyen `dll.toggleConsole`, `dll.setLogConfig` y `dll.getInfo`. Evita comandos dependientes del juego (`lua.*`, `client.login`, `unit.*`) hasta que el jugador haya llegado a la pantalla adecuada.

## Pipe de Eventos

Además del pipe RPC principal (`\\.\pipe\WowGameCommand`), el SDK abre un pipe secundario en `\\.\pipe\WowGameEvent` para transmitir eventos en tiempo real. Este pipe es de solo escritura desde el lado del SDK — conéctate como cliente para recibir notificaciones de eventos a medida que ocurren.

Ver [Métodos events.*](03-metodos/events.md) para eventos disponibles y el ejemplo del listener en Python CLI.
