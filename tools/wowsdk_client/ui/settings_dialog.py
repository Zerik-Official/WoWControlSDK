"""Dialog to edit the client settings and the session token."""

# Standard library
from __future__ import annotations

from tkinter import filedialog
from typing import Any, Callable

# Third-party
import customtkinter as ctk

# Project imports
from ..i18n import LANGUAGES, tr
from ..settings import Settings, default_binary
from . import theme
from .chrome import WindowChrome


APPEARANCES: tuple[str, ...] = ("dark", "light", "system")


class PathRow:
    """Label, entry and browse button for a file path."""

    def __init__(self, master: Any, row: int, label: str, value: str, placeholder: str,
                 file_types: list[tuple[str, str]]) -> None:
        ctk.CTkLabel(master, text=label, anchor="w").grid(row=row, column=0, sticky="w", padx=(0, 10), pady=4)
        self.entry = ctk.CTkEntry(master, width=380, placeholder_text=placeholder)
        if value:
            self.entry.insert(0, value)
        self.entry.grid(row=row, column=1, sticky="ew", pady=4)
        ctk.CTkButton(master, text=tr("settings.browse"), width=80, fg_color=theme.SURFACE_ALT, hover_color=theme.SURFACE,
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
        self.transient(master)
        self._master = master
        self._settings = settings
        self._on_save = on_save
        self.chrome = WindowChrome(self, tr("settings.title"), resizable=False, minimizable=False, taskbar=False)

        body = ctk.CTkFrame(self.chrome.body, fg_color="transparent")
        body.pack(fill="both", expand=True, padx=20, pady=20)
        body.grid_columnconfigure(0, minsize=150)
        body.grid_columnconfigure(1, weight=1)

        ctk.CTkLabel(body, text=tr("settings.game_and_sdk"), font=theme.title(15), anchor="w").grid(
            row=0, column=0, columnspan=3, sticky="w", pady=(0, 6))
        self._wow = PathRow(body, 1, "Wow.exe", settings.wow_path, "C:\\World of Warcraft\\Wow.exe",
                            [("Wow.exe", "Wow.exe"), ("Executables", "*.exe")])
        self._dll = PathRow(body, 2, tr("settings.dll"), settings.dll_path,
                            default_binary("WoWControlSDK.dll") or "WoWControlSDK.dll",
                            [("DLL", "*.dll")])
        self._injector = PathRow(body, 3, tr("settings.injector"), settings.injector_path,
                                 default_binary("WowInjector.exe") or "WowInjector.exe",
                                 [("Executables", "*.exe")])

        ctk.CTkLabel(body, text=tr("settings.launch_options"), font=theme.title(15), anchor="w").grid(
            row=4, column=0, columnspan=3, sticky="w", pady=(16, 6))
        self._use_auth = self._switch(body, 5, tr("settings.use_auth"), settings.use_auth)
        self._kill = self._switch(body, 6, tr("settings.kill"), settings.kill_existing)
        self._console = self._switch(body, 7, tr("settings.console"), settings.open_console)
        self._logs = self._switch(body, 8, tr("settings.logs"), settings.show_logs_in_console)
        self._hidden = self._switch(body, 9, tr("settings.hidden"), settings.start_hidden)
        self._auto_events = self._switch(body, 10, tr("settings.auto_events"), settings.auto_connect_events)
        self._manual_map = self._switch(body, 11, tr("settings.manual_map"), settings.manual_map)
        ctk.CTkLabel(
            body, text=tr("settings.manual_map_help"),
            font=theme.small(), text_color=theme.MUTED, anchor="w", justify="left", wraplength=520,
        ).grid(row=12, column=0, columnspan=3, sticky="w")
        ctk.CTkLabel(body, text=tr("settings.wow_args"), anchor="w").grid(row=13, column=0, sticky="w", pady=4)
        self._args = ctk.CTkEntry(body)
        self._args.insert(0, settings.wow_args)
        self._args.grid(row=13, column=1, columnspan=2, sticky="ew", pady=4)

        ctk.CTkLabel(body, text=tr("settings.session"), font=theme.title(15), anchor="w").grid(
            row=14, column=0, columnspan=3, sticky="w", pady=(16, 6))
        ctk.CTkLabel(body, text=tr("settings.token"), anchor="w").grid(row=15, column=0, sticky="w", pady=4)
        self._token = ctk.CTkEntry(body, show="\u2022", placeholder_text=tr("settings.token_placeholder"))
        if token:
            self._token.insert(0, token)
        self._token.grid(row=15, column=1, columnspan=2, sticky="ew", pady=4)
        ctk.CTkLabel(
            body, text=tr("settings.token_help"),
            font=theme.small(), text_color=theme.MUTED, anchor="w", justify="left", wraplength=520,
        ).grid(row=16, column=0, columnspan=3, sticky="w")

        self._appearance_labels = {tr(f"settings.appearance.{mode}"): mode for mode in APPEARANCES}
        ctk.CTkLabel(body, text=tr("settings.appearance"), anchor="w").grid(row=17, column=0, sticky="w",
                                                                            pady=(16, 4))
        self._appearance = ctk.CTkSegmentedButton(body, values=list(self._appearance_labels),
                                                  selected_color=theme.ACCENT,
                                                  selected_hover_color=theme.ACCENT_HOVER)
        self._appearance.set(tr(f"settings.appearance.{settings.appearance}")
                             if settings.appearance in APPEARANCES else tr("settings.appearance.dark"))
        self._appearance.grid(row=17, column=1, sticky="w", pady=(16, 4))

        self._language_labels = {tr("settings.language.auto"): ""}
        self._language_labels.update({name: code for code, name in LANGUAGES.items()})
        ctk.CTkLabel(body, text=tr("settings.language"), anchor="w").grid(row=18, column=0, sticky="w", pady=4)
        self._language = ctk.CTkOptionMenu(body, values=list(self._language_labels), fg_color=theme.SURFACE_ALT,
                                           button_color=theme.ACCENT, button_hover_color=theme.ACCENT_HOVER)
        self._language.set(LANGUAGES.get(settings.language, tr("settings.language.auto")))
        self._language.grid(row=18, column=1, sticky="w", pady=4)

        buttons = ctk.CTkFrame(body, fg_color="transparent")
        buttons.grid(row=19, column=0, columnspan=3, sticky="e", pady=(20, 0))
        ctk.CTkButton(buttons, text=tr("settings.cancel"), width=90, fg_color=theme.SURFACE_ALT, hover_color=theme.SURFACE,
                      text_color=("#1a1a1a", "#e6e6e6"), command=self.destroy).pack(side="left", padx=(0, 8))
        ctk.CTkButton(buttons, text=tr("settings.save"), width=90, fg_color=theme.ACCENT, hover_color=theme.ACCENT_HOVER,
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
        self.chrome.center_on(self._master)
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
            start_hidden=bool(self._hidden.get()),
            manual_map=bool(self._manual_map.get()),
            auto_connect_events=bool(self._auto_events.get()),
            appearance=self._appearance_labels.get(self._appearance.get(), "dark"),
            language=self._language_labels.get(self._language.get(), ""),
        )
        token = self._token.get().strip() or None
        self._on_save(settings, token)
        self.destroy()
