"""
Calls to the SDK DLL without the game, through ``WowInjector.exe --call``.

The DLL is 32-bit, so this 64-bit process cannot load it. The injector loads it in its own
process, where the DLL starts no hooks or pipes and only answers the methods that work
outside the game.
"""

# Standard library
from __future__ import annotations

import json
import subprocess
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Any

# Project imports
from .i18n import tr
from .settings import Settings
from .transport import RpcResult

OFFLINE_METHODS: frozenset[str] = frozenset({"dll.getInfo"})
CALL_TIMEOUT_S = 15


@dataclass(frozen=True)
class DllBuild:
    """Version and commit reported by the DLL."""

    version: str
    commit: str | None
    commit_date: str | None
    dirty: bool

    @property
    def label(self) -> str:
        """Short text such as ``2.3.0 · 4f09723``."""
        if not self.commit:
            return self.version
        return f"{self.version} · {self.commit[:7]}{'-dirty' if self.dirty else ''}"


def supports(method: str) -> bool:
    """Whether a method can run without the game."""
    return method in OFFLINE_METHODS


def call(settings: Settings, method: str) -> RpcResult:
    """Run a method in the DLL through the injector and return its response."""
    request: dict[str, Any] = {"jsonrpc": "2.0", "id": 1, "method": method, "params": {}}
    injector = settings.resolved_injector()
    dll = settings.resolved_dll()
    started = time.perf_counter()

    def finish(response: dict[str, Any] | None, error: str | None) -> RpcResult:
        return RpcResult(request=request, response=response, error=error,
                         elapsed_ms=(time.perf_counter() - started) * 1000, offline=True)

    for label, path in (("WowInjector.exe", injector), ("WoWControlSDK.dll", dll)):
        if not path or not Path(path).is_file():
            return finish(None, tr("offline.file_missing", label=label))

    try:
        completed = subprocess.run(
            [injector, "--dll", dll, "--call", method],
            capture_output=True,
            timeout=CALL_TIMEOUT_S,
            creationflags=subprocess.CREATE_NO_WINDOW,
        )
    except (OSError, subprocess.TimeoutExpired) as exc:
        return finish(None, tr("offline.failed", error=exc))

    output = completed.stdout.decode("utf-8", errors="replace").strip()
    line = next((item for item in reversed(output.splitlines()) if item.startswith("{")), "")
    if not line:
        detail = completed.stderr.decode("utf-8", errors="replace").strip() or output
        return finish(None, tr("offline.no_response", detail=detail or completed.returncode))
    try:
        parsed = json.loads(line)
    except json.JSONDecodeError as exc:
        return finish(None, tr("transport.invalid_json", error=exc))
    return finish(parsed if isinstance(parsed, dict) else None,
                  None if isinstance(parsed, dict) else tr("transport.not_object"))


def read_build(settings: Settings) -> DllBuild | None:
    """Version and commit of the configured DLL, or ``None`` when they cannot be read."""
    result = call(settings, "dll.getInfo")
    if not result.ok or result.response is None:
        return None
    info = result.response.get("result")
    if not isinstance(info, dict) or not isinstance(info.get("version"), str):
        return None
    commit = info.get("commit") if isinstance(info.get("commit"), dict) else {}
    return DllBuild(
        version=info["version"],
        commit=commit.get("hash"),
        commit_date=commit.get("date"),
        dirty=bool(commit.get("dirty", False)),
    )
