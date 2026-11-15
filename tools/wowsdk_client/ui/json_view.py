"""Read-only text box that shows JSON with syntax colors."""

# Standard library
from __future__ import annotations

import json
import re
from typing import Any

# Third-party
import customtkinter as ctk

# Project imports
from . import theme

TOKEN_PATTERN = re.compile(
    r'(?P<key>"(?:\\.|[^"\\])*"(?=\s*:))'
    r'|(?P<string>"(?:\\.|[^"\\])*")'
    r"|(?P<number>-?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?)"
    r"|(?P<literal>\btrue\b|\bfalse\b|\bnull\b)"
)


class JsonView(ctk.CTkTextbox):
    """Text box that pretty-prints and colors JSON values."""

    def __init__(self, master: Any, **kwargs: Any) -> None:
        super().__init__(master, font=theme.mono(), wrap="none", **kwargs)
        for name, color in theme.JSON_COLORS.items():
            self.tag_config(name, foreground=color)
        self.configure(state="disabled")

    def show_value(self, value: Any) -> None:
        """Replace the content with a JSON value."""
        self.show_text(json.dumps(value, indent=2, ensure_ascii=False), colorize=True)

    def show_text(self, text: str, colorize: bool = False) -> None:
        """Replace the content with plain text, optionally colored as JSON."""
        self.configure(state="normal")
        self.delete("1.0", "end")
        self.insert("1.0", text)
        if colorize:
            self._colorize(text)
        self.configure(state="disabled")

    def append_line(self, text: str, max_lines: int = 5000) -> None:
        """Add a line at the end, keeping at most ``max_lines`` lines."""
        self.configure(state="normal")
        self.insert("end", text + "\n")
        lines = int(self.index("end-1c").split(".")[0])
        if lines > max_lines:
            self.delete("1.0", f"{lines - max_lines}.0")
        self.see("end")
        self.configure(state="disabled")

    def clear(self) -> None:
        """Remove all content."""
        self.show_text("")

    def text(self) -> str:
        """Current content."""
        return self.get("1.0", "end-1c")

    def _colorize(self, text: str) -> None:
        for match in TOKEN_PATTERN.finditer(text):
            kind = match.lastgroup
            if kind is None:
                continue
            self.tag_add(kind, f"1.0+{match.start()}c", f"1.0+{match.end()}c")
