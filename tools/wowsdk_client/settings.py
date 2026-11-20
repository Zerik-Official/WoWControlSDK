"""Client settings stored as JSON in the user's application data folder."""

# Standard library
from __future__ import annotations

import json
import os
import sys
from dataclasses import asdict, dataclass, fields
from pathlib import Path

APP_NAME = "WoWControlClient"


def app_dir() -> Path:
    """Folder that contains the client, or the repository root when running from source."""
    if getattr(sys, "frozen", False):
        return Path(sys.executable).resolve().parent
    return Path(__file__).resolve().parents[2]


def default_binary(name: str) -> str:
    """Path of an SDK binary shipped next to the client or built in the repository."""
    root = app_dir()
    for candidate in (root / "binaries" / name, root / "bin" / "release" / name):
        if candidate.is_file():
            return str(candidate)
    return ""


def settings_path() -> Path:
    """Location of the settings file."""
    base = os.environ.get("APPDATA") or str(Path.home())
    return Path(base) / APP_NAME / "settings.json"


@dataclass
class Settings:
    """Everything the user can configure in the client."""

    wow_path: str = ""
    dll_path: str = ""
    injector_path: str = ""
    use_auth: bool = True
    kill_existing: bool = True
    wow_args: str = "-console"
    open_console: bool = False
    show_logs_in_console: bool = False
    start_hidden: bool = False
    manual_map: bool = False
    auto_connect_events: bool = True
    appearance: str = "dark"
    language: str = ""

    def resolved_dll(self) -> str:
        """Configured DLL path, or the bundled one."""
        return self.dll_path or default_binary("WoWControlSDK.dll")

    def resolved_injector(self) -> str:
        """Configured injector path, or the bundled one."""
        return self.injector_path or default_binary("WowInjector.exe")

    @classmethod
    def load(cls) -> Settings:
        """Read the settings file, falling back to defaults for anything missing or invalid."""
        path = settings_path()
        try:
            data = json.loads(path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError):
            return cls()
        if not isinstance(data, dict):
            return cls()
        known = {item.name for item in fields(cls)}
        values = {key: value for key, value in data.items() if key in known}
        defaults = cls()
        for key, value in list(values.items()):
            if type(value) is not type(getattr(defaults, key)):
                del values[key]
        return cls(**values)

    def save(self) -> None:
        """Write the settings file."""
        path = settings_path()
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(asdict(self), indent=2), encoding="utf-8")
