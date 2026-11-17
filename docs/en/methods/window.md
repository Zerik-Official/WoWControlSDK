# window.* methods

Manage the main World of Warcraft window: visibility, position, size, focus, transparency, title, icon and taskbar notifications.

The SDK locates the window with `FindWindowA` over the known internal classes (`GxWindowClassD3d`, `GxWindowClassD3d9Ex` and `GxWindowClassOpenGl`). Every operation uses native Win32 calls on the `HWND`.

!!! note
    `HWND` is a Windows window handle. The SDK returns it as an unsigned integer so other languages and libraries can use it.

## window.getHandle

Returns the `HWND` of the main window.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `handle` | integer or null | `HWND` as an unsigned integer, or `null` when the window is not found |

## window.getInfo

Returns the complete window information in one call.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `title` | string | Window title |
| `className` | string | Win32 class name, such as `GxWindowClassD3d` |
| `visible` | boolean | Whether the window is visible |
| `minimized` | boolean | Whether the window is minimized |
| `maximized` | boolean | Whether the window is maximized |
| `topMost` | boolean | Whether `WS_EX_TOPMOST` is set |
| `startHidden` | boolean | Whether the window is still kept hidden by [`window.setStartHidden`](#windowsetstarthidden) |
| `bounds.x` | integer | X coordinate of the top-left corner |
| `bounds.y` | integer | Y coordinate of the top-left corner |
| `bounds.width` | integer | Width in pixels |
| `bounds.height` | integer | Height in pixels |

When the window is not found, the response is `{"error": "Window not found"}`.

## window.show

Shows the window, equivalent to `ShowWindow(SW_SHOW)`. It also ends the hidden start of [`window.setStartHidden`](#windowsetstarthidden), like `window.restore`, `window.maximize` and `window.focus`.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | Always `true` |

## window.hide

Hides the window, equivalent to `ShowWindow(SW_HIDE)`. The game keeps running at full speed and does not throttle FPS the way minimizing does.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | Always `true` |

## window.setStartHidden

Keeps the game window hidden from the moment the client creates it, for headless launches. While active, the SDK removes `WS_VISIBLE` when the game window is created and turns every attempt of the client to show it (`ShowWindow`, `SetWindowPos` with `SWP_SHOWWINDOW`) into a hidden window. It only affects the `GxWindowClass*` game window, not other windows such as the SDK console.

Call it before the game creates its window: [`WowInjector.exe --hidden`](../injection.md#arguments) sends it while the game is still frozen. `window.show`, `window.restore`, `window.maximize` and `window.focus` end the hidden start and show the window, for example once the character is in the world. When the hidden start ends, the SDK makes the window one pixel taller for two frames and then restores it, so the game rebuilds its back buffer at the real size instead of showing a blurry image.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `hidden` | boolean | Yes | `true` to keep the window hidden, `false` to stop (the window stays as it is until shown) |

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | Always `true` |
| `startHidden` | boolean | The new state |

### Example

```json
{"jsonrpc": "2.0", "id": 1, "method": "window.setStartHidden", "params": {"hidden": true}}
```

```json
{"jsonrpc": "2.0", "id": 1, "result": {"ok": true, "startHidden": true}}
```

### Errors

| Condition | Response |
| --- | --- |
| `hidden` is missing or not a boolean | `{"error": "hidden must be a boolean"}` |

!!! note
    A hidden window has no taskbar button. The game keeps running and loading normally; fullscreen mode may not start correctly while the window is hidden, so windowed mode is recommended for headless launches.

## window.minimize

Minimizes the window, equivalent to `ShowWindow(SW_MINIMIZE)`.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | Always `true` |

!!! note
    WoW reduces background processing when the window is minimized. To limit background FPS without minimizing, set the `maxFPSBk` CVar through `lua.setCVar` or `lua.execute`.

## window.restore

Restores the window from the minimized or maximized state, equivalent to `ShowWindow(SW_RESTORE)`.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | Always `true` |

## window.maximize

Maximizes the window, equivalent to `ShowWindow(SW_MAXIMIZE)`.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | Always `true` |

## window.focus

Brings the window to the foreground and gives it focus. If the window is minimized, it is restored first.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | Always `true` |

## window.getPosition

Returns the position of the top-left corner.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `x` | integer | X coordinate |
| `y` | integer | Y coordinate |

When the window is not found, the response is `{"error": "Window not found"}`.

## window.setPosition

Moves the window without changing its size.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `x` | integer | Yes | New X coordinate |
| `y` | integer | Yes | New Y coordinate |

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | Always `true` |

## window.getSize

Returns the window dimensions, including borders and title bar.

### Parameters

None.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `width` | integer | Width in pixels |
| `height` | integer | Height in pixels |

When the window is not found, the response is `{"error": "Window not found"}`.

## window.setSize

Changes the window size without moving it.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `width` | integer | Yes | New width in pixels |
| `height` | integer | Yes | New height in pixels |

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | Always `true` |

!!! note
    The call uses `SetWindowPos(SWP_NOMOVE)`. WoW receives `WM_SIZE` and adjusts its renderer automatically. The aspect-ratio lock (`gxAspect` / `windowResizeLock`) only applies to interactive resizing, not to this call.

## window.setBounds

Changes position and size in one call.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `x` | integer | Yes | New X coordinate |
| `y` | integer | Yes | New Y coordinate |
| `width` | integer | Yes | New width in pixels |
| `height` | integer | Yes | New height in pixels |

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | Always `true` |

!!! warning
    Omitted values default to `0`. Provide all four fields. To change only the position or only the size, use `window.setPosition` or `window.setSize`.

!!! note
    The call clears cursor clipping with `ClipCursor(NULL)` before moving the window, because WoW clips the cursor to the window rectangle after a format change (`DeviceSetFormat`).

## window.setTitle

Changes the window title.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `title` | string | Yes | New title |

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | Always `true` |

## window.setTopMost

Toggles the `WS_EX_TOPMOST` style, which keeps the window above all others.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `topMost` | boolean | Yes | `true` for always on top, `false` for normal behavior |

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | Always `true` |

## window.setOpacity

Sets the window opacity.

### Parameters

| Name | Type | Required | Default | Description |
| --- | --- | --- | --- | --- |
| `percent` | integer | No | `100` | Opacity from `0` (invisible) to `100` (opaque) |

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | Always `true` |

!!! note
    Values below `100` enable `WS_EX_LAYERED` and use `SetLayeredWindowAttributes`. A value of `100` removes the layered style. Opacity does not work in D3D exclusive fullscreen mode.

## window.flashTaskbar

Flashes the taskbar button, similar to a Discord notification. It uses `FlashWindowEx(FLASHW_TRAY | FLASHW_TIMERNOFG)`.

### Parameters

| Name | Type | Required | Default | Description |
| --- | --- | --- | --- | --- |
| `count` | integer | No | `0` | Number of flashes. `0` flashes until the window receives focus |
| `timeoutMs` | integer | No | `0` | Interval between flashes in milliseconds. `0` uses the system default |

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | Always `true` |

## window.setIcon

Changes the window icon: the title bar icon (`ICON_SMALL`, 16×16) and the Alt+Tab icon (`ICON_BIG`, 32×32). The `.ico` file may contain multiple resolutions; the SDK selects the appropriate representation.

### Parameters

| Name | Type | Required | Description |
| --- | --- | --- | --- |
| `path` | string | One of both | Absolute path to the `.ico` file |
| `type` | string | No | `"small"` for 16×16 only, `"big"` for 32×32 only. Omit it to set both |
| `restore` | boolean | One of both | `true` to restore WoW's original icon |

`path` and `restore` are mutually exclusive. Exactly one must be provided.

### Response

| Field | Type | Description |
| --- | --- | --- |
| `ok` | boolean | `true` when the parameters were accepted |

### Errors

| Condition | Error |
| --- | --- |
| Both `path` and `restore` are provided | `Cannot specify both 'path' and 'restore'` |
| Neither is provided | `Missing required parameter: 'path' or 'restore'` |
| `path` is empty | `Parameter 'path' cannot be empty` |
| `type` is not `small` or `big` | `Invalid 'type' value. Expected 'small' or 'big'` |

!!! note
    When an `.ico` is loaded without `type`, the SDK extracts the best 16×16 representation for `ICON_SMALL` and 32×32 for `ICON_BIG`. Windows selects the best representation inside the container for the current DPI and context.

    WoW's original icon is saved only on the first modification. Later calls do not overwrite the backup, so `restore` can return to the original icon at any time. Custom icons are freed with `DestroyIcon` when the icon changes or the SDK unloads; the original icon belongs to the application and is never destroyed.

!!! warning
    The taskbar icon is usually taken from the `Wow.exe` resources. `window.setIcon` affects the title bar and Alt+Tab, not the taskbar button.

## Window events

The window service subclasses `GWLP_WNDPROC` and emits these events to the event pipe:

| Event | Payload | Trigger |
| --- | --- | --- |
| `window.resized` | `{"width": <int>, "height": <int>}` | After `WM_SIZE` |
| `window.moved` | `{"x": <int>, "y": <int>}` | After `WM_MOVE` |
| `window.focusChanged` | `{"focused": <boolean>}` | `WM_ACTIVATE`, `WM_SETFOCUS` or `WM_KILLFOCUS` |
| `window.stateChanged` | `{"state": "minimized" \| "maximized" \| "restored"}` | `WM_SYSCOMMAND` with `SC_MINIMIZE`, `SC_MAXIMIZE` or `SC_RESTORE` |
| `window.destroyed` | `{}` | `WM_DESTROY` |

!!! note
    If WoW switches renderer, for example between D3D and D3D9Ex, and recreates the window, `WM_DESTROY` invalidates the cached `HWND`. The next window method discovers the new handle automatically.
