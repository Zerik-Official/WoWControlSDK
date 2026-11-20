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
from .. import offline
from ..catalog import Method, all_methods, find_method, mask_request
from ..i18n import set_language, tr
from ..launcher import (InjectorOutput, LaunchError, build_command, describe_command, diagnose, is_error_line,
                        launch, parse_pid)
from ..offline import DllBuild
from ..sequence import Step, autosave_path
from ..sequence import load as load_sequence
from ..sequence import save as save_sequence
from ..settings import Settings
from ..transport import CommandClient, RpcResult
from . import theme
from .events_panel import EventsPanel
from .icons import icon
from .json_view import JsonView
from .method_form import FormError, MethodForm
from .processes_panel import ProcessesPanel
from .sequence_dialog import SequenceDialog
from .settings_dialog import SettingsDialog
from .sidebar import Sidebar
from .chrome import WindowChrome
from .tooltip import tooltip

STATUS_POLL_MS = 2000
QUEUE_POLL_MS = 80
HISTORY_LIMIT = 100
PREFERRED_SIZE = (1360, 820)
MIN_SIZE = (980, 560)
SCREEN_MARGIN = (40, 80)
TAB_KEYS = ("app.tab.response", "app.tab.history", "app.tab.events", "app.tab.processes", "app.tab.activity")


class HistoryEntry:
    """A finished call kept in the history list."""

    def __init__(self, method: Method, result: RpcResult, finished_at: str) -> None:
        self.method = method
        self.result = result
        self.finished_at = finished_at


def request_params(result: RpcResult) -> dict[str, Any]:
    """The parameters a call was sent with."""
    params = result.request.get("params")
    return dict(params) if isinstance(params, dict) else {}


class ClientApp(ctk.CTk):
    """Window with the method list, the request form and the output tabs."""

    def __init__(self) -> None:
        self.settings = Settings.load()
        set_language(self.settings.language)
        ctk.set_appearance_mode(self.settings.appearance)
        super().__init__()
        self.title(tr("app.title"))
        self.chrome = WindowChrome(self, tr("app.title"), resizable=True, minimizable=True, on_close=self._close,
                                   min_size=MIN_SIZE)
        self.content = self.chrome.body

        self.client = CommandClient()
        self.sequence: list[Step] = self._load_sequence()
        self._results: queue.Queue[tuple[Method, RpcResult]] = queue.Queue()
        self._injector_output: queue.Queue[InjectorOutput] = queue.Queue()
        self._builds: queue.Queue[DllBuild | None] = queue.Queue()
        self._launch_errors: list[str] = []
        self._history: list[HistoryEntry] = []
        self._last_params: dict[str, dict[str, Any]] = {}
        self._shown: tuple[Method, RpcResult] | None = None
        self._editing: int | None = None
        self._sequence_dialog: SequenceDialog | None = None
        self._build: DllBuild | None = None
        self._busy = False

        self.content.grid_columnconfigure(1, weight=0)
        self.content.grid_columnconfigure(2, weight=1)
        self.content.grid_rowconfigure(0, weight=1)

        self._build_titlebar()
        self.sidebar = Sidebar(self.content, on_select=self._select_method)
        self.sidebar.grid(row=0, column=0, sticky="nsw")
        self._build_center()
        self._build_output()

        self.bind("<Control-Return>", lambda _event: self.send())
        self.bind("<Control-Shift-Return>", lambda _event: self._add_to_sequence())
        self.bind("<Control-f>", lambda _event: self.sidebar.focus_search())
        self.bind("<Control-r>", lambda _event: self._repeat_shown())
        self.protocol("WM_DELETE_WINDOW", self._close)
        self.after(STATUS_POLL_MS, self._poll_status)
        self.after(QUEUE_POLL_MS, self._poll_queues)
        self._refresh_status()
        self._refresh_sequence_button()
        self._read_dll_build()
        self.sidebar.select(all_methods()[0])
        self.after(20, self._fit_to_screen)

    def _fit_to_screen(self) -> None:
        self.chrome.center_on(None, PREFERRED_SIZE)
        scale = ctk.ScalingTracker.get_window_scaling(self)
        if self.winfo_screenwidth() < (PREFERRED_SIZE[0] + SCREEN_MARGIN[0]) * scale or \
                self.winfo_screenheight() < (PREFERRED_SIZE[1] + SCREEN_MARGIN[1]) * scale:
            self.chrome.maximize()

    def _build_titlebar(self) -> None:
        left = self.chrome.left
        self._status = ctk.CTkLabel(left, text="", font=theme.small(12))
        self._status.grid(row=0, column=0, sticky="w")
        self._auth = ctk.CTkLabel(left, text="", font=theme.small(12), text_color=theme.MUTED)
        self._auth.grid(row=0, column=1, sticky="w", padx=(14, 0))
        self._dll = ctk.CTkLabel(left, text="", font=theme.small(12), text_color=theme.MUTED, compound="left",
                                 image=icon("info", theme.MUTED, 14))
        self._dll.grid(row=0, column=2, sticky="w", padx=(14, 0))
        tooltip(self._dll, self._dll_tooltip)
        for widget in (self._status, self._auth):
            self.chrome.make_draggable(widget)

        right = self.chrome.right
        self._sequence_button = ctk.CTkButton(right, text="", width=120, height=28,
                                              image=icon("list-ordered", theme.TEXT, 15), compound="left",
                                              fg_color=theme.SURFACE_ALT, hover_color=theme.SURFACE,
                                              text_color=theme.TEXT, command=self.open_sequence)
        self._sequence_button.grid(row=0, column=0, padx=(0, 6))
        tooltip(self._sequence_button, tr("app.sequence_tip"))
        ctk.CTkButton(right, text=tr("app.settings"), width=100, height=28, image=icon("settings", theme.TEXT, 15),
                      compound="left", fg_color=theme.SURFACE_ALT, hover_color=theme.SURFACE, text_color=theme.TEXT,
                      command=self._open_settings).grid(row=0, column=1, padx=(0, 6))
        ctk.CTkButton(right, text=tr("app.launch"), width=120, height=28, image=icon("play", theme.ON_ACCENT, 15),
                      compound="left", text_color=theme.ON_ACCENT, fg_color=theme.ACCENT,
                      hover_color=theme.ACCENT_HOVER, command=self._launch).grid(row=0, column=2)

    def _build_center(self) -> None:
        center = ctk.CTkFrame(self.content, width=520, corner_radius=0, fg_color="transparent")
        center.grid(row=0, column=1, sticky="nsew", padx=(16, 8), pady=12)
        center.grid_propagate(False)
        center.grid_rowconfigure(0, weight=1)
        center.grid_columnconfigure(0, weight=1)

        self.form = MethodForm(center, on_submit=self.send)
        self.form.grid(row=0, column=0, sticky="nsew")

        bar = ctk.CTkFrame(center, fg_color="transparent")
        bar.grid(row=1, column=0, sticky="ew", pady=(8, 0))
        bar.grid_columnconfigure(4, weight=1)
        self._send = ctk.CTkButton(bar, text=tr("app.send"), width=180, image=icon("send", theme.ON_ACCENT, 16),
                                   compound="left", text_color=theme.ON_ACCENT, fg_color=theme.ACCENT,
                                   hover_color=theme.ACCENT_HOVER, text_color_disabled=theme.ON_ACCENT_DISABLED,
                                   command=self.send)
        self._send.grid(row=0, column=0)
        self._add_step = ctk.CTkButton(bar, text=tr("app.add_step"), width=150,
                                       image=icon("list-plus", theme.TEXT, 16), compound="left",
                                       fg_color=theme.SURFACE_ALT, hover_color=theme.SURFACE, text_color=theme.TEXT,
                                       command=self._add_to_sequence)
        self._add_step.grid(row=0, column=1, padx=(8, 0))
        self._add_step_tip = tooltip(self._add_step, tr("app.add_step_tip"))
        copy = ctk.CTkButton(bar, text="", width=34, image=icon("copy", theme.TEXT, 16), fg_color=theme.SURFACE_ALT,
                             hover_color=theme.SURFACE, command=self._copy_request)
        copy.grid(row=0, column=2, padx=(8, 0))
        tooltip(copy, tr("app.copy_request"))
        self._cancel_edit = ctk.CTkButton(bar, text="", width=34, image=icon("x", theme.TEXT, 16),
                                          fg_color=theme.SURFACE_ALT, hover_color=theme.SURFACE,
                                          command=self._stop_editing)
        tooltip(self._cancel_edit, tr("app.cancel_edit"))
        self._form_error = ctk.CTkLabel(bar, text="", text_color=theme.DANGER, font=theme.small(), anchor="w",
                                        justify="left", wraplength=480)
        self._form_error.grid(row=1, column=0, columnspan=5, sticky="w", pady=(4, 0))

    def _build_output(self) -> None:
        tabs = ctk.CTkTabview(self.content, segmented_button_selected_color=theme.ACCENT,
                              segmented_button_selected_hover_color=theme.ACCENT_HOVER)
        tabs.grid(row=0, column=2, sticky="nsew", padx=(8, 16), pady=(4, 12))
        self.tabs = tabs
        for key in TAB_KEYS:
            tab = tabs.add(tr(key))
            tab.grid_columnconfigure(0, weight=1)
            tab.grid_rowconfigure(1, weight=1)

        response = tabs.tab(tr("app.tab.response"))
        info = ctk.CTkFrame(response, fg_color="transparent")
        info.grid(row=0, column=0, sticky="ew", pady=(0, 6))
        info.grid_columnconfigure(0, weight=1)
        self._response_info = ctk.CTkLabel(info, text=tr("app.no_request"), font=theme.small(13),
                                           text_color=theme.MUTED, anchor="w")
        self._response_info.grid(row=0, column=0, sticky="ew")
        self._repeat_button = ctk.CTkButton(info, text=tr("app.repeat"), width=100,
                                            image=icon("rotate-ccw", theme.TEXT, 15), compound="left",
                                            fg_color=theme.SURFACE_ALT, hover_color=theme.SURFACE,
                                            text_color=theme.TEXT, state="disabled", command=self._repeat_shown)
        self._repeat_button.grid(row=0, column=1, padx=(8, 0))
        tooltip(self._repeat_button, tr("app.repeat_tip"))
        self._copy_response = ctk.CTkButton(info, text="", width=34, image=icon("copy", theme.TEXT, 15),
                                            fg_color=theme.SURFACE_ALT, hover_color=theme.SURFACE, state="disabled",
                                            command=self._copy_shown_response)
        self._copy_response.grid(row=0, column=2, padx=(6, 0))
        tooltip(self._copy_response, tr("app.copy_response"))
        self.response = JsonView(response)
        self.response.grid(row=1, column=0, sticky="nsew")

        history = tabs.tab(tr("app.tab.history"))
        ctk.CTkLabel(history, text=tr("app.history_hint"), font=theme.small(), text_color=theme.MUTED,
                     anchor="w").grid(row=0, column=0, sticky="ew", pady=(0, 6))
        self._history_list = ctk.CTkScrollableFrame(history, fg_color="transparent")
        self._history_list.grid(row=1, column=0, sticky="nsew")
        self._history_list.grid_columnconfigure(0, weight=1)

        events = tabs.tab(tr("app.tab.events"))
        events.grid_rowconfigure(0, weight=1)
        events.grid_rowconfigure(1, weight=0)
        self.events = EventsPanel(events, token=lambda: self.client.token, log=self.log)
        self.events.grid(row=0, column=0, sticky="nsew")

        processes = tabs.tab(tr("app.tab.processes"))
        processes.grid_rowconfigure(0, weight=1)
        processes.grid_rowconfigure(1, weight=0)
        self.processes = ProcessesPanel(processes, game_path=lambda: self.settings.wow_path, log=self.log)
        self.processes.grid(row=0, column=0, sticky="nsew")

        activity = tabs.tab(tr("app.tab.activity"))
        activity.grid_rowconfigure(0, weight=1)
        activity.grid_rowconfigure(1, weight=0)
        self.activity = JsonView(activity)
        self.activity.grid(row=0, column=0, sticky="nsew")

    def log(self, text: str) -> None:
        """Add a line to the activity log."""
        self.activity.append_line(f"{time.strftime('%H:%M:%S')}  {text}")

    def _select_method(self, method: Method) -> None:
        if self._editing is not None:
            editing = self.sequence[self._editing] if self._editing < len(self.sequence) else None
            if editing is None or editing.method != method.name:
                self._stop_editing()
        self.form.show(method)
        remembered = self._last_params.get(method.name)
        if remembered is not None:
            self.form.fill(remembered)
        self._form_error.configure(text="")

    def _load_method(self, name: str, params: dict[str, Any]) -> Method | None:
        method = find_method(name)
        if method is None:
            self._form_error.configure(text=tr("app.unknown_method", method=name))
            return None
        self.sidebar.select(method)
        self.form.fill(params)
        return method

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

    def call_method(self, name: str, params: dict[str, Any]) -> RpcResult:
        """Send a call through the command pipe, or through the injector when it works without the game."""
        if offline.supports(name) and not self.client.is_available():
            return offline.call(self.settings, name)
        return self.client.call(name, params)

    def send(self) -> None:
        """Validate the form and send the request on a worker thread."""
        if self._busy:
            return
        current = self._current_request()
        if current is None:
            return
        method, params = current
        if method.confirm and not messagebox.askyesno(tr("app.title"), method.confirm, parent=self):
            return
        self._last_params[method.name] = params
        self._busy = True
        self._send.configure(state="disabled", text=tr("app.waiting"))
        self._response_info.configure(text=tr("app.waiting_game", method=method.name), text_color=theme.MUTED)
        threading.Thread(target=self._call, args=(method, params), daemon=True).start()

    def _call(self, method: Method, params: dict[str, Any]) -> None:
        self._results.put((method, self.call_method(method.name, params)))

    def _copy_request(self) -> None:
        current = self._current_request()
        if current is None:
            return
        method, params = current
        request = mask_request(self.client.build_request(method.name, params))
        self.clipboard_clear()
        self.clipboard_append(json.dumps(request, ensure_ascii=False))
        self.log(tr("app.copied", method=method.name))

    def _show_header(self, method: Method, result: RpcResult, label: str, color: Any) -> None:
        self._shown = (method, result)
        self._repeat_button.configure(state="normal")
        self._copy_response.configure(state="normal")
        self._response_info.configure(text=label, text_color=color)

    def _show_result(self, method: Method, result: RpcResult) -> None:
        status = tr("app.result_ok") if result.ok else tr("app.result_error")
        label = f"{method.name}  •  {status}  •  {result.elapsed_ms:.0f} ms"
        if result.offline:
            label += f"  •  {tr('app.offline')}"
        self._show_header(method, result, label, theme.SUCCESS if result.ok else theme.DANGER)
        if result.error is not None:
            self.response.show_text(result.error)
        else:
            self.response.show_value(result.response)
        self.tabs.set(tr("app.tab.response"))

    def _add_history(self, method: Method, result: RpcResult) -> None:
        entry = HistoryEntry(method, result, time.strftime("%H:%M:%S"))
        self._history.insert(0, entry)
        del self._history[HISTORY_LIMIT:]
        for child in self._history_list.winfo_children():
            child.destroy()
        for row, item in enumerate(self._history):
            mark = "✓" if item.result.ok else "✗"
            text = f"{item.finished_at}   {mark}  {item.method.name}   {item.result.elapsed_ms:.0f} ms"
            line = ctk.CTkFrame(self._history_list, fg_color="transparent")
            line.grid(row=row, column=0, sticky="ew", pady=1)
            line.grid_columnconfigure(0, weight=1)
            ctk.CTkButton(
                line, text=text, anchor="w", font=theme.mono(12), fg_color="transparent",
                hover_color=theme.SURFACE_ALT, text_color=theme.SUCCESS if item.result.ok else theme.DANGER,
                command=lambda selected=item: self._show_history(selected),
            ).grid(row=0, column=0, sticky="ew")
            repeat = ctk.CTkButton(line, text="", width=30, height=26, image=icon("rotate-ccw", theme.TEXT, 14),
                                   fg_color="transparent", hover_color=theme.SURFACE_ALT,
                                   command=lambda selected=item: self._repeat(selected.method.name,
                                                                              request_params(selected.result)))
            repeat.grid(row=0, column=1, padx=(4, 0))
            tooltip(repeat, tr("app.repeat_tip"))

    def _show_history(self, entry: HistoryEntry) -> None:
        request = mask_request(entry.result.request)
        self._show_header(entry.method, entry.result, f"{entry.method.name}  •  {entry.finished_at}", theme.MUTED)
        self.response.show_value({
            "request": request,
            "response": entry.result.response if entry.result.error is None else entry.result.error,
        })
        self.tabs.set(tr("app.tab.response"))

    def _repeat(self, name: str, params: dict[str, Any]) -> None:
        self._stop_editing()
        method = self._load_method(name, params)
        if method is not None:
            self._last_params[method.name] = params
            self.log(tr("app.loaded", method=method.name))

    def _repeat_shown(self) -> None:
        if self._shown is not None:
            method, result = self._shown
            self._repeat(method.name, request_params(result))

    def _copy_shown_response(self) -> None:
        if self._shown is None:
            return
        method, result = self._shown
        text = result.error if result.error is not None else json.dumps(result.response, indent=2,
                                                                         ensure_ascii=False)
        self.clipboard_clear()
        self.clipboard_append(text)
        self.log(tr("app.response_copied", method=method.name))

    @staticmethod
    def _load_sequence() -> list[Step]:
        try:
            return load_sequence(autosave_path())
        except ValueError:
            return []

    def sequence_changed(self, rebuild: bool = True) -> None:
        """Save the sequence and refresh the views that show it."""
        try:
            save_sequence(self.sequence, autosave_path())
        except OSError as exc:
            self.log(tr("sequence.save_failed", error=exc))
        self._refresh_sequence_button()
        if rebuild and self._sequence_dialog is not None and self._sequence_dialog.winfo_exists():
            self._sequence_dialog.refresh()

    def _refresh_sequence_button(self) -> None:
        count = len(self.sequence)
        self._sequence_button.configure(text=f"{tr('app.sequence')} ({count})" if count else tr("app.sequence"))

    def open_sequence(self) -> None:
        """Show the sequence window, creating it when needed."""
        dialog = self._sequence_dialog
        if dialog is not None and dialog.winfo_exists():
            if not dialog.running:
                dialog.refresh()
            dialog.deiconify()
            dialog.lift()
            dialog.focus_force()
            return
        self._sequence_dialog = SequenceDialog(self, self)

    def _add_to_sequence(self) -> None:
        current = self._current_request()
        if current is None:
            return
        method, params = current
        self._last_params[method.name] = params
        index = self._editing
        if index is not None and index < len(self.sequence) and self.sequence[index].method == method.name:
            self.sequence[index].params = params
            self.log(tr("app.step_updated", index=index + 1, method=method.name))
        else:
            self.sequence.append(Step(method.name, params))
            self.log(tr("app.step_added", index=len(self.sequence), method=method.name))
        self._stop_editing()
        self.sequence_changed()

    def edit_step(self, index: int) -> None:
        """Load a step into the form; the add button then updates it."""
        if index >= len(self.sequence):
            return
        step = self.sequence[index]
        self._stop_editing()
        if self._load_method(step.method, step.params) is None:
            return
        self._editing = index
        self._add_step.configure(text=tr("app.update_step", index=index + 1), fg_color=theme.ACCENT,
                                 hover_color=theme.ACCENT_HOVER, text_color=theme.ON_ACCENT,
                                 image=icon("pencil", theme.ON_ACCENT, 16))
        self._add_step_tip.set_text(tr("app.update_step_tip"))
        self._cancel_edit.grid(row=0, column=3, padx=(8, 0), sticky="w")
        self.lift()

    def _stop_editing(self) -> None:
        if self._editing is None:
            return
        self._editing = None
        self._add_step.configure(text=tr("app.add_step"), fg_color=theme.SURFACE_ALT, hover_color=theme.SURFACE,
                                 text_color=theme.TEXT, image=icon("list-plus", theme.TEXT, 16))
        self._add_step_tip.set_text(tr("app.add_step_tip"))
        self._cancel_edit.grid_remove()

    def show_sequence_result(self, method: str, result: RpcResult) -> None:
        """Show a step that finished and add it to the history once."""
        found = find_method(method)
        if found is None:
            return
        self._show_result(found, result)
        if not any(item.result is result for item in self._history):
            self._add_history(found, result)

    def _read_dll_build(self) -> None:
        settings = self.settings
        threading.Thread(target=lambda: self._builds.put(offline.read_build(settings)), name="dll-build",
                         daemon=True).start()

    def _show_dll_build(self, build: DllBuild | None) -> None:
        self._build = build
        self._dll.configure(text=f"DLL {build.label}" if build else tr("app.dll_unknown"))

    def _dll_tooltip(self) -> str:
        build = self._build
        if build is None:
            return tr("app.dll_unknown_tip")
        lines = [tr("app.dll_version", version=build.version)]
        if build.commit:
            lines.append(tr("app.dll_commit", commit=build.commit))
            lines.append(tr("app.dll_date", date=build.commit_date or ""))
            if build.dirty:
                lines.append(tr("app.dll_dirty"))
        lines.append(self.settings.resolved_dll())
        return "\n".join(lines)

    def _poll_queues(self) -> None:
        while True:
            try:
                method, result = self._results.get_nowait()
            except queue.Empty:
                break
            self._busy = False
            self._send.configure(state="normal", text=tr("app.send"))
            self._show_result(method, result)
            self._add_history(method, result)
        while True:
            try:
                output = self._injector_output.get_nowait()
            except queue.Empty:
                break
            self._handle_injector_output(output)
        while True:
            try:
                build = self._builds.get_nowait()
            except queue.Empty:
                break
            self._show_dll_build(build)
        self.after(QUEUE_POLL_MS, self._poll_queues)

    def _handle_injector_output(self, output: InjectorOutput) -> None:
        if output.line is not None:
            self.log(output.line)
            if is_error_line(output.line):
                self._launch_errors.append(output.line)
            pid = parse_pid(output.line)
            if pid is not None:
                self.log(tr("app.game_pid", pid=pid))
            return

        code = output.exit_code or 0
        self.log(tr("app.injector_exited", code=code))
        self.processes.refresh()
        self._refresh_status()
        if code == 0:
            self.log(tr("app.launch_ok"))
            if self.settings.auto_connect_events:
                self.events.connect()
            return

        message = f"{tr('app.launch_failed', code=code)}\n\n{diagnose(self._launch_errors)}"
        if self._launch_errors:
            message += "\n\n" + tr("app.last_error", line=self._launch_errors[-1])
        messagebox.showerror(tr("app.launch_failed_title"), message, parent=self)

    def _poll_status(self) -> None:
        self._refresh_status()
        self.after(STATUS_POLL_MS, self._poll_status)

    def _refresh_status(self) -> None:
        if self.client.is_available():
            self._status.configure(text=tr("app.sdk_connected"), text_color=theme.SUCCESS)
        else:
            self._status.configure(text=tr("app.sdk_not_running"), text_color=theme.DANGER)
        self._auth.configure(text=tr("app.token_set") if self.client.token else tr("app.token_missing"))

    def _open_settings(self) -> None:
        SettingsDialog(self, self.settings, self.client.token, self._apply_settings)

    def _apply_settings(self, settings: Settings, token: str | None) -> None:
        language_changed = settings.language != self.settings.language
        binaries_changed = (settings.resolved_dll(), settings.resolved_injector()) != \
            (self.settings.resolved_dll(), self.settings.resolved_injector())
        self.settings = settings
        settings.save()
        ctk.set_appearance_mode(settings.appearance)
        self.client.token = token
        self.log(tr("app.settings_saved"))
        self._refresh_status()
        if binaries_changed:
            self._read_dll_build()
        if language_changed:
            messagebox.showinfo(tr("app.title"), tr("app.language_restart"), parent=self)

    def _confirm_running_games(self) -> bool:
        running = [item for item in self.processes.refresh() if item.kind == "injector" or item.sdk_loaded]
        if not running or self.settings.kill_existing:
            return True
        answer = messagebox.askyesnocancel(tr("app.title"), tr("app.running_games", count=len(running)),
                                           parent=self)
        if answer is None:
            return False
        if answer:
            self.processes.close_processes(running)
        return True

    def _launch(self) -> None:
        if not self._confirm_running_games():
            return
        try:
            result = launch(self.settings, self._injector_output)
        except LaunchError as exc:
            messagebox.showerror(tr("app.launch_failed_title"), str(exc), parent=self)
            return
        self._launch_errors = []
        self.events.disconnect()
        self.client.token = result.token
        command = build_command(self.settings, result.token is not None)
        self.log(f"[client] {describe_command(command)}")
        self.log(tr("app.token_created") if result.token else tr("app.auth_disabled"))
        self._refresh_status()
        self.tabs.set(tr("app.tab.activity"))

    def _close(self) -> None:
        self.events.disconnect()
        if self._sequence_dialog is not None and self._sequence_dialog.winfo_exists():
            self._sequence_dialog.stop()
        self.destroy()


def main() -> None:
    """Start the client."""
    app = ClientApp()
    app.mainloop()
