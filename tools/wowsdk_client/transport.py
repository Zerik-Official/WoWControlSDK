"""JSON-RPC transport over the SDK command pipe."""

# Standard library
from __future__ import annotations

import itertools
import json
import threading
import time
from dataclasses import dataclass
from typing import Any

# Third-party
import pywintypes
import win32file
import win32pipe

# Project imports
from .i18n import tr

COMMAND_PIPE = r"\\.\pipe\WowGameCommand"

ERROR_FILE_NOT_FOUND = 2
ERROR_ACCESS_DENIED = 5
ERROR_SEM_TIMEOUT = 121
ERROR_PIPE_BUSY = 231
ERROR_BROKEN_PIPE = 109
ERROR_PIPE_NOT_CONNECTED = 233

READ_CHUNK = 65536
BUSY_WAIT_MS = 2000
CONNECT_ATTEMPTS = 3


class TransportError(Exception):
    """The command pipe could not be reached or answered badly."""


@dataclass(frozen=True)
class RpcResult:
    """A request and the response it produced."""

    request: dict[str, Any]
    response: dict[str, Any] | None
    error: str | None
    elapsed_ms: float
    offline: bool = False

    @property
    def ok(self) -> bool:
        """Whether the call returned a result without a protocol or method error."""
        if self.error is not None or self.response is None:
            return False
        if "error" in self.response:
            return False
        result = self.response.get("result")
        return not (isinstance(result, dict) and "error" in result)


class CommandClient:
    """Sends one JSON-RPC request per pipe connection, as the SDK expects."""

    def __init__(self, token: str | None = None) -> None:
        self._token = token
        self._ids = itertools.count(1)
        self._lock = threading.Lock()

    @property
    def token(self) -> str | None:
        """Session token added to every request, or ``None`` without authentication."""
        return self._token

    @token.setter
    def token(self, value: str | None) -> None:
        self._token = value or None

    def build_request(self, method: str, params: dict[str, Any] | None = None) -> dict[str, Any]:
        """The request object for a call, with the token when one is set."""
        with self._lock:
            request_id = next(self._ids)
        request: dict[str, Any] = {
            "jsonrpc": "2.0",
            "id": request_id,
            "method": method,
            "params": params or {},
        }
        if self._token and not method.startswith("auth."):
            request["auth"] = self._token
        return request

    def call(self, method: str, params: dict[str, Any] | None = None) -> RpcResult:
        """Send a request and wait for its response."""
        request = self.build_request(method, params)
        started = time.perf_counter()
        try:
            response = self._exchange(request)
            error = None
        except TransportError as exc:
            response = None
            error = str(exc)
        elapsed = (time.perf_counter() - started) * 1000
        return RpcResult(request=request, response=response, error=error, elapsed_ms=elapsed)

    def is_available(self) -> bool:
        """Whether the command pipe exists, even if it is busy serving another request."""
        try:
            win32pipe.WaitNamedPipe(COMMAND_PIPE, 1)
        except pywintypes.error as exc:
            return exc.winerror in (ERROR_PIPE_BUSY, ERROR_SEM_TIMEOUT)
        return True

    def _exchange(self, request: dict[str, Any]) -> dict[str, Any]:
        handle = self._open()
        try:
            win32file.WriteFile(handle, json.dumps(request).encode("utf-8"))
            raw = self._read_all(handle)
        except pywintypes.error as exc:
            raise TransportError(tr("transport.pipe_error", code=exc.winerror, error=exc.strerror)) from exc
        finally:
            win32file.CloseHandle(handle)

        text = raw.decode("utf-8", errors="replace").rstrip("\x00").strip()
        if not text:
            raise TransportError(tr("transport.empty"))
        try:
            parsed = json.loads(text)
        except json.JSONDecodeError as exc:
            raise TransportError(tr("transport.invalid_json", error=exc)) from exc
        if not isinstance(parsed, dict):
            raise TransportError(tr("transport.not_object"))
        return parsed

    @staticmethod
    def _open() -> Any:
        for _ in range(CONNECT_ATTEMPTS):
            try:
                return win32file.CreateFile(
                    COMMAND_PIPE,
                    win32file.GENERIC_READ | win32file.GENERIC_WRITE,
                    0,
                    None,
                    win32file.OPEN_EXISTING,
                    0,
                    None,
                )
            except pywintypes.error as exc:
                if exc.winerror == ERROR_PIPE_BUSY:
                    try:
                        win32pipe.WaitNamedPipe(COMMAND_PIPE, BUSY_WAIT_MS)
                    except pywintypes.error:
                        pass
                    continue
                if exc.winerror == ERROR_FILE_NOT_FOUND:
                    raise TransportError(tr("transport.not_running")) from exc
                if exc.winerror == ERROR_ACCESS_DENIED:
                    raise TransportError(tr("transport.access_denied")) from exc
                raise TransportError(tr("transport.open_failed", error=exc.strerror)) from exc
        raise TransportError(tr("transport.busy"))

    @staticmethod
    def _read_all(handle: Any) -> bytes:
        chunks: list[bytes] = []
        while True:
            try:
                _, data = win32file.ReadFile(handle, READ_CHUNK)
            except pywintypes.error as exc:
                if exc.winerror in (ERROR_BROKEN_PIPE, ERROR_PIPE_NOT_CONNECTED):
                    break
                raise
            if not data:
                break
            chunks.append(bytes(data))
        return b"".join(chunks)
