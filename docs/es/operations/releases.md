# Versiones

Una versión se crea empujando una etiqueta que empieza por `v`. El flujo `Build and Release` compila el SDK en un runner de Windows, verifica los artefactos, genera un manifiesto y publica una versión de GitHub.

## Disparador

```bash
git tag v1.0.0
git push origin v1.0.0
```

El nombre de la etiqueta se convierte en la versión y en el nombre del paquete.

## Pipeline

```mermaid
sequenceDiagram
    participant Dev as Mantenedor
    participant CI as GitHub Actions
    participant GH as GitHub Release

    Dev->>CI: push etiqueta v1.0.0
    CI->>CI: cmake -B build -A Win32
    CI->>CI: cmake --build build --config Release
    CI->>CI: pyinstaller --windowed --onedir (cliente de escritorio)
    CI->>CI: verificar DLL, inyector, cliente y licencia
    CI->>CI: calcular SHA-256 y tamaño de cada binario
    CI->>CI: escribir manifest.json
    CI->>CI: crear WoWControlSDK-v1.0.0.zip y WoWControlClient-v1.0.0.zip
    CI->>GH: publicar versión con notas generadas
```

| Paso | Qué hace |
| --- | --- |
| Configurar | Ejecuta `cmake -B build -A Win32` |
| Compilar | Ejecuta `cmake --build build --config Release` |
| Cliente | Instala `requirements-build.txt` y compila el [cliente de escritorio](cli.md) con PyInstaller, sin consola y como carpeta (`--windowed --onedir`) |
| Verificar | Falla si falta `bin/release/WoWControlSDK.dll`, `bin/release/WowInjector.exe`, `dist/WoWControlClient/WoWControlClient.exe` o `LICENSE` |
| Manifiesto | Calcula el hash SHA-256 y el tamaño en bytes de la DLL y del inyector |
| Empaquetar | Crea `WoWControlSDK-<version>.zip` con los binarios y `WoWControlClient-<version>.zip` con el cliente y una copia de los binarios |
| Publicar | Crea la versión de GitHub con notas generadas y sube los dos zip y `manifest.json` |

!!! note
    El paquete del SDK solo contiene lo que necesita un launcher. El cliente se publica en un zip aparte para que las aplicaciones que descargan el SDK no bajen también el cliente.

## Contenido de los paquetes

```text
WoWControlSDK-<version>/
├── binaries/
│   ├── WoWControlSDK.dll
│   └── WowInjector.exe
├── LICENSE
└── manifest.json
```

```text
WoWControlClient-<version>/
├── WoWControlClient.exe
├── _internal/
├── binaries/
│   ├── WoWControlSDK.dll
│   └── WowInjector.exe
└── LICENSE
```

## Manifiesto

El flujo escribe `manifest.json` con esta forma:

```json
{
  "schema_version": 1,
  "version": "v1.0.0",
  "commit": "<SHA completo del commit>",
  "artifacts": {
    "WoWControlSDK.dll": {
      "sha256": "<digest hexadecimal en minúsculas>",
      "size": 123456
    },
    "WowInjector.exe": {
      "sha256": "<digest hexadecimal en minúsculas>",
      "size": 654321
    }
  }
}
```

| Campo | Descripción |
| --- | --- |
| `schema_version` | Versión del formato del manifiesto. Actualmente `1` |
| `version` | Nombre de la etiqueta que disparó la versión |
| `commit` | SHA del commit al que apunta la etiqueta |
| `artifacts` | Mapa de nombre de archivo a hash y tamaño |

## Verificar una descarga

Compara el hash del binario descargado con el manifiesto:

```powershell
Get-FileHash .\WoWControlSDK.dll -Algorithm SHA256
```

El hash resultante debe coincidir con `artifacts["WoWControlSDK.dll"].sha256`.

## Paquete local

Una compilación local produce los mismos binarios, pero no el zip ni el manifiesto. Ejecuta los comandos de [Compilación](building.md) y recoge `bin/release/WoWControlSDK.dll` y `bin/release/WowInjector.exe` si necesitas un paquete sin GitHub Actions.

## Versionado

Las etiquetas no están vinculadas a una cadena de versión dentro de la DLL. El método `dll.getInfo` informa la versión definida en `project(WoWControlSDK VERSION …)` del `CMakeLists.txt` raíz, así que actualízala antes de etiquetar una versión. El commit de la etiqueta se graba en la DLL automáticamente (consulta [Versión y commit](building.md#version-y-commit)).
