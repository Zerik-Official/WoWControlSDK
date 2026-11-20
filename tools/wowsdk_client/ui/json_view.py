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
        self.tag_config("header", foreground=theme.ACCENT)
        self.tag_config("success", foreground=theme.SUCCESS)
        self.tag_config("danger", foreground=theme.DANGER)
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

    def append_block(self, header: str, value: Any = None, text: str | None = None, tag: str = "header") -> None:
        """Add a colored header line followed by a JSON value or a plain text, and scroll to it."""
        self.configure(state="normal")
        self.insert("end", header + "\n", (tag,))
        if text is not None or value is not None:
            body = text if text is not None else json.dumps(value, indent=2, ensure_ascii=False)
            start = self.index("end-1c")
            self.insert("end", body + "\n")
            if text is None:
                self._colorize(body, start)
        self.insert("end", "\n")
        self.see("end")
        self.configure(state="disabled")

    def clear(self) -> None:
        """Remove all content."""
        self.show_text("")

    def text(self) -> str:
        """Current content."""
        return self.get("1.0", "end-1c")

    def _colorize(self, text: str, start: str = "1.0") -> None:
        for match in TOKEN_PATTERN.finditer(text):
            kind = match.lastgroup
            if kind is None:
                continue
            self.tag_add(kind, f"{start}+{match.start()}c", f"{start}+{match.end()}c")
