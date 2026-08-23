# Métodos window.*

Gestiona la ventana principal de World of Warcraft — visibilidad, posición, tamaño, foco y efectos visuales.

---

El SDK localiza la ventana mediante `FindWindowA` sobre las clases internas conocidas (`GxWindowClassD3d`, `GxWindowClassD3d9Ex`, `GxWindowClassOpenGl`). Todas las operaciones usan Win32 nativo directamente sobre el `HWND`.

> [!NOTE]
> El `HWND` es un identificador de ventana de Windows (handle) que se puede usar con la API Win32. El SDK lo devuelve como un entero (`uintptr_t`) para interoperabilidad con otros lenguajes y librerías.

---

## `window.getHandle`

Obtiene el `HWND` de la ventana principal de WoW como entero.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `handle` | integer \| null | `HWND` como `uintptr_t`, o `null` si no se encuentra |

---

## `window.getInfo`

Devuelve información completa de la ventana en una sola llamada.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `title` | string | título de la ventana |
| `className` | string | nombre de la clase Win32 (p.ej. `GxWindowClassD3d`) |
| `visible` | boolean | `true` si no está oculta |
| `minimized` | boolean | `true` si está minimizada |
| `maximized` | boolean | `true` si está maximizada |
| `topMost` | boolean | `true` si tiene `WS_EX_TOPMOST` |
| `bounds.x` | integer | coordenada X de la esquina superior izquierda |
| `bounds.y` | integer | coordenada Y de la esquina superior izquierda |
| `bounds.width` | integer | ancho en píxeles |
| `bounds.height` | integer | alto en píxeles |

---

## `window.show`

Muestra la ventana (equivalente a `ShowWindow(SW_SHOW)`).

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `ok` | boolean | siempre `true` |

---

## `window.hide`

Oculta la ventana (equivalente a `ShowWindow(SW_HIDE)`). El proceso del juego sigue ejecutándose a full speed — no reduce FPS como al minimizar.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `ok` | boolean | siempre `true` |

---

## `window.minimize`

Minimiza la ventana (equivalente a `ShowWindow(SW_MINIMIZE)`).

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `ok` | boolean | siempre `true` |

> [!NOTE]
> Al minimizar, WoW reduce su procesamiento en background. Para limitar FPS en background sin minimizar, usa el CVar `maxFPSBk` vía `lua.setCVar("maxFPSBk", 30)`.

---

## `window.restore`

Restaura la ventana desde minimizada o maximizada (equivalente a `ShowWindow(SW_RESTORE)`).

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `ok` | boolean | siempre `true` |

---

## `window.maximize`

Maximiza la ventana (equivalente a `ShowWindow(SW_MAXIMIZE)`).

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `ok` | boolean | siempre `true` |

---

## `window.focus`

Trae la ventana al frente y le da el foco (llama a `SetForegroundWindow`; si estaba minimizada, la restaura primero).

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `ok` | boolean | siempre `true` |

---

## `window.getPosition`

Obtiene la posición de la esquina superior izquierda de la ventana.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `x` | integer | coordenada X |
| `y` | integer | coordenada Y |

---

## `window.setPosition`

Cambia la posición de la ventana sin alterar su tamaño.

### Parámetros

| Nombre | Tipo | Requerido | Descripción |
|---|---|---|---|
| `x` | integer | sí | nueva coordenada X |
| `y` | integer | sí | nueva coordenada Y |

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `ok` | boolean | siempre `true` |

---

## `window.getSize`

Obtiene las dimensiones de la ventana (incluyendo bordes y barra de título).

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `width` | integer | ancho en píxeles |
| `height` | integer | alto en píxeles |

---

## `window.setSize`

Cambia el tamaño de la ventana sin moverla.

### Parámetros

| Nombre | Tipo | Requerido | Descripción |
|---|---|---|---|
| `width` | integer | sí | nuevo ancho en píxeles |
| `height` | integer | sí | nuevo alto en píxeles |

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `ok` | boolean | siempre `true` |

> [!NOTE]
> El cambio se hace con `SetWindowPos(SWP_NOMOVE)` — WoW recibirá `WM_SIZE` y ajustará su renderizador automáticamente. El bloqueo de aspect ratio (`gxAspect` / `windowResizeLock`) solo se aplica durante resize interactivo (`WM_SIZING`), no programático.

---

## `window.setBounds`

Cambia posición y tamaño en una sola llamada (parcial: omite los campos no enviados).

### Parámetros

| Nombre | Tipo | Requerido | Descripción |
|---|---|---|---|
| `x` | integer | no | nueva coordenada X |
| `y` | integer | no | nueva coordenada Y |
| `width` | integer | no | nuevo ancho en píxeles |
| `height` | integer | no | nuevo alto en píxeles |

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `ok` | boolean | siempre `true` |

> [!NOTE]
> Limpia el recorte de cursor (`ClipCursor(NULL)`) antes de mover, ya que WoW recorta el cursor al rect de la ventana tras cambios de formato (`DeviceSetFormat`).

---

## `window.setTitle`

Cambia el título de la ventana.

### Parámetros

| Nombre | Tipo | Requerido | Descripción |
|---|---|---|---|
| `title` | string | sí | nuevo título |

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `ok` | boolean | siempre `true` |

---

## `window.setTopMost`

Activa o desactiva el estilo `WS_EX_TOPMOST` (la ventana se mantiene sobre todas las demás).

### Parámetros

| Nombre | Tipo | Requerido | Descripción |
|---|---|---|---|
| `topMost` | boolean | sí | `true` = siempre encima, `false` = normal |

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `ok` | boolean | siempre `true` |

---

## `window.setOpacity`

Ajusta la opacidad de la ventana (0 = invisible, 100 = opaca). Usa `WS_EX_LAYERED` + `SetLayeredWindowAttributes`.

### Parámetros

| Nombre | Tipo | Requerido | Por defecto | Descripción |
|---|---|---|---|---|
| `percent` | integer | no | 100 | opacidad 0–100 |

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `ok` | boolean | siempre `true` |

> [!NOTE]
> Valores < 100 activan `WS_EX_LAYERED`; 100 lo desactiva restaurando el estilo original. No funciona en modo fullscreen exclusivo de D3D.

---

## `window.flashTaskbar`

Hace parpadear el botón de la barra de tareas (estilo notificación de Discord). Usa `FlashWindowEx(FLASHW_TRAY | FLASHW_TIMERNOFG)`.

### Parámetros

| Nombre | Tipo | Requerido | Por defecto | Descripción |
|---|---|---|---|---|
| `count` | integer | no | 0 (infinito) | número de parpadeos; 0 = hasta que la ventana reciba foco |
| `timeoutMs` | integer | no | 0 (default del sistema) | intervalo entre parpadeos en ms |

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `ok` | boolean | siempre `true` |

---

## `window.setIcon`

Cambia el icono de la ventana (barra de título `ICON_SMALL` 16×16 y/o Alt+Tab `ICON_BIG` 32×32). Acepta un archivo `.ico` que puede contener múltiples resoluciones; el SDK selecciona la representación más adecuada dentro del contenedor.

### Parámetros

| Nombre | Tipo | Requerido | Descripción |
|---|---|---|---|
| `path` | string | sí* | ruta absoluta al archivo `.ico` |
| `type` | string | no | `"small"` (solo 16×16), `"big"` (solo 32×32), omitido = ambos |
| `restore` | boolean | sí* | `true` para restaurar el icono original de WoW |

> [!NOTE]
> `path` y `restore` son mutuamente excluyentes; debe proporcionarse exactamente uno.

### Respuesta

| Campo | Tipo | Descripción |
|---|---|---|
| `ok` | boolean | `true` si la operación tuvo éxito |

> [!NOTE]
> Al cargar un `.ico` sin `type`, el SDK extrae la mejor representación 16×16 para `ICON_SMALL` y 32×32 para `ICON_BIG`. Windows selecciona automáticamente la representación más adecuada dentro del contenedor `.ico` según el DPI y contexto.
>
> El SDK guarda el icono original de WoW **solo en la primera modificación**. Llamadas posteriores a `setIcon` no sobrescriben el backup, permitiendo restaurar al estado original en cualquier momento.
>
> Los iconos personalizados cargados con `LoadImage` son liberados (`DestroyIcon`) automáticamente al cambiar de icono o al descargar el SDK. El icono original de WoW **nunca se destruye** (propiedad de la aplicación).
>
> **El icono de la barra de tareas probablemente se toma de los recursos del ejecutable (Wow.exe); `window.setIcon` solo afecta titlebar y Alt+Tab.**

---

## Eventos de ventana

El servicio hace subclassing de `GWLP_WNDPROC` y emite estos eventos al event pipe (`\\.\pipe\WowGameEvent`):

| Evento | Payload | Descripción |
|---|---|---|
| `window.resized` | `{width, height}` | tras `WM_SIZE` |
| `window.moved` | `{x, y}` | tras `WM_MOVE` |
| `window.focusChanged` | `{focused: boolean}` | `WM_ACTIVATE` / `WM_SETFOCUS` / `WM_KILLFOCUS` |
| `window.stateChanged` | `{state: "minimized"\|"maximized"\|"restored"}` | `WM_SYSCOMMAND` SC_MINIMIZE / SC_RESTORE / SC_MAXIMIZE |
| `window.destroyed` | `{}` | `WM_DESTROY` — el cache de HWND se invalida; la próxima llamada vuelve a buscar |

> [!NOTE]
> Si WoW cambia de renderer (p.ej. D3D ↔ D3D9Ex) y recrea la ventana, `WM_DESTROY` invalida el cache y la siguiente llamada a cualquier método re-descubre el `HWND` automáticamente.