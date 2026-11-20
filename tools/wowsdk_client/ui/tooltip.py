"""Small text bubbles shown while the mouse rests on a widget."""

# Standard library
from __future__ import annotations

import tkinter as tk
from typing import Any, Callable

SHOW_DELAY_MS = 300
LEAVE_CHECK_MS = 40
WATCH_MS = 150
OFFSET = (14, 18)
BACKGROUND = "#2b2b31"
FOREGROUND = "#e6e6e6"
BORDER = "#45454d"
WRAP = 320


class Tooltip:
    """
    Shows a text next to the pointer after a short delay and hides it when the pointer leaves.

    CustomTkinter widgets are made of several Tk widgets, and moving between them sends
    ``<Leave>`` and ``<Enter>``. The tooltip only hides once the pointer is really outside the
    widget, so it neither flickers nor restarts its delay.
    """

    def __init__(self, widget: Any, text: str | Callable[[], str]) -> None:
        self._widget = widget
        self._text = text
        self._window: tk.Toplevel | None = None
        self._show_job: str | None = None
        self._leave_job: str | None = None
        self._watch_job: str | None = None
        widget.bind("<Enter>", self._on_enter, add="+")
        widget.bind("<Leave>", self._on_leave, add="+")
        widget.bind("<ButtonPress>", self._hide, add="+")

    def set_text(self, text: str | Callable[[], str]) -> None:
        """Change the text shown from now on."""
        self._text = text

    def _current_text(self) -> str:
        return self._text() if callable(self._text) else self._text

    def _pointer_inside(self) -> bool:
        """Whether the pointer is over this widget and no other window covers it there."""
        try:
            x, y = self._widget.winfo_pointerxy()
            under = self._widget.winfo_containing(x, y)
        except (tk.TclError, KeyError):
            return False
        if under is None:
            return False
        return str(under) == str(self._widget) or str(under).startswith(str(self._widget) + ".")

    def _watch(self) -> None:
        if self._window is None:
            return
        if not self._pointer_inside():
            self._hide()
            return
        self._watch_job = self._widget.after(WATCH_MS, self._watch)

    def _on_enter(self, _event: Any = None) -> None:
        if self._leave_job is not None:
            self._widget.after_cancel(self._leave_job)
            self._leave_job = None
        if self._show_job is None and self._window is None:
            self._show_job = self._widget.after(SHOW_DELAY_MS, self._show)

    def _on_leave(self, _event: Any = None) -> None:
        if self._leave_job is None:
            self._leave_job = self._widget.after(LEAVE_CHECK_MS, self._check_leave)

    def _check_leave(self) -> None:
        self._leave_job = None
        if not self._pointer_inside():
            self._hide()

    def _show(self) -> None:
        self._show_job = None
        text = self._current_text()
        if not text or self._window is not None or not self._pointer_inside():
            return
        x, y = self._widget.winfo_pointerxy()
        window = tk.Toplevel(self._widget)
        window.wm_overrideredirect(True)
        window.attributes("-topmost", True)
        window.configure(background=BORDER)
        tk.Label(window, text=text, justify="left", background=BACKGROUND, foreground=FOREGROUND,
                 font=("Segoe UI", 9), padx=8, pady=4, wraplength=WRAP).pack(padx=1, pady=1)
        window.wm_geometry(f"+{x + OFFSET[0]}+{y + OFFSET[1]}")
        self._window = window
        self._watch_job = self._widget.after(WATCH_MS, self._watch)

    def _hide(self, _event: Any = None) -> None:
        for job in (self._show_job, self._leave_job, self._watch_job):
            if job is not None:
                try:
                    self._widget.after_cancel(job)
                except tk.TclError:
                    pass
        self._show_job = None
        self._leave_job = None
        self._watch_job = None
        if self._window is not None:
            self._window.destroy()
            self._window = None


def tooltip(widget: Any, text: str | Callable[[], str]) -> Tooltip:
    """Attach a tooltip to a widget and return it."""
    return Tooltip(widget, text)
