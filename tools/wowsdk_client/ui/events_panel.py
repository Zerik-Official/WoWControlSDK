"""Live view of the SDK event pipe."""

# Standard library
from __future__ import annotations

import json
import queue
import time
from typing import Any, Callable

# Third-party
import customtkinter as ctk

# Project imports
from ..events import EventStream, StreamMessage
from ..i18n import tr
from . import theme
from .json_view import JsonView

POLL_MS = 100
MAX_PER_TICK = 200


class EventsPanel(ctk.CTkFrame):
    """Connects to the event pipe and lists events, with a name filter and pause."""

    def __init__(self, master: Any, token: Callable[[], str | None], log: Callable[[str], None],
                 **kwargs: Any) -> None:
        super().__init__(master, fg_color="transparent", **kwargs)
        self.grid_columnconfigure(3, weight=1)
        self.grid_rowconfigure(1, weight=1)
        self._token = token
        self._log = log
        self._stream = EventStream()
        self._paused = False
        self._count = 0

        self._toggle = ctk.CTkButton(self, text=tr("events.connect"), width=100, fg_color=theme.ACCENT,
                                     hover_color=theme.ACCENT_HOVER, command=self.toggle)
        self._toggle.grid(row=0, column=0, padx=(0, 6), pady=(0, 6))
        self._pause = ctk.CTkButton(self, text=tr("events.pause"), width=80, fg_color=theme.SURFACE_ALT,
                                    hover_color=theme.SURFACE, text_color=("#1a1a1a", "#e6e6e6"),
                                    command=self._toggle_pause)
        self._pause.grid(row=0, column=1, padx=(0, 6), pady=(0, 6))
        ctk.CTkButton(self, text=tr("events.clear"), width=70, fg_color=theme.SURFACE_ALT, hover_color=theme.SURFACE,
                      text_color=("#1a1a1a", "#e6e6e6"), command=self._clear).grid(row=0, column=2, padx=(0, 6),
                                                                                    pady=(0, 6))
        self._filter = ctk.CTkEntry(self, placeholder_text=tr("events.filter"))
        self._filter.grid(row=0, column=3, sticky="ew", pady=(0, 6))
        self._counter = ctk.CTkLabel(self, text=tr("events.count", count=0), font=theme.small(), text_color=theme.MUTED)
        self._counter.grid(row=0, column=4, padx=(8, 0), pady=(0, 6))

        self._view = JsonView(self)
        self._view.grid(row=1, column=0, columnspan=5, sticky="nsew")
        self.after(POLL_MS, self._poll)

    def toggle(self) -> None:
        """Connect or disconnect."""
        if self._stream.running:
            self._stream.stop()
            self._toggle.configure(text=tr("events.connect"))
        else:
            self._stream.start(self._token())
            self._toggle.configure(text=tr("events.disconnect"))

    def disconnect(self) -> None:
        """Stop reading events."""
        self._stream.stop()

    def _toggle_pause(self) -> None:
        self._paused = not self._paused
        self._pause.configure(text=tr("events.resume") if self._paused else tr("events.pause"))

    def _clear(self) -> None:
        self._view.clear()
        self._count = 0
        self._counter.configure(text=tr("events.count", count=0))

    def _poll(self) -> None:
        for _ in range(MAX_PER_TICK):
            try:
                message = self._stream.messages.get_nowait()
            except queue.Empty:
                break
            self._handle(message)
        self.after(POLL_MS, self._poll)

    def _handle(self, message: StreamMessage) -> None:
        if message.kind == "status":
            self._log(f"[events] {message.text}")
            return
        if message.kind == "stopped":
            self._toggle.configure(text=tr("events.connect"))
            return
        if self._paused:
            return
        query = self._filter.get().strip().lower()
        if query and query not in message.text.lower():
            return
        stamp = time.strftime("%H:%M:%S")
        if message.event is not None:
            args = json.dumps(message.event.get("args"), ensure_ascii=False)
            line = f"{stamp}  {message.text}  {args}"
        else:
            line = f"{stamp}  {message.text}"
        self._view.append_line(line)
        self._count += 1
        self._counter.configure(text=tr("events.count", count=self._count))
