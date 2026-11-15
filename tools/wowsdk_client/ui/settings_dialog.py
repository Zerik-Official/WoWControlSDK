"""Dialog to edit the client settings and the session token."""

# Standard library
from __future__ import annotations

from tkinter import filedialog
from typing import Any, Callable

# Third-party
import customtkinter as ctk

# Project imports
from ..settings import Settings, default_binary
from . import theme


class PathRow:
    """Label, entry and browse button for a file path."""

    def __init__(self, master: Any, row: int, label: str, value: str, placeholder: str,
                 file_types: list[tuple[str, str]]) -> None:
        ctk.CTkLabel(master, text=label, anchor="w").grid(row=row, column=0, sticky="w", padx=(0, 10), pady=4)
        self.entry = ctk.CTkEntry(master, width=380, placeholder_text=placeholder)
        if value:
            self.entry.insert(0, value)
        self.entry.grid(row=row, column=1, sticky="ew", pady=4)
        ctk.CTkButton(master, text="Browse", width=80, fg_color=theme.SURFACE_ALT, hover_color=theme.SURFACE,
                      text_color=("#1a1a1a", "#e6e6e6"),
                      command=lambda: self._browse(file_types)).grid(row=row, column=2, padx=(8, 0), pady=4)

    def _browse(self, file_types: list[tuple[str, str]]) -> None:
        path = filedialog.askopenfilename(filetypes=file_types)
        if path:
            self.entry.delete(0, "end")
            self.entry.insert(0, path.replace("/", "\\"))

    def value(self) -> str:
        """Entered path."""
        return self.entry.get().strip()


class SettingsDialog(ctk.CTkToplevel):
    """Edits paths, launch options and the token used for requests."""

    def __init__(self, master: Any, settings: Settings, token: str | None,
                 on_save: Callable[[Settings, str | None], None]) -> None:
        super().__init__(master)
        self.title("Settings")
        self.resizable(False, False)
        self.transient(master)
        self._settings = settings
        self._on_save = on_save

        body = ctk.CTkFrame(self, fg_color="transparent")
        body.pack(fill="both", expand=True, padx=20, pady=20)
        body.grid_columnconfigure(0, minsize=150)
        body.grid_columnconfigure(1, weight=1)

        ctk.CTkLabel(body, text="Game and SDK", font=theme.title(15), anchor="w").grid(
            row=0, column=0, columnspan=3, sticky="w", pady=(0, 6))
        self._wow = PathRow(body, 1, "Wow.exe", settings.wow_path, "C:\\World of Warcraft\\Wow.exe",
                            [("Wow.exe", "Wow.exe"), ("Executables", "*.exe")])
        self._dll = PathRow(body, 2, "SDK DLL", settings.dll_path,
                            default_binary("WoWControlSDK.dll") or "WoWControlSDK.dll",
                            [("DLL", "*.dll")])
        self._injector = PathRow(body, 3, "Injector", settings.injector_path,
                                 default_binary("WowInjector.exe") or "WowInjector.exe",
                                 [("Executables", "*.exe")])

        ctk.CTkLabel(body, text="Launch options", font=theme.title(15), anchor="w").grid(
            row=4, column=0, columnspan=3, sticky="w", pady=(16, 6))
        self._use_auth = self._switch(body, 5, "Use a session token (recommended)", settings.use_auth)
        self._kill = self._switch(body, 6, "Close running Wow.exe before launching", settings.kill_existing)
        self._console = self._switch(body, 7, "Open the in-game SDK console", settings.open_console)
        self._logs = self._switch(body, 8, "Show captured logs in the console", settings.show_logs_in_console)
        ctk.CTkLabel(body, text="Wow.exe arguments", anchor="w").grid(row=9, column=0, sticky="w", pady=4)
        self._args = ctk.CTkEntry(body)
        self._args.insert(0, settings.wow_args)
        self._args.grid(row=9, column=1, columnspan=2, sticky="ew", pady=4)

        ctk.CTkLabel(body, text="Session", font=theme.title(15), anchor="w").grid(
            row=10, column=0, columnspan=3, sticky="w", pady=(16, 6))
        ctk.CTkLabel(body, text="Token", anchor="w").grid(row=11, column=0, sticky="w", pady=4)
        self._token = ctk.CTkEntry(body, show="\u2022", placeholder_text="Empty when the SDK runs with --no-auth")
        if token:
            self._token.insert(0, token)
        self._token.grid(row=11, column=1, columnspan=2, sticky="ew", pady=4)
        ctk.CTkLabel(
            body, text="Launching from this client creates a new token. Paste one here to control a game "
                       "started elsewhere. The token is never saved to disk.",
            font=theme.small(), text_color=theme.MUTED, anchor="w", justify="left", wraplength=520,
        ).grid(row=12, column=0, columnspan=3, sticky="w")

        ctk.CTkLabel(body, text="Appearance", anchor="w").grid(row=13, column=0, sticky="w", pady=(16, 4))
        self._appearance = ctk.CTkSegmentedButton(body, values=["dark", "light", "system"],
                                                  selected_color=theme.ACCENT,
                                                  selected_hover_color=theme.ACCENT_HOVER)
        self._appearance.set(settings.appearance)
        self._appearance.grid(row=13, column=1, sticky="w", pady=(16, 4))

        buttons = ctk.CTkFrame(body, fg_color="transparent")
        buttons.grid(row=14, column=0, columnspan=3, sticky="e", pady=(20, 0))
        ctk.CTkButton(buttons, text="Cancel", width=90, fg_color=theme.SURFACE_ALT, hover_color=theme.SURFACE,
                      text_color=("#1a1a1a", "#e6e6e6"), command=self.destroy).pack(side="left", padx=(0, 8))
        ctk.CTkButton(buttons, text="Save", width=90, fg_color=theme.ACCENT, hover_color=theme.ACCENT_HOVER,
                      command=self._save).pack(side="left")

        self.after(50, self._focus)

    @staticmethod
    def _switch(master: Any, row: int, text: str, value: bool) -> ctk.CTkSwitch:
        switch = ctk.CTkSwitch(master, text=text, progress_color=theme.ACCENT)
        if value:
            switch.select()
        switch.grid(row=row, column=0, columnspan=3, sticky="w", pady=3)
        return switch

    def _focus(self) -> None:
        self.lift()
        self.focus_force()
        self.grab_set()

    def _save(self) -> None:
        settings = Settings(
            wow_path=self._wow.value(),
            dll_path=self._dll.value(),
            injector_path=self._injector.value(),
            use_auth=bool(self._use_auth.get()),
            kill_existing=bool(self._kill.get()),
            wow_args=self._args.get().strip(),
            open_console=bool(self._console.get()),
            show_logs_in_console=bool(self._logs.get()),
            appearance=self._appearance.get(),
        )
        token = self._token.get().strip() or None
        self._on_save(settings, token)
        self.destroy()
