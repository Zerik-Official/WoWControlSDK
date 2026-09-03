# window.* Methods

Manages the main World of Warcraft window — visibility, position, size, focus, and visual effects.

---

The SDK locates the window via `FindWindowA` over the known internal classes (`GxWindowClassD3d`, `GxWindowClassD3d9Ex`, `GxWindowClassOpenGl`). All operations use native Win32 directly on the `HWND`.

> [!NOTE]
> The `HWND` is a Windows window handle. The SDK returns it as an integer (`uintptr_t`) for interoperability with other languages and libraries.

---

## `window.getHandle`

Returns the `HWND` of the main WoW window as an integer.

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `handle` | integer \| null | `HWND` as `uintptr_t`, or `null` if not found |

---

## `window.getInfo`

Returns complete window information in a single call.

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `title` | string | window title |
| `className` | string | Win32 class name (e.g. `GxWindowClassD3d`) |
| `visible` | boolean | `true` if not hidden |
| `minimized` | boolean | `true` if minimized |
| `maximized` | boolean | `true` if maximized |
| `topMost` | boolean | `true` if `WS_EX_TOPMOST` is set |
| `bounds.x` | integer | X coordinate of top-left corner |
| `bounds.y` | integer | Y coordinate of top-left corner |
| `bounds.width` | integer | width in pixels |
| `bounds.height` | integer | height in pixels |

---

## `window.show`

Shows the window (equivalent to `ShowWindow(SW_SHOW)`).

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `ok` | boolean | always `true` |

---

## `window.hide`

Hides the window (equivalent to `ShowWindow(SW_HIDE)`). The game process keeps running at full speed — does not throttle FPS like minimizing does.

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `ok` | boolean | always `true` |

---

## `window.minimize`

Minimizes the window (equivalent to `ShowWindow(SW_MINIMIZE)`).

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `ok` | boolean | always `true` |

> [!NOTE]
> When minimized, WoW reduces background processing. To limit background FPS without minimizing, use CVar `maxFPSBk` via `lua.setCVar("maxFPSBk", 30)`.

---

## `window.restore`

Restores the window from minimized or maximized state (equivalent to `ShowWindow(SW_RESTORE)`).

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `ok` | boolean | always `true` |

---

## `window.maximize`

Maximizes the window (equivalent to `ShowWindow(SW_MAXIMIZE)`).

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `ok` | boolean | always `true` |

---

## `window.focus`

Brings the window to front and gives it focus (calls `SetForegroundWindow`; restores first if minimized).

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `ok` | boolean | always `true` |

---

## `window.getPosition`

Gets the window's top-left corner position.

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `x` | integer | X coordinate |
| `y` | integer | Y coordinate |

---

## `window.setPosition`

Moves the window without changing its size.

### Parameters

| Name | Type | Required | Description |
|---|---|---|---|
| `x` | integer | yes | new X coordinate |
| `y` | integer | yes | new Y coordinate |

### Response

| Field | Type | Description |
|---|---|---|
| `ok` | boolean | always `true` |

---

## `window.getSize`

Gets the window dimensions (including borders and title bar).

### Parameters

None.

### Response

| Field | Type | Description |
|---|---|---|
| `width` | integer | width in pixels |
| `height` | integer | height in pixels |

---

## `window.setSize`

Changes the window size without moving it.

### Parameters

| Name | Type | Required | Description |
|---|---|---|---|
| `width` | integer | yes | new width in pixels |
| `height` | integer | yes | new height in pixels |

### Response

| Field | Type | Description |
|---|---|---|
| `ok` | boolean | always `true` |

> [!NOTE]
> Done via `SetWindowPos(SWP_NOMOVE)` — WoW receives `WM_SIZE` and adjusts its renderer automatically. The aspect-ratio lock (`gxAspect` / `windowResizeLock`) only applies during interactive resize (`WM_SIZING`), not programmatic.

---

## `window.setBounds`

Changes position and size in one call (partial: omits any field not provided).

### Parameters

| Name | Type | Required | Description |
|---|---|---|---|
| `x` | integer | no | new X coordinate |
| `y` | integer | no | new Y coordinate |
| `width` | integer | no | new width in pixels |
| `height` | integer | no | new height in pixels |

### Response

| Field | Type | Description |
|---|---|---|
| `ok` | boolean | always `true` |

> [!NOTE]
> Clears cursor clipping (`ClipCursor(NULL)`) before moving, since WoW clips the cursor to the window rect after format changes (`DeviceSetFormat`).

---

## `window.setTitle`

Changes the window title.

### Parameters

| Name | Type | Required | Description |
|---|---|---|---|
| `title` | string | yes | new title |

### Response

| Field | Type | Description |
|---|---|---|
| `ok` | boolean | always `true` |

---

## `window.setTopMost`

Toggles the `WS_EX_TOPMOST` style (window stays above all others).

### Parameters

| Name | Type | Required | Description |
|---|---|---|---|
| `topMost` | boolean | yes | `true` = always on top, `false` = normal |

### Response

| Field | Type | Description |
|---|---|---|
| `ok` | boolean | always `true` |

---

## `window.setOpacity`

Sets window opacity (0 = invisible, 100 = opaque). Uses `WS_EX_LAYERED` + `SetLayeredWindowAttributes`.

### Parameters

| Name | Type | Required | Default | Description |
|---|---|---|---|---|
| `percent` | integer | no | 100 | opacity 0–100 |

### Response

| Field | Type | Description |
|---|---|---|
| `ok` | boolean | always `true` |

> [!NOTE]
> Values < 100 enable `WS_EX_LAYERED`; 100 disables it restoring original style. Does not work in D3D exclusive fullscreen mode.

---

## `window.flashTaskbar`

Flashes the taskbar button (Discord-style notification). Uses `FlashWindowEx(FLASHW_TRAY | FLASHW_TIMERNOFG)`.

### Parameters

| Name | Type | Required | Default | Description |
|---|---|---|---|---|
| `count` | integer | no | 0 (infinite) | number of flashes; 0 = until window receives focus |
| `timeoutMs` | integer | no | 0 (system default) | interval between flashes in ms |

### Response

| Field | Type | Description |
|---|---|---|
| `ok` | boolean | always `true` |

---

## `window.setIcon`

Changes the window icon (title bar `ICON_SMALL` 16×16 and/or Alt+Tab `ICON_BIG` 32×32). Accepts an `.ico` file which may contain multiple resolutions; the SDK selects the most appropriate representation within the container.

### Parameters

| Name | Type | Required | Description |
|---|---|---|---|
| `path` | string | yes* | absolute path to the `.ico` file |
| `type` | string | no | `"small"` (16×16 only), `"big"` (32×32 only), omitted = both |
| `restore` | boolean | yes* | `true` to restore WoW's original icon |

* `path` and `restore` are mutually exclusive; exactly one must be provided.

### Response

| Field | Type | Description |
|---|---|---|
| `ok` | boolean | `true` if the operation succeeded |

> [!NOTE]
> When loading an `.ico` without `type`, the SDK extracts the best 16×16 representation for `ICON_SMALL` and 32×32 for `ICON_BIG`. Windows automatically selects the most appropriate representation within the `.ico` container based on DPI and context.
>
> The SDK saves WoW's original icon **only on the first modification**. Subsequent `setIcon` calls do not overwrite the backup, allowing restoration to the original state at any time.
>
> Custom icons loaded via `LoadImage` are automatically freed (`DestroyIcon`) when changing icons or unloading the SDK. WoW's original icon is **never destroyed** (owned by the application).
>
> **The taskbar icon is probably taken from the executable's resources (Wow.exe); `window.setIcon` only affects titlebar and Alt+Tab.**

---

## Window Events

The service subclasses `GWLP_WNDPROC` and emits these events to the event pipe (`\\.\pipe\WowGameEvent`):

| Event | Payload | Description |
|---|---|---|
| `window.resized` | `{width, height}` | after `WM_SIZE` |
| `window.moved` | `{x, y}` | after `WM_MOVE` |
| `window.focusChanged` | `{focused: boolean}` | `WM_ACTIVATE` / `WM_SETFOCUS` / `WM_KILLFOCUS` |
| `window.stateChanged` | `{state: "minimized"\|"maximized"\|"restored"}` | `WM_SYSCOMMAND` SC_MINIMIZE / SC_RESTORE / SC_MAXIMIZE |
| `window.destroyed` | `{}` | `WM_DESTROY` — HWND cache invalidated; next call re-discovers |

> [!NOTE]
> If WoW switches renderer (e.g. D3D ↔ D3D9Ex) and recreates the window, `WM_DESTROY` invalidates the cache and the next call to any method re-discovers the `HWND` automatically.