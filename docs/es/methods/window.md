# Métodos window.*

Gestionan la ventana principal de World of Warcraft: visibilidad, posición, tamaño, foco, transparencia, título, icono y notificaciones de la barra de tareas.

El SDK localiza la ventana con `FindWindowA` sobre las clases internas conocidas (`GxWindowClassD3d`, `GxWindowClassD3d9Ex` y `GxWindowClassOpenGl`). Cada operación usa llamadas nativas de Win32 sobre el `HWND`.

!!! note
    `HWND` es un identificador de ventana de Windows. El SDK lo devuelve como entero sin signo para que otros lenguajes y librerías puedan usarlo.

## window.getHandle

Devuelve el `HWND` de la ventana principal.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `handle` | integer o null | `HWND` como entero sin signo, o `null` cuando no se encuentra la ventana |

## window.getInfo

Devuelve toda la información de la ventana en una sola llamada.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `title` | string | Título de la ventana |
| `className` | string | Nombre de clase de Win32, como `GxWindowClassD3d` |
| `visible` | boolean | Indica si la ventana es visible |
| `minimized` | boolean | Indica si la ventana está minimizada |
| `maximized` | boolean | Indica si la ventana está maximizada |
| `topMost` | boolean | Indica si `WS_EX_TOPMOST` está activo |
| `startHidden` | boolean | Indica si [`window.setStartHidden`](#windowsetstarthidden) sigue manteniendo la ventana oculta |
| `bounds.x` | integer | Coordenada X de la esquina superior izquierda |
| `bounds.y` | integer | Coordenada Y de la esquina superior izquierda |
| `bounds.width` | integer | Ancho en píxeles |
| `bounds.height` | integer | Alto en píxeles |

Cuando no se encuentra la ventana, la respuesta es `{"error": "Window not found"}`.

## window.show

Muestra la ventana, equivalente a `ShowWindow(SW_SHOW)`. También termina el arranque oculto de [`window.setStartHidden`](#windowsetstarthidden), igual que `window.restore`, `window.maximize` y `window.focus`.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | Siempre `true` |

## window.hide

Oculta la ventana, equivalente a `ShowWindow(SW_HIDE)`. El juego sigue ejecutándose a plena velocidad y no limita los FPS como al minimizar.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | Siempre `true` |

## window.setStartHidden

Mantiene oculta la ventana del juego desde el momento en que el cliente la crea, para lanzamientos headless. Mientras está activo, el SDK quita `WS_VISIBLE` al crear la ventana del juego y convierte cada intento del cliente de mostrarla (`ShowWindow`, `SetWindowPos` con `SWP_SHOWWINDOW`) en una ventana oculta. Solo afecta a la ventana del juego `GxWindowClass*`, no a otras como la consola del SDK.

Llámalo antes de que el juego cree su ventana: [`WowInjector.exe --hidden`](../injection.md#argumentos) lo envía con el juego todavía congelado. `window.show`, `window.restore`, `window.maximize` y `window.focus` terminan el arranque oculto y muestran la ventana, por ejemplo cuando el personaje ya está en el mundo. Al terminar el arranque oculto, el SDK hace la ventana un píxel más alta durante dos frames y luego la restaura, para que el juego reconstruya su back buffer al tamaño real en lugar de mostrar una imagen pixelada.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `hidden` | boolean | Sí | `true` para mantener la ventana oculta, `false` para dejar de hacerlo (la ventana sigue como está hasta mostrarla) |

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | Siempre `true` |
| `startHidden` | boolean | El nuevo estado |

### Ejemplo

```json
{"jsonrpc": "2.0", "id": 1, "method": "window.setStartHidden", "params": {"hidden": true}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "startHidden": true}}
```

### Errores

| Condición | Respuesta |
| --- | --- |
| Falta `hidden` o no es booleano | `{"error": "hidden must be a boolean"}` |

!!! note
    Una ventana oculta no tiene botón en la barra de tareas. El juego sigue ejecutándose y cargando con normalidad; el modo pantalla completa puede no iniciarse bien con la ventana oculta, así que se recomienda el modo ventana para los lanzamientos headless.

## window.minimize

Minimiza la ventana, equivalente a `ShowWindow(SW_MINIMIZE)`.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | Siempre `true` |

!!! note
    WoW reduce el procesamiento en segundo plano cuando la ventana está minimizada. Para limitar los FPS en segundo plano sin minimizar, define la CVar `maxFPSBk` mediante `lua.setCVar` o `lua.execute`.

## window.restore

Restaura la ventana desde el estado minimizado o maximizado, equivalente a `ShowWindow(SW_RESTORE)`.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | Siempre `true` |

## window.maximize

Maximiza la ventana, equivalente a `ShowWindow(SW_MAXIMIZE)`.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | Siempre `true` |

## window.focus

Trae la ventana al primer plano y le da el foco. Si está minimizada, primero se restaura.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | Siempre `true` |

## window.getPosition

Devuelve la posición de la esquina superior izquierda.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `x` | integer | Coordenada X |
| `y` | integer | Coordenada Y |

Cuando no se encuentra la ventana, la respuesta es `{"error": "Window not found"}`.

## window.setPosition

Mueve la ventana sin cambiar su tamaño.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `x` | integer | Sí | Nueva coordenada X |
| `y` | integer | Sí | Nueva coordenada Y |

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | Siempre `true` |

## window.getSize

Devuelve las dimensiones de la ventana, incluidos los bordes y la barra de título.

### Parámetros

Ninguno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `width` | integer | Ancho en píxeles |
| `height` | integer | Alto en píxeles |

Cuando no se encuentra la ventana, la respuesta es `{"error": "Window not found"}`.

## window.setSize

Cambia el tamaño de la ventana sin moverla.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `width` | integer | Sí | Nuevo ancho en píxeles |
| `height` | integer | Sí | Nuevo alto en píxeles |

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | Siempre `true` |

!!! note
    La llamada usa `SetWindowPos(SWP_NOMOVE)`. WoW recibe `WM_SIZE` y ajusta su renderizador automáticamente. El bloqueo de relación de aspecto (`gxAspect` / `windowResizeLock`) solo se aplica al redimensionado interactivo, no a esta llamada.

## window.setBounds

Cambia posición y tamaño en una sola llamada.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `x` | integer | Sí | Nueva coordenada X |
| `y` | integer | Sí | Nueva coordenada Y |
| `width` | integer | Sí | Nuevo ancho en píxeles |
| `height` | integer | Sí | Nuevo alto en píxeles |

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | Siempre `true` |

!!! warning
    Los valores omitidos toman `0` por defecto. Indica siempre los cuatro campos. Para cambiar solo la posición o solo el tamaño, usa `window.setPosition` o `window.setSize`.

!!! note
    La llamada limpia el recorte del cursor con `ClipCursor(NULL)` antes de mover la ventana, porque WoW recorta el cursor al rectángulo de la ventana tras un cambio de formato (`DeviceSetFormat`).

## window.setTitle

Cambia el título de la ventana.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `title` | string | Sí | Nuevo título |

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | Siempre `true` |

## window.setTopMost

Alterna el estilo `WS_EX_TOPMOST`, que mantiene la ventana por encima de las demás.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `topMost` | boolean | Sí | `true` para mantener siempre encima, `false` para el comportamiento normal |

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | Siempre `true` |

## window.setOpacity

Define la opacidad de la ventana.

### Parámetros

| Nombre | Tipo | Obligatorio | Por defecto | Descripción |
| --- | --- | --- | --- | --- |
| `percent` | integer | No | `100` | Opacidad de `0` (invisible) a `100` (opaca) |

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | Siempre `true` |

!!! note
    Los valores por debajo de `100` activan `WS_EX_LAYERED` y usan `SetLayeredWindowAttributes`. Un valor de `100` elimina el estilo en capas. La opacidad no funciona en el modo de pantalla completa exclusiva de D3D.

## window.flashTaskbar

Hace parpadear el botón de la barra de tareas, de forma similar a una notificación de Discord. Usa `FlashWindowEx(FLASHW_TRAY | FLASHW_TIMERNOFG)`.

### Parámetros

| Nombre | Tipo | Obligatorio | Por defecto | Descripción |
| --- | --- | --- | --- | --- |
| `count` | integer | No | `0` | Número de parpadeos. `0` parpadea hasta que la ventana recibe el foco |
| `timeoutMs` | integer | No | `0` | Intervalo entre parpadeos en milisegundos. `0` usa el valor predeterminado del sistema |

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | Siempre `true` |

## window.setIcon

Cambia el icono de la ventana: el icono de la barra de título (`ICON_SMALL`, 16×16) y el icono de Alt+Tab (`ICON_BIG`, 32×32). El archivo `.ico` puede contener varias resoluciones; el SDK selecciona la representación adecuada.

### Parámetros

| Nombre | Tipo | Obligatorio | Descripción |
| --- | --- | --- | --- |
| `path` | string | Uno de los dos | Ruta absoluta al archivo `.ico` |
| `type` | string | No | `"small"` solo para 16×16, `"big"` solo para 32×32. Omitirlo para definir ambos |
| `restore` | boolean | Uno de los dos | `true` para restaurar el icono original de WoW |

`path` y `restore` son mutuamente excluyentes. Debe indicarse exactamente uno.

### Respuesta

| Campo | Tipo | Descripción |
| --- | --- | --- |
| `ok` | boolean | `true` cuando los parámetros se aceptan |

### Errores

| Condición | Error |
| --- | --- |
| Se indican `path` y `restore` | `Cannot specify both 'path' and 'restore'` |
| No se indica ninguno | `Missing required parameter: 'path' or 'restore'` |
| `path` está vacío | `Parameter 'path' cannot be empty` |
| `type` no es `small` ni `big` | `Invalid 'type' value. Expected 'small' or 'big'` |

!!! note
    Cuando se carga un `.ico` sin `type`, el SDK extrae la mejor representación de 16×16 para `ICON_SMALL` y de 32×32 para `ICON_BIG`. Windows elige la mejor representación del contenedor para el DPI y el contexto actuales.

    El icono original de WoW se guarda solo en la primera modificación. Las llamadas posteriores no sobrescriben la copia de seguridad, así que `restore` puede volver al icono original en cualquier momento. Los iconos personalizados se liberan con `DestroyIcon` cuando el icono cambia o el SDK se descarga; el icono original pertenece a la aplicación y nunca se destruye.

!!! warning
    El icono de la barra de tareas suele tomarse de los recursos de `Wow.exe`. `window.setIcon` afecta a la barra de título y a Alt+Tab, no al botón de la barra de tareas.

## Eventos de ventana

El servicio de ventana subclasifica `GWLP_WNDPROC` y emite estos eventos al pipe de eventos:

| Evento | Carga útil | Disparador |
| --- | --- | --- |
| `window.resized` | `{"width": <int>, "height": <int>}` | Después de `WM_SIZE` |
| `window.moved` | `{"x": <int>, "y": <int>}` | Después de `WM_MOVE` |
| `window.focusChanged` | `{"focused": <boolean>}` | `WM_ACTIVATE`, `WM_SETFOCUS` o `WM_KILLFOCUS` |
| `window.stateChanged` | `{"state": "minimized" \| "maximized" \| "restored"}` | `WM_SYSCOMMAND` con `SC_MINIMIZE`, `SC_MAXIMIZE` o `SC_RESTORE` |
| `window.destroyed` | `{}` | `WM_DESTROY` |

!!! note
    Si WoW cambia de renderizador, por ejemplo entre D3D y D3D9Ex, y recrea la ventana, `WM_DESTROY` invalida el `HWND` cacheado. El siguiente método de ventana descubre el nuevo identificador automáticamente.
