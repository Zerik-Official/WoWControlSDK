"""Colors, sizes and fonts shared by the interface."""

# Standard library
from __future__ import annotations

# Third-party
import customtkinter as ctk

ACCENT = "#d9a441"
ACCENT_HOVER = "#b8862f"
SUCCESS = "#4caf7a"
DANGER = "#d9534f"
MUTED = ("#6b6b6b", "#9a9a9a")
SURFACE = ("#ececec", "#1f1f23")
SURFACE_ALT = ("#e2e2e2", "#26262b")
SIDEBAR_WIDTH = 260

JSON_COLORS: dict[str, str] = {
    "key": "#7fb8e6",
    "string": "#a8d08d",
    "number": "#e6b673",
    "literal": "#c792ea",
}


def mono(size: int = 13) -> ctk.CTkFont:
    """Monospaced font for JSON and logs."""
    return ctk.CTkFont(family="Consolas", size=size)


def title(size: int = 18) -> ctk.CTkFont:
    """Bold font for headings."""
    return ctk.CTkFont(size=size, weight="bold")


def small(size: int = 12) -> ctk.CTkFont:
    """Font for hints and secondary text."""
    return ctk.CTkFont(size=size)
