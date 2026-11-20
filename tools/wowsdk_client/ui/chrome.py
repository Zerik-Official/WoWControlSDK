"""
Frameless windows with the client's own title bar.

The native title bar of Windows 10 cannot change its color, so every window of the client
hides it and draws its own: it moves the window, minimizes, maximizes to the work area of
its monitor, resizes from the edges and keeps the window on the taskbar.
"""

# Standard library
from __future__ import annotations

import ctypes
import tkinter as tk
from ctypes import wintypes
from typing import Any, Callable

# Third-party
import customtkinter as ctk

# Project imports
from . import theme
from .icons import icon

GWL_EXSTYLE = -20
WS_EX_APPWINDOW = 0x00040000
WS_EX_TOOLWINDOW = 0x00000080
WS_EX_NOACTIVATE = 0x08000000
SW_MINIMIZE = 6
SWP_NOSIZE = 0x0001
SWP_NOMOVE = 0x0002
SWP_NOZORDER = 0x0004
SWP_NOACTIVATE = 0x0010
SWP_FRAMECHANGED = 0x0020
MONITOR_DEFAULTTONEAREST = 2

BAR_HEIGHT = 40
RESIZE_BORDER = 6
GLYPH_FONT = ("Segoe MDL2 Assets", 10)
GLYPH_MINIMIZE = ""
GLYPH_MAXIMIZE = ""
GLYPH_RESTORE = ""
GLYPH_CLOSE = ""
CLOSE_HOVER = "#c42b1c"
EDGE_CURSORS: dict[str, str] = {
    "n": "sb_v_double_arrow", "s": "sb_v_double_arrow",
    "e": "sb_h_double_arrow", "w": "sb_h_double_arrow",
    "nw": "size_nw_se", "se": "size_nw_se", "ne": "size_ne_sw", "sw": "size_ne_sw",
}

_user32 = ctypes.windll.user32


class _MonitorInfo(ctypes.Structure):
    _fields_ = [("cbSize", wintypes.DWORD), ("rcMonitor", wintypes.RECT), ("rcWork", wintypes.RECT),
                ("dwFlags", wintypes.DWORD)]


def _hwnd(window: Any) -> int:
    return _user32.GetParent(window.winfo_id()) or window.winfo_id()


def _rect(window: Any) -> tuple[int, int, int, int]:
    rect = wintypes.RECT()
    _user32.GetWindowRect(_hwnd(window), ctypes.byref(rect))
    return rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top


def _move(window: Any, x: int, y: int, width: int | None = None, height: int | None = None) -> None:
    """Move or resize the window in screen pixels and keep Tk from shrinking it back to its content."""
    position = f"+{int(x)}+{int(y)}"
    size = "" if width is None or height is None else f"{int(width)}x{int(height)}"
    tk.Wm.wm_geometry(window, size + position)


def _work_area(window: Any) -> tuple[int, int, int, int]:
    monitor = _user32.MonitorFromWindow(_hwnd(window), MONITOR_DEFAULTTONEAREST)
    info = _MonitorInfo()
    info.cbSize = ctypes.sizeof(_MonitorInfo)
    _user32.GetMonitorInfoW(monitor, ctypes.byref(info))
    work = info.rcWork
    return work.left, work.top, work.right - work.left, work.bottom - work.top


def _scaling(window: Any) -> float:
    try:
        return float(ctk.ScalingTracker.get_window_scaling(window))
    except (AttributeError, KeyError, tk.TclError):
        return 1.0


class WindowChrome:
    """Replaces the native frame of a window with the client's title bar and edges."""

    def __init__(self, window: Any, title: str, *, resizable: bool = True, minimizable: bool = True,
                 on_close: Callable[[], None] | None = None, min_size: tuple[int, int] = (480, 320),
                 taskbar: bool = True) -> None:
        self.window = window
        self._resizable = resizable
        self._min_size = min_size
        self._on_close = on_close or window.destroy
        self._maximized = False
        self._restore_rect: tuple[int, int, int, int] | None = None
        self._drag: tuple[int, int, int, int] | None = None
        self._resize: tuple[str, int, int, tuple[int, int, int, int]] | None = None
        self._edge = ""
        self._max_button: ctk.CTkButton | None = None

        window.overrideredirect(True)
        window.configure(fg_color=theme.WINDOW_BORDER)
        window.grid_rowconfigure(1, weight=1)
        window.grid_columnconfigure(0, weight=1)

        self.bar = ctk.CTkFrame(window, height=BAR_HEIGHT, corner_radius=0, fg_color=theme.TITLEBAR)
        self.bar.grid(row=0, column=0, sticky="ew", padx=1, pady=(1, 0))
        self.bar.grid_propagate(False)
        self.bar.grid_rowconfigure(0, weight=1)
        self.bar.grid_columnconfigure(3, weight=1)

        self._icon = ctk.CTkLabel(self.bar, text="", image=icon("cpu", theme.ACCENT, 18), width=24)
        self._icon.grid(row=0, column=0, padx=(12, 6))
        self.title = ctk.CTkLabel(self.bar, text=title, font=theme.title(13), anchor="w")
        self.title.grid(row=0, column=1, sticky="w")
        self.left = ctk.CTkFrame(self.bar, fg_color="transparent")
        self.left.grid(row=0, column=2, sticky="w", padx=(14, 0))
        self.right = ctk.CTkFrame(self.bar, fg_color="transparent")
        self.right.grid(row=0, column=4, sticky="e", padx=(0, 8))

        buttons = ctk.CTkFrame(self.bar, fg_color="transparent")
        buttons.grid(row=0, column=5, sticky="nse")
        column = 0
        if minimizable:
            self._button(buttons, column, GLYPH_MINIMIZE, theme.SURFACE, self.minimize)
            column += 1
        if resizable:
            self._max_button = self._button(buttons, column, GLYPH_MAXIMIZE, theme.SURFACE, self.toggle_maximize)
            column += 1
        self._button(buttons, column, GLYPH_CLOSE, CLOSE_HOVER, self._on_close, close=True)

        self.body = ctk.CTkFrame(window, corner_radius=0, fg_color=theme.WINDOW_BACKGROUND)
        self.body.grid(row=1, column=0, sticky="nsew", padx=1, pady=(0, 1))

        for widget in (self.bar, self.title, self._icon, self.left):
            self.make_draggable(widget)
        if resizable:
            window.bind("<Motion>", self._track_edge, add="+")
            window.bind("<ButtonPress-1>", self._start_resize, add="+")
            window.bind("<B1-Motion>", self._do_resize, add="+")
            window.bind("<ButtonRelease-1>", self._end_resize, add="+")
        window.bind("<ButtonPress-1>", self._activate, add="+")
        if taskbar:
            window.after(10, self._show_in_taskbar)
        else:
            window.after(10, self._make_activatable)

    def _button(self, master: Any, column: int, glyph: str, hover: Any, command: Callable[[], None],
                close: bool = False) -> ctk.CTkButton:
        button = ctk.CTkButton(master, text=glyph, font=GLYPH_FONT, width=46, height=BAR_HEIGHT, corner_radius=0,
                               fg_color="transparent", hover_color=hover, text_color=theme.TEXT, command=command)
        button.grid(row=0, column=column, sticky="ns")
        if close:
            button.bind("<Enter>", lambda _event: button.configure(text_color="#ffffff"), add="+")
            button.bind("<Leave>", lambda _event: button.configure(text_color=theme.TEXT), add="+")
        return button

    def make_draggable(self, widget: Any) -> None:
        """Let the window be moved by dragging this widget."""
        widget.bind("<ButtonPress-1>", self._start_move, add="+")
        widget.bind("<B1-Motion>", self._do_move, add="+")
        widget.bind("<ButtonRelease-1>", lambda _event: setattr(self, "_drag", None), add="+")
        if self._resizable:
            widget.bind("<Double-Button-1>", lambda _event: self.toggle_maximize(), add="+")

    def _show_in_taskbar(self) -> None:
        hwnd = _hwnd(self.window)
        style = _user32.GetWindowLongW(hwnd, GWL_EXSTYLE)
        _user32.SetWindowLongW(hwnd, GWL_EXSTYLE, (style & ~(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE)) | WS_EX_APPWINDOW)
        self.window.wm_withdraw()
        self.window.after(10, self.window.wm_deiconify)

    def _make_activatable(self) -> None:
        """Let a dialog without a native frame take the keyboard focus, without a taskbar button."""
        if not self.window.winfo_exists():
            return
        hwnd = _hwnd(self.window)
        style = _user32.GetWindowLongW(hwnd, GWL_EXSTYLE)
        _user32.SetWindowLongW(hwnd, GWL_EXSTYLE, style & ~(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE))
        _user32.SetWindowPos(hwnd, 0, 0, 0, 0, 0,
                             SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED)

    def _activate(self, event: Any) -> None:
        """Bring the window to the front on a click so its fields receive the keyboard."""
        hwnd = _hwnd(self.window)
        if _user32.GetForegroundWindow() != hwnd:
            _user32.SetForegroundWindow(hwnd)
        widget = event.widget
        if isinstance(widget, (tk.Entry, tk.Text)):
            widget.focus_set()

    def center_on(self, master: Any | None = None, size: tuple[int, int] | None = None) -> None:
        """Place the window in the middle of another window, or of the monitor's work area."""
        self.window.update_idletasks()
        scale = _scaling(self.window)
        if size is not None:
            width, height = round(size[0] * scale), round(size[1] * scale)
        else:
            width, height = self.window.winfo_reqwidth(), self.window.winfo_reqheight()
        if master is not None:
            mx, my, mw, mh = _rect(master)
        else:
            mx, my, mw, mh = _work_area(self.window)
        wx, wy, ww, wh = _work_area(master if master is not None else self.window)
        x = min(max(mx + (mw - width) // 2, wx), wx + max(ww - width, 0))
        y = min(max(my + (mh - height) // 2, wy), wy + max(wh - height, 0))
        _move(self.window, x, y, width, height)

    def minimize(self) -> None:
        """Minimize to the taskbar."""
        _user32.ShowWindow(_hwnd(self.window), SW_MINIMIZE)

    def toggle_maximize(self) -> None:
        """Fill the work area of the monitor, or go back to the previous size."""
        if not self._resizable:
            return
        if self._maximized and self._restore_rect is not None:
            _move(self.window, *self._restore_rect)
            self._maximized = False
        else:
            self._restore_rect = _rect(self.window)
            _move(self.window, *_work_area(self.window))
            self._maximized = True
        if self._max_button is not None:
            self._max_button.configure(text=GLYPH_RESTORE if self._maximized else GLYPH_MAXIMIZE)

    def maximize(self) -> None:
        """Fill the work area of the monitor."""
        if not self._maximized:
            self.toggle_maximize()

    def _start_move(self, event: Any) -> None:
        if self._edge:
            return
        x, y, width, height = _rect(self.window)
        if self._maximized and self._restore_rect is not None:
            _, _, restore_width, restore_height = self._restore_rect
            ratio = (event.x_root - x) / max(width, 1)
            x = event.x_root - int(restore_width * ratio)
            y = event.y_root - BAR_HEIGHT // 2
            _move(self.window, x, y, restore_width, restore_height)
            self._maximized = False
            if self._max_button is not None:
                self._max_button.configure(text=GLYPH_MAXIMIZE)
        self._drag = (event.x_root, event.y_root, x, y)

    def _do_move(self, event: Any) -> None:
        if self._drag is None or self._resize is not None:
            return
        start_x, start_y, x, y = self._drag
        _move(self.window, x + event.x_root - start_x, y + event.y_root - start_y)

    def _edge_at(self, x_root: int, y_root: int) -> str:
        if self._maximized:
            return ""
        x, y, width, height = _rect(self.window)
        edge = ""
        if y_root - y < RESIZE_BORDER:
            edge = "n"
        elif y + height - y_root <= RESIZE_BORDER:
            edge = "s"
        if x_root - x < RESIZE_BORDER:
            edge += "w"
        elif x + width - x_root <= RESIZE_BORDER:
            edge += "e"
        return edge

    def _track_edge(self, event: Any) -> None:
        if self._resize is not None:
            return
        edge = self._edge_at(event.x_root, event.y_root)
        if edge != self._edge:
            self._edge = edge
            self.window.configure(cursor=EDGE_CURSORS.get(edge, ""))

    def _start_resize(self, event: Any) -> None:
        edge = self._edge_at(event.x_root, event.y_root)
        if edge:
            self._drag = None
            self._resize = (edge, event.x_root, event.y_root, _rect(self.window))

    def _do_resize(self, event: Any) -> None:
        if self._resize is None:
            return
        edge, start_x, start_y, (x, y, width, height) = self._resize
        scale = _scaling(self.window)
        min_width, min_height = round(self._min_size[0] * scale), round(self._min_size[1] * scale)
        dx, dy = event.x_root - start_x, event.y_root - start_y
        if "e" in edge:
            width = max(width + dx, min_width)
        if "s" in edge:
            height = max(height + dy, min_height)
        if "w" in edge:
            new_width = max(width - dx, min_width)
            x += width - new_width
            width = new_width
        if "n" in edge:
            new_height = max(height - dy, min_height)
            y += height - new_height
            height = new_height
        _move(self.window, x, y, width, height)

    def _end_resize(self, _event: Any) -> None:
        self._resize = None
