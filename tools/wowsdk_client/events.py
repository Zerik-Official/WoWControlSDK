"""Background reader for the SDK event pipe."""

# Standard library
from __future__ import annotations

import json
import queue
import threading
import time
from dataclasses import dataclass
from typing import Any

# Third-party
import pywintypes
import win32file
import win32pipe

EVENT_PIPE = r"\\.\pipe\WowGameEvent"

ERROR_FILE_NOT_FOUND = 2
ERROR_ACCESS_DENIED = 5
ERROR_BROKEN_PIPE = 109
ERROR_PIPE_BUSY = 231
ERROR_PIPE_NOT_CONNECTED = 233

POLL_INTERVAL_S = 0.05
READ_CHUNK = 65536


@dataclass(frozen=True)
class StreamMessage:
    """Something the event stream wants the interface to show."""

    kind: str
    text: str
    event: dict[str, Any] | None = None


class EventStream:
    """Connects to the event pipe on a worker thread and queues every line it reads."""

    def __init__(self) -> None:
        self.messages: queue.Queue[StreamMessage] = queue.Queue()
        self._thread: threading.Thread | None = None
        self._stop = threading.Event()

    @property
    def running(self) -> bool:
        """Whether the worker thread is alive."""
        return self._thread is not None and self._thread.is_alive()

    def start(self, token: str | None) -> None:
        """Connect and start reading; does nothing when already running."""
        if self.running:
            return
        self._stop.clear()
        self._thread = threading.Thread(target=self._run, args=(token,), name="event-stream", daemon=True)
        self._thread.start()

    def stop(self) -> None:
        """Ask the worker thread to disconnect."""
        self._stop.set()

    def _status(self, text: str) -> None:
        self.messages.put(StreamMessage("status", text))

    def _open(self) -> Any:
        try:
            return win32file.CreateFile(
                EVENT_PIPE,
                win32file.GENERIC_READ | win32file.GENERIC_WRITE,
                0,
                None,
                win32file.OPEN_EXISTING,
                0,
                None,
            )
        except pywintypes.error as exc:
            if exc.winerror == ERROR_ACCESS_DENIED:
                return win32file.CreateFile(EVENT_PIPE, win32file.GENERIC_READ, 0, None, win32file.OPEN_EXISTING, 0, None)
            raise

    def _run(self, token: str | None) -> None:
        try:
            handle = self._open()
        except pywintypes.error as exc:
            if exc.winerror == ERROR_FILE_NOT_FOUND:
                self._status("The event pipe does not exist: is the game running with the SDK?")
            elif exc.winerror == ERROR_PIPE_BUSY:
                self._status("The event pipe is busy: another client (for example the launcher) is connected")
            else:
                self._status(f"Could not open the event pipe: {exc.strerror}")
            self.messages.put(StreamMessage("stopped", ""))
            return

        try:
            if token:
                win32file.WriteFile(handle, (json.dumps({"auth": token}) + "\n").encode("utf-8"))
            self._status("Connected to the event pipe")
            self._read_loop(handle)
        except pywintypes.error as exc:
            if exc.winerror in (ERROR_BROKEN_PIPE, ERROR_PIPE_NOT_CONNECTED):
                self._status("The event pipe was closed")
            else:
                self._status(f"Event pipe error: {exc.strerror}")
        finally:
            win32file.CloseHandle(handle)
            self.messages.put(StreamMessage("stopped", ""))

    def _read_loop(self, handle: Any) -> None:
        buffer = ""
        while not self._stop.is_set():
            _, available, _ = win32pipe.PeekNamedPipe(handle, 0)
            if available == 0:
                time.sleep(POLL_INTERVAL_S)
                continue
            _, data = win32file.ReadFile(handle, min(available, READ_CHUNK))
            buffer += bytes(data).decode("utf-8", errors="replace")
            while "\n" in buffer:
                line, buffer = buffer.split("\n", 1)
                line = line.strip()
                if line:
                    self._emit(line)
        self._status("Disconnected from the event pipe")

    def _emit(self, line: str) -> None:
        try:
            parsed = json.loads(line)
        except json.JSONDecodeError:
            self.messages.put(StreamMessage("raw", line))
            return
        if isinstance(parsed, dict):
            name = str(parsed.get("event", ""))
            if name == "auth.failed":
                self._status("The SDK rejected the session token")
            self.messages.put(StreamMessage("event", name, parsed))
        else:
            self.messages.put(StreamMessage("raw", line))
