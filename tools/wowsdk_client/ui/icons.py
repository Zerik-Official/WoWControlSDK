"""Tinted icons for the interface, built from the white PNG files in ``data/icons``."""

# Standard library
from __future__ import annotations

from functools import cache
from pathlib import Path

# Third-party
import customtkinter as ctk
from PIL import Image

ICONS_DIR: Path = Path(__file__).resolve().parents[1] / "data" / "icons"

ThemeColor = str | tuple[str, str]


@cache
def _source(name: str) -> Image.Image | None:
    """The white icon, or ``None`` when the file is missing."""
    path = ICONS_DIR / f"{name}.png"
    try:
        return Image.open(path).convert("RGBA")
    except OSError:
        return None


@cache
def _tinted(name: str, color: str) -> Image.Image | None:
    """The icon painted in one color, keeping its transparency."""
    source = _source(name)
    if source is None:
        return None
    painted = Image.new("RGBA", source.size, color)
    painted.putalpha(source.getchannel("A"))
    return painted


@cache
def icon(name: str, color: ThemeColor, size: int = 18) -> ctk.CTkImage | None:
    """A CTkImage of the icon, with one color or a (light, dark) pair of colors."""
    light, dark = (color, color) if isinstance(color, str) else color
    light_image = _tinted(name, light)
    dark_image = _tinted(name, dark)
    if light_image is None or dark_image is None:
        return None
    return ctk.CTkImage(light_image=light_image, dark_image=dark_image, size=(size, size))
