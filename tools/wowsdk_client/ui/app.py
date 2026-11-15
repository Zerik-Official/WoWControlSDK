"""Main window of the WoWControlSDK client."""

# Standard library
from __future__ import annotations

import json
import queue
import threading
import time
from tkinter import messagebox
from typing import Any

# Third-party
import customtkinter as ctk

# Project imports
from ..catalog import Method, all_methods
from ..launcher import LaunchError, describe_command, build_command, launch, parse_pid
from ..settings import Settings
from ..transport import CommandClient, RpcResult
from . import theme
from .events_panel import EventsPanel
from .json_view import JsonView
from .method_form import FormError, MethodForm
from .settings_dialog import SettingsDialog
from .sidebar import Sidebar

WINDOW_TITLE = "WoWControlSDK Client"
STATUS_POLL_MS = 2000
QUEUE_POLL_MS = 80
HISTORY_LIMIT = 100
PREFERRED_SIZE = (1360, 820)
MIN_SIZE = (980, 560)
SCREEN_MARGIN = (40, 80)


class HistoryEntry:
    """A finished call kept in the history list."""

    def __init__(self, method: Method, result: RpcResult, finished_at: str) -> None:
        self.method = method
        self.result = result
        self.finished_at = finished_at


class ClientApp(ctk.CTk):
    """Window with the method list, the request form and the output tabs."""

    def __init__(self) -> None:
        self.settings = Settings.load()
        ctk.set_appearance_mode(self.settings.appearance)
        super().__init__()
        self.title(WINDOW_TITLE)
        self._fit_to_screen()

        self.client = CommandClient()
        self._results: queue.Queue[tuple[Method, RpcResult]] = queue.Queue()
        self._injector_output: queue.Queue[str] = queue.Queue()
        self._history: list[HistoryEntry] = []
        self._busy = False

        self.grid_columnconfigure(1, weight=0)
        self.grid_columnconfigure(2, weight=1)
        self.grid_rowconfigure(1, weight=1)

        self._build_header()
        self.sidebar = Sidebar(self, on_select=self._select_method)
        self.sidebar.grid(row=1, column=0, sticky="nsw")
        self._build_center()
        self._build_output()

        self.bind("<Control-Return>", lambda _event: self.send())
        self.protocol("WM_DELETE_WINDOW", self._close)
        self.after(STATUS_POLL_MS, self._poll_status)
        self.after(QUEUE_POLL_MS, self._poll_queues)
        self._refresh_status()
        self.sidebar.select(all_methods()[0])

    def _fit_to_screen(self) -> None:
        width = min(PREFERRED_SIZE[0], self.winfo_screenwidth() - SCREEN_MARGIN[0])
        height = min(PREFERRED_SIZE[1], self.winfo_screenheight() - SCREEN_MARGIN[1])
        self.geometry(f"{width}x{height}+{SCREEN_MARGIN[0] // 2}+0")
        self.minsize(MIN_SIZE[0], MIN_SIZE[1])
        if width < PREFERRED_SIZE[0] or height < PREFERRED_SIZE[1]:
            self.after(0, lambda: self.state("zoomed"))

    def _build_header(self) -> None:
        header = ctk.CTkFrame(self, height=56, corner_radius=0, fg_color=theme.SURFACE_ALT)
        header.grid(row=0, column=0, columnspan=3, sticky="ew")
        header.grid_columnconfigure(2, weight=1)

        ctk.CTkLabel(header, text=WINDOW_TITLE, font=theme.title(17)).grid(row=0, column=0, padx=(16, 20), pady=12)
        self._status = ctk.CTkLabel(header, text="", font=theme.small(13))
        self._status.grid(row=0, column=1, sticky="w")
        self._auth = ctk.CTkLabel(header, text="", font=theme.small(13), text_color=theme.MUTED)
        self._auth.grid(row=0, column=2, sticky="w", padx=(16, 0))

        ctk.CTkButton(header, text="Settings", width=96, fg_color=theme.SURFACE, hover_color=theme.SURFACE_ALT,
                      text_color=("#1a1a1a", "#e6e6e6"), command=self._open_settings).grid(row=0, column=3,
                                                                                           padx=(0, 8))
        ctk.CTkButton(header, text="Launch WoW", width=120, fg_color=theme.ACCENT, hover_color=theme.ACCENT_HOVER,
                      command=self._launch).grid(row=0, column=4, padx=(0, 16))

    def _build_center(self) -> None:
        center = ctk.CTkFrame(self, width=520, corner_radius=0, fg_color="transparent")
        center.grid(row=1, column=1, sticky="nsew", padx=(16, 8), pady=12)
        center.grid_propagate(False)
        center.grid_rowconfigure(0, weight=1)
        center.grid_columnconfigure(0, weight=1)

        self.form = MethodForm(center, on_submit=self.send)
        self.form.grid(row=0, column=0, sticky="nsew")

        bar = ctk.CTkFrame(center, fg_color="transparent")
        bar.grid(row=1, column=0, sticky="ew", pady=(8, 0))
        bar.grid_columnconfigure(2, weight=1)
        self._send = ctk.CTkButton(bar, text="Send  (Ctrl+Enter)", width=170, fg_color=theme.ACCENT,
                                   hover_color=theme.ACCENT_HOVER, command=self.send)
        self._send.grid(row=0, column=0)
        ctk.CTkButton(bar, text="Copy request", width=120, fg_color=theme.SURFACE_ALT, hover_color=theme.SURFACE,
                      text_color=("#1a1a1a", "#e6e6e6"), command=self._copy_request).grid(row=0, column=1,
                                                                                          padx=(8, 0))
        self._form_error = ctk.CTkLabel(bar, text="", text_color=theme.DANGER, font=theme.small(), anchor="w")
        self._form_error.grid(row=0, column=2, sticky="w", padx=(12, 0))

    def _build_output(self) -> None:
        tabs = ctk.CTkTabview(self, segmented_button_selected_color=theme.ACCENT,
                              segmented_button_selected_hover_color=theme.ACCENT_HOVER)
        tabs.grid(row=1, column=2, sticky="nsew", padx=(8, 16), pady=(4, 12))
        self.tabs = tabs
        for name in ("Response", "History", "Events", "Activity"):
            tab = tabs.add(name)
            tab.grid_columnconfigure(0, weight=1)
            tab.grid_rowconfigure(1, weight=1)

        response = tabs.tab("Response")
        self._response_info = ctk.CTkLabel(response, text="No request sent yet", font=theme.small(13),
                                           text_color=theme.MUTED, anchor="w")
        self._response_info.grid(row=0, column=0, sticky="ew", pady=(0, 6))
        self.response = JsonView(response)
        self.response.grid(row=1, column=0, sticky="nsew")

        history = tabs.tab("History")
        ctk.CTkLabel(history, text="Select a call to see its request and response", font=theme.small(),
                     text_color=theme.MUTED, anchor="w").grid(row=0, column=0, sticky="ew", pady=(0, 6))
        self._history_list = ctk.CTkScrollableFrame(history, fg_color="transparent")
        self._history_list.grid(row=1, column=0, sticky="nsew")
        self._history_list.grid_columnconfigure(0, weight=1)

        events = tabs.tab("Events")
        events.grid_rowconfigure(0, weight=1)
        events.grid_rowconfigure(1, weight=0)
        self.events = EventsPanel(events, token=lambda: self.client.token, log=self.log)
        self.events.grid(row=0, column=0, sticky="nsew")

        activity = tabs.tab("Activity")
        activity.grid_rowconfigure(0, weight=1)
        activity.grid_rowconfigure(1, weight=0)
        self.activity = JsonView(activity)
        self.activity.grid(row=0, column=0, sticky="nsew")

    def log(self, text: str) -> None:
        """Add a line to the activity log."""
        self.activity.append_line(f"{time.strftime('%H:%M:%S')}  {text}")

    def _select_method(self, method: Method) -> None:
        self.form.show(method)
        self._form_error.configure(text="")

    def _current_request(self) -> tuple[Method, dict[str, Any]] | None:
        method = self.form.method
        if method is None:
            return None
        try:
            params = self.form.params()
        except FormError as exc:
            self._form_error.configure(text=str(exc))
            return None
        self._form_error.configure(text="")
        return method, params

    def send(self) -> None:
        """Validate the form and send the request on a worker thread."""
        if self._busy:
            return
        current = self._current_request()
        if current is None:
            return
        method, params = current
        if method.confirm and not messagebox.askyesno(WINDOW_TITLE, method.confirm, parent=self):
            return
        self._busy = True
        self._send.configure(state="disabled", text="Waiting...")
        self._response_info.configure(text=f"{method.name}  •  waiting for the game...", text_color=theme.MUTED)
        threading.Thread(target=self._call, args=(method, params), daemon=True).start()

    def _call(self, method: Method, params: dict[str, Any]) -> None:
        self._results.put((method, self.client.call(method.name, params)))

    def _copy_request(self) -> None:
        current = self._current_request()
        if current is None:
            return
        method, params = current
        request = self.client.build_request(method.name, params)
        if "auth" in request:
            request["auth"] = "<session token>"
        self.clipboard_clear()
        self.clipboard_append(json.dumps(request, ensure_ascii=False))
        self.log(f"Copied the {method.name} request to the clipboard")

    def _show_result(self, method: Method, result: RpcResult) -> None:
        status = "OK" if result.ok else "Error"
        color = theme.SUCCESS if result.ok else theme.DANGER
        self._response_info.configure(text=f"{method.name}  •  {status}  •  {result.elapsed_ms:.0f} ms",
                                      text_color=color)
        if result.error is not None:
            self.response.show_text(result.error)
        else:
            self.response.show_value(result.response)
        self.tabs.set("Response")

    def _add_history(self, method: Method, result: RpcResult) -> None:
        entry = HistoryEntry(method, result, time.strftime("%H:%M:%S"))
        self._history.insert(0, entry)
        del self._history[HISTORY_LIMIT:]
        for child in self._history_list.winfo_children():
            child.destroy()
        for row, item in enumerate(self._history):
            mark = "✓" if item.result.ok else "✗"
            text = f"{item.finished_at}   {mark}  {item.method.name}   {item.result.elapsed_ms:.0f} ms"
            ctk.CTkButton(
                self._history_list, text=text, anchor="w", font=theme.mono(12), fg_color="transparent",
                hover_color=theme.SURFACE_ALT, text_color=theme.SUCCESS if item.result.ok else theme.DANGER,
                command=lambda selected=item: self._show_history(selected),
            ).grid(row=row, column=0, sticky="ew", pady=1)

    def _show_history(self, entry: HistoryEntry) -> None:
        request = dict(entry.result.request)
        if "auth" in request:
            request["auth"] = "<session token>"
        self._response_info.configure(text=f"{entry.method.name}  •  {entry.finished_at}",
                                      text_color=theme.MUTED)
        self.response.show_value({
            "request": request,
            "response": entry.result.response if entry.result.error is None else entry.result.error,
        })
        self.tabs.set("Response")

    def _poll_queues(self) -> None:
        while True:
            try:
                method, result = self._results.get_nowait()
            except queue.Empty:
                break
            self._busy = False
            self._send.configure(state="normal", text="Send  (Ctrl+Enter)")
            self._show_result(method, result)
            self._add_history(method, result)
        while True:
            try:
                line = self._injector_output.get_nowait()
            except queue.Empty:
                break
            self.log(line)
            pid = parse_pid(line)
            if pid is not None:
                self.log(f"[client] game process id {pid}")
        self.after(QUEUE_POLL_MS, self._poll_queues)

    def _poll_status(self) -> None:
        self._refresh_status()
        self.after(STATUS_POLL_MS, self._poll_status)

    def _refresh_status(self) -> None:
        if self.client.is_available():
            self._status.configure(text="● SDK connected", text_color=theme.SUCCESS)
        else:
            self._status.configure(text="● SDK not running", text_color=theme.DANGER)
        self._auth.configure(text="Session token set" if self.client.token else "No session token")

    def _open_settings(self) -> None:
        SettingsDialog(self, self.settings, self.client.token, self._apply_settings)

    def _apply_settings(self, settings: Settings, token: str | None) -> None:
        self.settings = settings
        settings.save()
        ctk.set_appearance_mode(settings.appearance)
        self.client.token = token
        self.log("Settings saved")
        self._refresh_status()

    def _launch(self) -> None:
        if self.settings.kill_existing and self.client.is_available():
            if not messagebox.askyesno(WINDOW_TITLE, "A game with the SDK is already running. Close it and "
                                                     "start a new one?", parent=self):
                return
        try:
            result = launch(self.settings, self._injector_output)
        except LaunchError as exc:
            messagebox.showerror(WINDOW_TITLE, str(exc), parent=self)
            return
        self.events.disconnect()
        self.client.token = result.token
        command = build_command(self.settings, result.token is not None)
        self.log(f"[client] {describe_command(command)}")
        self.log("[client] new session token created" if result.token else "[client] authentication disabled")
        self._refresh_status()
        self.tabs.set("Activity")

    def _close(self) -> None:
        self.events.disconnect()
        self.destroy()


def main() -> None:
    """Start the client."""
    app = ClientApp()
    app.mainloop()
