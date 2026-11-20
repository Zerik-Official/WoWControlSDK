"""
Sequences: lists of method calls that run one after another, each after an optional delay.

Secret parameters, such as passwords, stay in memory only. The sequence is saved without
them, so after reopening the client those steps need the secret again before running.
"""

# Standard library
from __future__ import annotations

import json
import os
import queue
import threading
import time
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Callable

# Project imports
from .catalog import Method, ParamType, find_method
from .i18n import tr
from .settings import APP_NAME
from .transport import RpcResult

FILE_VERSION = 1
DELAY_STEP_S = 0.1
MAX_DELAY_S = 3600.0

Caller = Callable[[str, dict[str, Any]], RpcResult]


def autosave_path() -> Path:
    """File where the current sequence is kept between sessions."""
    base = os.environ.get("APPDATA") or str(Path.home())
    return Path(base) / APP_NAME / "sequence.json"


@dataclass
class Step:
    """One call of a sequence."""

    method: str
    params: dict[str, Any] = field(default_factory=dict)
    delay_s: float = 0.0

    def catalog_method(self) -> Method | None:
        """The catalog entry of the method, if the client knows it."""
        return find_method(self.method)

    def missing_params(self) -> list[str]:
        """Required parameters without a value, such as a password that was not saved."""
        method = self.catalog_method()
        if method is None:
            return []
        return [param.name for param in method.params
                if param.required and param.type is not ParamType.BOOLEAN and param.name not in self.params]

    def summary(self, limit: int = 70) -> str:
        """Parameters as short text, with secret values hidden."""
        method = self.catalog_method()
        secrets = {param.name for param in method.params if param.type is ParamType.SECRET} if method else set()
        parts = []
        for name, value in self.params.items():
            shown = "•••" if name in secrets else json.dumps(value, ensure_ascii=False)
            parts.append(f"{name}={shown}")
        text = ", ".join(parts)
        return text if len(text) <= limit else text[:limit - 1] + "…"

    def to_json(self) -> dict[str, Any]:
        """The step as saved to disk, without secret values."""
        method = self.catalog_method()
        secrets = {param.name for param in method.params if param.type is ParamType.SECRET} if method else set()
        return {
            "method": self.method,
            "params": {name: value for name, value in self.params.items() if name not in secrets},
            "delay": self.delay_s,
        }

    @classmethod
    def from_json(cls, data: Any) -> Step | None:
        """A step read from disk, or ``None`` when it is malformed."""
        if not isinstance(data, dict) or not isinstance(data.get("method"), str):
            return None
        params = data.get("params") if isinstance(data.get("params"), dict) else {}
        try:
            delay = min(max(float(data.get("delay", 0.0)), 0.0), MAX_DELAY_S)
        except (TypeError, ValueError):
            delay = 0.0
        return cls(method=data["method"], params=params, delay_s=delay)


def save(steps: list[Step], path: Path) -> None:
    """Write a sequence file."""
    path.parent.mkdir(parents=True, exist_ok=True)
    data = {"version": FILE_VERSION, "steps": [step.to_json() for step in steps]}
    path.write_text(json.dumps(data, indent=2, ensure_ascii=False), encoding="utf-8")


def load(path: Path) -> list[Step]:
    """Read a sequence file; raises ``ValueError`` when it is not one."""
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise ValueError(str(exc)) from exc
    if not isinstance(data, dict) or not isinstance(data.get("steps"), list):
        raise ValueError(tr("sequence.invalid_file"))
    return [step for step in (Step.from_json(item) for item in data["steps"]) if step is not None]


@dataclass(frozen=True)
class RunUpdate:
    """Progress reported by a running sequence."""

    kind: str
    index: int = -1
    seconds: float = 0.0
    result: RpcResult | None = None
    text: str = ""


class SequenceRunner:
    """Runs a copy of some steps on a worker thread and queues its progress.

    ``indices`` chooses which steps run, in order; progress always reports the position of
    the step in the full sequence. ``use_delays`` set to ``False`` skips every delay, for
    example to retry one step at once.
    """

    def __init__(self, steps: list[Step], caller: Caller, stop_on_error: bool,
                 indices: list[int] | None = None, use_delays: bool = True) -> None:
        self.updates: queue.Queue[RunUpdate] = queue.Queue()
        chosen = indices if indices is not None else list(range(len(steps)))
        self.indices = [index for index in chosen if 0 <= index < len(steps)]
        self._steps = {index: Step(steps[index].method, dict(steps[index].params), steps[index].delay_s)
                       for index in self.indices}
        self._caller = caller
        self._stop_on_error = stop_on_error
        self._use_delays = use_delays
        self._stop = threading.Event()
        self._thread = threading.Thread(target=self._run, name="sequence", daemon=True)

    @property
    def running(self) -> bool:
        """Whether the worker thread is alive."""
        return self._thread.is_alive()

    def start(self) -> None:
        """Start running."""
        self._thread.start()

    def stop(self) -> None:
        """Stop before the next step; a call already sent still finishes."""
        self._stop.set()

    def _wait(self, index: int, seconds: float) -> bool:
        deadline = time.monotonic() + seconds
        while not self._stop.is_set():
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                return True
            self.updates.put(RunUpdate("waiting", index, seconds=remaining))
            self._stop.wait(min(DELAY_STEP_S, remaining))
        return False

    def _run(self) -> None:
        for index in self.indices:
            step = self._steps[index]
            if self._stop.is_set():
                self.updates.put(RunUpdate("stopped"))
                return
            if self._use_delays and step.delay_s > 0 and not self._wait(index, step.delay_s):
                self.updates.put(RunUpdate("stopped"))
                return
            missing = step.missing_params()
            if step.catalog_method() is None or missing:
                text = (tr("sequence.unknown_method", method=step.method) if step.catalog_method() is None
                        else tr("sequence.missing", names=", ".join(missing)))
                self.updates.put(RunUpdate("invalid", index, text=text))
                if self._stop_on_error:
                    self.updates.put(RunUpdate("stopped"))
                    return
                continue
            self.updates.put(RunUpdate("running", index))
            result = self._caller(step.method, step.params)
            self.updates.put(RunUpdate("result", index, result=result))
            if not result.ok and self._stop_on_error:
                self.updates.put(RunUpdate("stopped"))
                return
        self.updates.put(RunUpdate("finished"))
