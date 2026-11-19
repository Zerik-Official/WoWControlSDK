# Building

The project uses CMake to produce two artifacts: the injected DLL and the standalone injector. Visual Studio 2022 provides the C++17 toolchain.

## Requirements

| Tool | Version | Purpose |
| --- | --- | --- |
| Windows | 10 or 11, x64 | Target platform |
| Visual Studio | 2022 with Desktop development with C++ | Compiler and Windows SDK |
| CMake | 3.20 or later | Build system |
| Git | 2.40 or later | Source control |

The third-party sources are vendored under `deps/`, so no package manager or network fetch is required during the build.

## Configure and build

From the repository root:

```bash
cmake -B build -A Win32
cmake --build build --config Release
```

`cmake -B build -A Win32` generates the Visual Studio solution in `build/`. The `-A Win32` flag selects the 32-bit toolchain, which is required because `Wow.exe` is a 32-bit process. The second command builds the Release configuration.

For a Debug build:

```bash
cmake --build build --config Debug
```

## Targets and outputs

| Target | Output | Description |
| --- | --- | --- |
| `WoWControlSDK` | `bin/release/WoWControlSDK.dll` | The injected SDK |
| `WowInjector` | `bin/release/WowInjector.exe` | The freeze-sync injector |

Debug builds write to `bin/debug/`. The CMake configuration sets the runtime output directories explicitly, so both targets land in the same folder.

## Build settings

The root `CMakeLists.txt` sets the following:

| Setting | Value | Effect |
| --- | --- | --- |
| `CMAKE_CXX_STANDARD` | `17` | Requires a C++17 compiler |
| `_WIN32_WINNT` | `0x0601` | Targets Windows 7 APIs and later |
| `WIN32_LEAN_AND_MEAN` | Defined | Excludes rarely used Windows headers |
| `/arch:IA32` | Compiler option | Produces x86 code compatible with the 32-bit client |
| `/GS-` | Compiler option | Disables stack buffer security checks for the injected DLL |
| `/utf-8` | Compiler option | Treats source files as UTF-8 |

The DLL has no third-party link dependencies. The injector links against `kernel32` only.

## Clean rebuild

Remove the generated build folder and configure again:

```powershell
Remove-Item -Recurse -Force build
cmake -B build -A Win32
cmake --build build --config Release
```

The `build/`, `bin/` and `obj/` folders are ignored by Git.

## Continuous integration

The `CI` workflow runs on every pull request and push to `dev` and `main`. It compiles the DLL and the injector with the Win32 toolchain and validates both documentation languages with `mkdocs build --strict`.

The `Build and Release` workflow runs on every `v*` tag. It configures CMake, builds the Release configuration, verifies the expected outputs and packages the release. See [Releases](releases.md) for the artifact list and the manifest.

There is no unit test suite in the repository. Validation is manual: build the DLL, inject it into the supported client and exercise the methods through the [desktop client](cli.md).
