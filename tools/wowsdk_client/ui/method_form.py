"""Form generated from a catalog method."""

# Standard library
from __future__ import annotations

import json
from typing import Any, Callable

# Third-party
import customtkinter as ctk

# Project imports
from ..catalog import Method, Param, ParamType, find_domain
from ..i18n import tr
from . import theme
from .icons import icon



class FormError(Exception):
    """A field holds a value that cannot be sent."""


class ParamField:
    """Input widget for one parameter and the logic to read its value."""

    def __init__(self, master: Any, param: Param, on_submit: Callable[[], None]) -> None:
        self.param = param
        self.frame = ctk.CTkFrame(master, fg_color="transparent")
        self.frame.grid_columnconfigure(0, weight=1)

        label = param.name + (" *" if param.required else "")
        ctk.CTkLabel(self.frame, text=label, font=theme.title(13), anchor="w").grid(row=0, column=0, sticky="w")
        ctk.CTkLabel(
            self.frame, text=param.description, font=theme.small(), text_color=theme.MUTED, anchor="w",
            justify="left", wraplength=520,
        ).grid(row=1, column=0, sticky="w")

        self.widget: Any = self._build_input(on_submit)
        self.widget.grid(row=2, column=0, sticky="ew", pady=(4, 0))

    def _build_input(self, on_submit: Callable[[], None]) -> Any:
        param = self.param
        if param.type is ParamType.BOOLEAN:
            switch = ctk.CTkSwitch(self.frame, text="", progress_color=theme.ACCENT)
            if param.default is True:
                switch.select()
            return switch
        if param.type is ParamType.CHOICE:
            values = [choice or tr("form.default") for choice in param.choices]
            menu = ctk.CTkOptionMenu(self.frame, values=values, fg_color=theme.SURFACE_ALT,
                                     button_color=theme.ACCENT, button_hover_color=theme.ACCENT_HOVER)
            menu.set(str(param.default) if param.default else tr("form.default"))
            return menu
        if param.type is ParamType.TEXT:
            box = ctk.CTkTextbox(self.frame, height=140, font=theme.mono())
            if param.default is not None:
                box.insert("1.0", str(param.default))
            return box
        entry = ctk.CTkEntry(self.frame, show="•" if param.type is ParamType.SECRET else "")
        if param.default is not None:
            entry.insert(0, str(param.default))
        entry.bind("<Return>", lambda _event: on_submit())
        return entry

    def raw(self) -> str:
        """Current text of the input."""
        if self.param.type is ParamType.TEXT:
            return self.widget.get("1.0", "end-1c")
        if self.param.type is ParamType.CHOICE:
            value = self.widget.get()
            return "" if value == tr("form.default") else value
        return self.widget.get()

    def value(self) -> tuple[bool, Any]:
        """Whether the parameter should be sent, and its value."""
        param = self.param
        if param.type is ParamType.BOOLEAN:
            return True, bool(self.widget.get())

        text = self.raw()
        if param.type is not ParamType.TEXT:
            text = text.strip()
        if not text.strip():
            if param.required:
                raise FormError(tr("form.required", name=param.name))
            return False, None

        if param.type is ParamType.INTEGER:
            try:
                return True, int(text)
            except ValueError as exc:
                raise FormError(tr("form.integer", name=param.name)) from exc
        if param.type is ParamType.STRING_LIST:
            items = [item.strip() for item in text.split(",") if item.strip()]
            if param.required and not items:
                raise FormError(tr("form.one_value", name=param.name))
            return True, items
        if param.type is ParamType.JSON:
            try:
                return True, json.loads(text)
            except json.JSONDecodeError:
                return True, text
        return True, text

    def focus(self) -> None:
        """Move the keyboard focus to the input."""
        self.widget.focus_set()


class MethodForm(ctk.CTkScrollableFrame):
    """Shows the selected method and the inputs for its parameters."""

    def __init__(self, master: Any, on_submit: Callable[[], None], **kwargs: Any) -> None:
        super().__init__(master, fg_color="transparent", **kwargs)
        self.grid_columnconfigure(0, weight=1)
        self._on_submit = on_submit
        self._fields: list[ParamField] = []
        self.method: Method | None = None

        self._title = ctk.CTkLabel(self, text=tr("form.choose"), font=theme.title(20), anchor="w")
        self._title.grid(row=0, column=0, sticky="w", pady=(0, 2))
        self._description = ctk.CTkLabel(self, text=tr("form.choose_hint"),
                                         font=theme.small(13), text_color=theme.MUTED, anchor="w",
                                         justify="left", wraplength=560)
        self._description.grid(row=1, column=0, sticky="w", pady=(0, 12))
        self._fields_frame = ctk.CTkFrame(self, fg_color="transparent")
        self._fields_frame.grid(row=2, column=0, sticky="ew")
        self._fields_frame.grid_columnconfigure(0, weight=1)

    def show(self, method: Method) -> None:
        """Rebuild the form for a method."""
        self.method = method
        domain = find_domain(method.domain)
        self._title.configure(text=f"  {method.name}", compound="left",
                              image=icon(domain.icon, theme.ACCENT, 22) if domain else None)
        self._description.configure(text=method.description)
        for child in self._fields_frame.winfo_children():
            child.destroy()
        self._fields = []
        if not method.params:
            ctk.CTkLabel(self._fields_frame, text=tr("form.no_params"),
                         font=theme.small(13), text_color=theme.MUTED, anchor="w").grid(row=0, column=0, sticky="w")
            return
        for row, param in enumerate(method.params):
            field = ParamField(self._fields_frame, param, self._on_submit)
            field.frame.grid(row=row, column=0, sticky="ew", pady=(0, 12))
            self._fields.append(field)
        self._fields[0].focus()

    def params(self) -> dict[str, Any]:
        """Parameters to send, validated."""
        values: dict[str, Any] = {}
        for field in self._fields:
            include, value = field.value()
            if include:
                values[field.param.name] = value
        return values
