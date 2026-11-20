"""Start the game with WowInjector.exe and a fresh session token."""

# Standard library
from __future__ import annotations

import json
import queue
import re
import secrets
import shlex
import subprocess
import threading
from dataclasses import dataclass
from pathlib import Path

# Project imports
from .i18n import tr
from .settings import Settings

TOKEN_BYTES = 32
PID_PATTERN = re.compile(r"\[injector\] pid=(\d+)")
FAILURE_HINTS: tuple[tuple[str, str], ...] = (
    ("did not open its pipe", "hint.pipe_timeout"),
    ("timeout waiting for pipe", "hint.pipe_timeout"),
    ("game closed while the sdk was starting", "hint.game_closed"),
    ("loadlibrarya returned null", "hint.load_library"),
    ("loadlibrarya return null", "hint.load_library"),
    ("createprocess", "hint.create_process"),
    ("auth.init rejected", "hint.auth_rejected"),
    ("auth.init failed", "hint.auth_unavailable"),
    ("token must be", "hint.token"),
    ("window.setstarthidden failed", "hint.hidden"),
)


class LaunchError(Exception):
    """The game could not be started."""


@dataclass(frozen=True)
class InjectorOutput:
    """A line printed by the injector, or its exit code when ``line`` is ``None``."""

    line: str | None = None
    exit_code: int | None = None


@dataclass(frozen=True)
class LaunchResult:
    """A running injector process and the token it registered."""

    process: subprocess.Popen[bytes]
    token: str | None


def new_token() -> str:
    """A random session token of 64 hexadecimal characters."""
    return secrets.token_hex(TOKEN_BYTES)


def build_command(settings: Settings, use_token: bool) -> list[str]:
    """Injector command line for these settings; the token itself goes through stdin."""
    command = [
        settings.resolved_injector(),
        "--wow", settings.wow_path,
        "--dll", settings.resolved_dll(),
        "--token-stdin" if use_token else "--no-auth",
    ]
    if settings.manual_map:
        command.append("--manual-map")
    if settings.kill_existing:
        command.append("--kill")
    if settings.start_hidden:
        command.append("--hidden")
    if settings.wow_args.strip():
        command += ["--wow-args", settings.wow_args.strip()]
    if settings.open_console:
        command += ["--pipe-cmd", json.dumps({"jsonrpc": "2.0", "id": 1, "method": "dll.toggleConsole", "params": {}})]
    if settings.show_logs_in_console:
        command += ["--pipe-cmd", json.dumps(
            {"jsonrpc": "2.0", "id": 2, "method": "dll.setLogConfig", "params": {"showInConsole": True}}
        )]
    return command


def describe_command(command: list[str]) -> str:
    """Command line as text for the activity log."""
    return " ".join(shlex.quote(part) for part in command)


def validate(settings: Settings) -> None:
    """Fail early with a clear message when a required file is missing."""
    checks = (
        ("Wow.exe", settings.wow_path),
        ("WoWControlSDK.dll", settings.resolved_dll()),
        ("WowInjector.exe", settings.resolved_injector()),
    )
    for label, path in checks:
        if not path:
            raise LaunchError(tr("launch.path_missing", label=label))
        if not Path(path).is_file():
            raise LaunchError(tr("launch.file_missing", label=label, path=path))


def launch(settings: Settings, output: queue.Queue[InjectorOutput]) -> LaunchResult:
    """Start the injector and forward its output lines to ``output``."""
    validate(settings)
    token = new_token() if settings.use_auth else None
    command = build_command(settings, token is not None)
    try:
        process = subprocess.Popen(
            command,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            creationflags=subprocess.CREATE_NO_WINDOW,
        )
    except OSError as exc:
        raise LaunchError(tr("launch.start_failed", error=exc)) from exc

    if process.stdin is not None:
        if token is not None:
            process.stdin.write((token + "\n").encode("ascii"))
        process.stdin.close()

    threading.Thread(target=_forward_output, args=(process, output), name="injector-output", daemon=True).start()
    return LaunchResult(process=process, token=token)


def _forward_output(process: subprocess.Popen[bytes], output: queue.Queue[InjectorOutput]) -> None:
    if process.stdout is not None:
        for raw in process.stdout:
            line = raw.decode("utf-8", errors="replace").rstrip()
            if line:
                output.put(InjectorOutput(line=line))
    output.put(InjectorOutput(exit_code=process.wait()))


def is_error_line(line: str) -> bool:
    """Whether an injector line reports a failure."""
    lowered = line.lower()
    return any(word in lowered for word in ("error", "failed", "rejected"))


def diagnose(lines: list[str]) -> str:
    """Explanation for a failed launch, based on the injector's error lines."""
    text = " ".join(lines).lower()
    for pattern, key in FAILURE_HINTS:
        if pattern in text:
            return tr(key)
    return tr("hint.unknown")


def parse_pid(line: str) -> int | None:
    """Game process id from an injector output line."""
    match = PID_PATTERN.search(line)
    return int(match.group(1)) if match else None
