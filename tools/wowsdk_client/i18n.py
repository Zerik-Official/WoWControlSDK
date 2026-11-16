"""Interface texts in English and Spanish, loaded from ``data/locales``."""

# Standard library
from __future__ import annotations

import ctypes
import json
from functools import cache
from pathlib import Path

LANGUAGES: dict[str, str] = {"en": "English", "es": "Español"}
FALLBACK_LANGUAGE: str = "en"
LOCALES_DIR: Path = Path(__file__).resolve().parent / "data" / "locales"
SPANISH_PRIMARY_ID: int = 0x0A
PRIMARY_LANGUAGE_MASK: int = 0x3FF

_current: str = FALLBACK_LANGUAGE


@cache
def _texts(code: str) -> dict[str, str]:
    """Every text of one language, keyed by its identifier."""
    path = LOCALES_DIR / f"{code}.json"
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return {}
    return {str(key): str(value) for key, value in data.items()} if isinstance(data, dict) else {}


def system_language() -> str:
    """Language code of the Windows interface, falling back to English."""
    try:
        language_id = ctypes.windll.kernel32.GetUserDefaultUILanguage()
    except (AttributeError, OSError):
        return FALLBACK_LANGUAGE
    return "es" if language_id & PRIMARY_LANGUAGE_MASK == SPANISH_PRIMARY_ID else FALLBACK_LANGUAGE


def set_language(code: str) -> None:
    """Use a language code, or the system language when the code is empty or unknown."""
    global _current
    _current = code if code in LANGUAGES else system_language()


def language() -> str:
    """Language code in use."""
    return _current


def localized(texts: dict[str, str]) -> str:
    """The entry of a ``{"en": ..., "es": ...}`` mapping for the current language."""
    return texts.get(_current) or texts.get(FALLBACK_LANGUAGE) or next(iter(texts.values()), "")


def tr(key: str, **values: object) -> str:
    """Text for a key in the current language, with ``{name}`` placeholders filled in."""
    text = _texts(_current).get(key) or _texts(FALLBACK_LANGUAGE).get(key) or key
    return text.format(**values) if values else text
