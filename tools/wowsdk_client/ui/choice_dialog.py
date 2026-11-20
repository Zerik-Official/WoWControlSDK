"""Small window that asks the user to pick one of several options."""

# Standard library
from __future__ import annotations

from typing import Any

# Third-party
import customtkinter as ctk

# Project imports
from ..i18n import tr
from . import theme
from .chrome import WindowChrome

WIDTH = 460


def choose(master: Any, title: str, message: str, options: list[tuple[str, str]]) -> str | None:
    """Show the options as buttons and return the key of the chosen one, or ``None`` when cancelled."""
    window = ctk.CTkToplevel(master)
    window.transient(master)
    chrome = WindowChrome(window, title, resizable=False, minimizable=False, taskbar=False,
                          on_close=window.destroy)
    chosen: list[str] = []

    body = chrome.body
    body.grid_columnconfigure(0, weight=1)
    ctk.CTkLabel(body, text=message, font=theme.small(13), anchor="w", justify="left",
                 wraplength=WIDTH - 40).grid(row=0, column=0, sticky="ew", padx=20, pady=(16, 12))

    def pick(key: str) -> None:
        chosen.append(key)
        window.destroy()

    for row, (key, label) in enumerate(options, start=1):
        primary = row == 1
        ctk.CTkButton(body, text=label, height=34, anchor="w",
                      fg_color=theme.ACCENT if primary else theme.SURFACE_ALT,
                      hover_color=theme.ACCENT_HOVER if primary else theme.SURFACE,
                      text_color=theme.ON_ACCENT if primary else theme.TEXT,
                      command=lambda selected=key: pick(selected)).grid(row=row, column=0, sticky="ew", padx=20,
                                                                         pady=(0, 6))
    ctk.CTkButton(body, text=tr("settings.cancel"), width=100, fg_color="transparent", hover_color=theme.SURFACE_ALT,
                  text_color=theme.MUTED, command=window.destroy).grid(row=len(options) + 1, column=0, sticky="e",
                                                                       padx=20, pady=(6, 16))
    window.bind("<Escape>", lambda _event: window.destroy())

    body.configure(width=WIDTH)
    chrome.center_on(master)
    window.lift()
    window.focus_force()
    window.grab_set()
    master.wait_window(window)
    return chosen[0] if chosen else None
