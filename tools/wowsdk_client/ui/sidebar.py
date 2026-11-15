"""Searchable list of domains and methods."""

# Standard library
from __future__ import annotations

from typing import Any, Callable

# Third-party
import customtkinter as ctk

# Project imports
from ..catalog import DOMAINS, Method
from . import theme


class Sidebar(ctk.CTkFrame):
    """Lists every catalog method grouped by domain, filtered by a search box."""

    def __init__(self, master: Any, on_select: Callable[[Method], None], **kwargs: Any) -> None:
        super().__init__(master, width=theme.SIDEBAR_WIDTH, corner_radius=0, fg_color=theme.SURFACE, **kwargs)
        self.grid_propagate(False)
        self.grid_rowconfigure(1, weight=1)
        self.grid_columnconfigure(0, weight=1)
        self._on_select = on_select
        self._buttons: dict[str, ctk.CTkButton] = {}
        self._headers: dict[str, ctk.CTkLabel] = {}
        self._selected: str | None = None

        self._search = ctk.CTkEntry(self, placeholder_text="Search methods")
        self._search.grid(row=0, column=0, sticky="ew", padx=12, pady=(12, 8))
        self._search.bind("<KeyRelease>", lambda _event: self._filter())

        self._list = ctk.CTkScrollableFrame(self, fg_color="transparent")
        self._list.grid(row=1, column=0, sticky="nsew", padx=(4, 0), pady=(0, 8))
        self._list.grid_columnconfigure(0, weight=1)
        self._build()

    def _build(self) -> None:
        row = 0
        for domain in DOMAINS:
            header = ctk.CTkLabel(self._list, text=domain.title.upper(), font=theme.title(11),
                                  text_color=theme.MUTED, anchor="w")
            header.grid(row=row, column=0, sticky="ew", padx=8, pady=(10, 2))
            self._headers[domain.name] = header
            row += 1
            for method in domain.methods:
                button = ctk.CTkButton(
                    self._list, text=method.name.split(".", 1)[1], anchor="w", height=26,
                    fg_color="transparent", hover_color=theme.SURFACE_ALT, text_color=("#1a1a1a", "#e6e6e6"),
                    command=lambda selected=method: self.select(selected),
                )
                button.grid(row=row, column=0, sticky="ew", padx=4, pady=1)
                self._buttons[method.name] = button
                row += 1

    def select(self, method: Method) -> None:
        """Highlight a method and notify the owner."""
        if self._selected and self._selected in self._buttons:
            self._buttons[self._selected].configure(fg_color="transparent")
        self._selected = method.name
        self._buttons[method.name].configure(fg_color=theme.ACCENT_HOVER)
        self._on_select(method)

    def _filter(self) -> None:
        query = self._search.get().strip().lower()
        for domain in DOMAINS:
            visible = 0
            for method in domain.methods:
                button = self._buttons[method.name]
                match = not query or query in method.name.lower() or query in method.description.lower()
                if match:
                    button.grid()
                    visible += 1
                else:
                    button.grid_remove()
            if visible:
                self._headers[domain.name].grid()
            else:
                self._headers[domain.name].grid_remove()
