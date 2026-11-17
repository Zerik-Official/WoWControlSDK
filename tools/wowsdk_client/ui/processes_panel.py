"""List of running games and injectors, with the SDK state of each one."""

# Standard library
from __future__ import annotations

from tkinter import messagebox
from typing import Any, Callable

# Third-party
import customtkinter as ctk

# Project imports
from ..i18n import tr
from ..processes import ProcessError, TrackedProcess, list_processes, terminate
from . import theme
from .icons import icon

REFRESH_MS = 3000
WINDOW_KEYS: dict[str, str] = {
    "visible": "processes.window_visible",
    "hidden": "processes.window_hidden",
    "none": "processes.window_none",
}


class ProcessesPanel(ctk.CTkFrame):
    """Shows every Wow.exe and WowInjector.exe and closes the ones the user picks."""

    def __init__(self, master: Any, game_path: Callable[[], str], log: Callable[[str], None],
                 **kwargs: Any) -> None:
        super().__init__(master, fg_color="transparent", **kwargs)
        self.grid_columnconfigure(2, weight=1)
        self.grid_rowconfigure(2, weight=1)
        self._game_path = game_path
        self._log = log
        self._processes: list[TrackedProcess] = []

        ctk.CTkButton(self, text=tr("processes.refresh"), width=120, image=icon("refresh", theme.ON_ACCENT, 16),
                      compound="left", text_color=theme.ON_ACCENT, fg_color=theme.ACCENT,
                      hover_color=theme.ACCENT_HOVER, command=self.refresh).grid(row=0, column=0, padx=(0, 6),
                                                                                  pady=(0, 6))
        ctk.CTkButton(self, text=tr("processes.close_all"), width=170, image=icon("circle-x", "#ffffff", 16),
                      compound="left", fg_color=theme.DANGER,
                      hover_color=theme.DANGER, command=self._close_all).grid(row=0, column=1, padx=(0, 6),
                                                                              pady=(0, 6))
        self._summary = ctk.CTkLabel(self, text="", font=theme.small(), text_color=theme.MUTED, anchor="w")
        self._summary.grid(row=1, column=0, columnspan=3, sticky="ew", pady=(0, 6))

        self._list = ctk.CTkScrollableFrame(self, fg_color="transparent")
        self._list.grid(row=2, column=0, columnspan=3, sticky="nsew")
        self._list.grid_columnconfigure(0, weight=1)
        self.after(REFRESH_MS, self._auto_refresh)

    def refresh(self) -> list[TrackedProcess]:
        """Read the running processes again and redraw the list."""
        self._processes = list_processes(self._game_path())
        self._render()
        return self._processes

    def _auto_refresh(self) -> None:
        if self.winfo_ismapped():
            self.refresh()
        self.after(REFRESH_MS, self._auto_refresh)

    def _render(self) -> None:
        for child in self._list.winfo_children():
            child.destroy()

        games = [item for item in self._processes if item.kind == "game"]
        injectors = [item for item in self._processes if item.kind == "injector"]
        self._summary.configure(text=tr("processes.summary", games=len(games),
                                        sdk=sum(1 for item in games if item.sdk_loaded),
                                        injectors=len(injectors)))
        if not self._processes:
            ctk.CTkLabel(self._list, text=tr("processes.empty"), font=theme.small(13), text_color=theme.MUTED,
                         anchor="w").grid(row=0, column=0, sticky="w", pady=8)
            return

        for row, item in enumerate(self._processes):
            self._render_row(row, item)

    def _render_row(self, row: int, item: TrackedProcess) -> None:
        card = ctk.CTkFrame(self._list, fg_color=theme.SURFACE_ALT, corner_radius=8)
        card.grid(row=row, column=0, sticky="ew", pady=4)
        card.grid_columnconfigure(0, weight=1)

        kind = tr("processes.game") if item.kind == "game" else tr("processes.injector")
        ctk.CTkLabel(card, text=f"{kind}  •  {item.name}  •  PID {item.pid}", font=theme.title(13),
                     anchor="w").grid(row=0, column=0, sticky="w", padx=12, pady=(8, 0))

        badges: list[tuple[str, str]] = []
        if item.kind == "game":
            badges.append((tr("processes.sdk_loaded"), theme.SUCCESS) if item.sdk_loaded
                          else (tr("processes.sdk_missing"), theme.MUTED))
            if item.owns_pipe:
                badges.append((tr("processes.pipe_owner"), theme.ACCENT))
            badges.append((tr(WINDOW_KEYS[item.window]), theme.MUTED))
        if item.started is not None:
            badges.append((tr("processes.started", time=item.started.strftime("%H:%M:%S")), theme.MUTED))

        line = ctk.CTkFrame(card, fg_color="transparent")
        line.grid(row=1, column=0, columnspan=2, sticky="w", padx=12)
        for column, (text, color) in enumerate(badges):
            ctk.CTkLabel(line, text=text, font=theme.small(12), text_color=color).grid(row=0, column=column,
                                                                                      padx=(0, 14))
        if item.path:
            ctk.CTkLabel(card, text=item.path, font=theme.small(11), text_color=theme.MUTED, anchor="w").grid(
                row=2, column=0, columnspan=2, sticky="w", padx=12, pady=(0, 8))

        ctk.CTkButton(card, text=tr("processes.close"), width=90, height=28, fg_color=theme.DANGER,
                      hover_color=theme.DANGER, command=lambda selected=item: self._close(selected)).grid(
            row=0, column=1, padx=12, pady=(8, 0))

    def _close(self, item: TrackedProcess) -> None:
        if not messagebox.askyesno(tr("app.title"), tr("processes.confirm_close", name=item.name, pid=item.pid),
                                   parent=self):
            return
        self.close_processes([item])

    def _close_all(self) -> None:
        self.refresh()
        if not self._processes:
            return
        if not messagebox.askyesno(tr("app.title"), tr("processes.confirm_close_all", count=len(self._processes)),
                                   parent=self):
            return
        self.close_processes(self._processes)

    def close_processes(self, items: list[TrackedProcess]) -> None:
        """Close processes without asking, injectors first so they cannot start a new game."""
        for item in sorted(items, key=lambda entry: entry.kind != "injector"):
            try:
                terminate(item.pid)
            except ProcessError as exc:
                self._log(tr("processes.close_failed", pid=item.pid, error=exc))
                continue
            self._log(tr("processes.closed", name=item.name, pid=item.pid))
        self.after(300, self.refresh)
