"""Catalog of every SDK method the client can call, loaded from ``data/catalog``.

Each domain lives in its own JSON file with its methods, parameters and the English and
Spanish texts. ``index.json`` sets the order of the domains in the sidebar. Supporting a new
SDK method only needs a new entry in the file of its domain.
"""

# Standard library
from __future__ import annotations

import json
from dataclasses import dataclass, field
from enum import Enum
from pathlib import Path
from typing import Any

# Project imports
from .i18n import localized

CATALOG_DIR: Path = Path(__file__).resolve().parent / "data" / "catalog"

LocalizedText = dict[str, str]


class CatalogError(Exception):
    """A catalog file is missing or malformed."""


class ParamType(Enum):
    """Kind of input used for a parameter."""

    STRING = "string"
    TEXT = "text"
    SECRET = "secret"
    INTEGER = "integer"
    BOOLEAN = "boolean"
    STRING_LIST = "string_list"
    CHOICE = "choice"
    JSON = "json"


@dataclass(frozen=True)
class Param:
    """One parameter of a method."""

    name: str
    type: ParamType
    texts: LocalizedText
    required: bool = False
    default: str | int | bool | None = None
    choices: tuple[str, ...] = ()

    @property
    def description(self) -> str:
        """Description in the current language."""
        return localized(self.texts)


@dataclass(frozen=True)
class Method:
    """One SDK method."""

    name: str
    texts: LocalizedText
    params: tuple[Param, ...] = ()
    confirm_texts: LocalizedText | None = None

    @property
    def domain(self) -> str:
        """Domain part of the method name, for example ``group``."""
        return self.name.split(".", 1)[0]

    @property
    def action(self) -> str:
        """Method part of the name, for example ``getInvite``."""
        return self.name.split(".", 1)[1]

    @property
    def description(self) -> str:
        """Description in the current language."""
        return localized(self.texts)

    @property
    def confirm(self) -> str | None:
        """Confirmation question in the current language, if the method needs one."""
        return localized(self.confirm_texts) if self.confirm_texts else None


@dataclass(frozen=True)
class Domain:
    """A group of methods shown together in the sidebar."""

    name: str
    texts: LocalizedText
    icon: str
    methods: tuple[Method, ...] = field(default_factory=tuple)

    @property
    def title(self) -> str:
        """Title in the current language."""
        return localized(self.texts)


def _texts(value: Any, where: str) -> LocalizedText:
    if not isinstance(value, dict) or "en" not in value:
        raise CatalogError(f"{where} needs an object with at least an English text")
    return {str(code): str(text) for code, text in value.items()}


def _param(data: dict[str, Any], where: str) -> Param:
    return Param(
        name=str(data["name"]),
        type=ParamType(data["type"]),
        texts=_texts(data.get("description"), f"{where}.{data['name']}"),
        required=bool(data.get("required", False)),
        default=data.get("default"),
        choices=tuple(str(choice) for choice in data.get("choices", ())),
    )


def _method(data: dict[str, Any]) -> Method:
    name = str(data["name"])
    return Method(
        name=name,
        texts=_texts(data.get("description"), name),
        params=tuple(_param(item, name) for item in data.get("params", ())),
        confirm_texts=_texts(data["confirm"], f"{name}.confirm") if "confirm" in data else None,
    )


def _domain(name: str) -> Domain:
    path = CATALOG_DIR / f"{name}.json"
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
        return Domain(
            name=str(data["name"]),
            texts=_texts(data.get("title"), name),
            icon=str(data.get("icon", "")),
            methods=tuple(_method(item) for item in data.get("methods", ())),
        )
    except (OSError, json.JSONDecodeError, KeyError, ValueError) as exc:
        raise CatalogError(f"{path.name}: {exc}") from exc


def load_domains() -> tuple[Domain, ...]:
    """Every domain listed in ``index.json``, in order."""
    try:
        index = json.loads((CATALOG_DIR / "index.json").read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        raise CatalogError(f"index.json: {exc}") from exc
    return tuple(_domain(str(name)) for name in index.get("domains", ()))


DOMAINS: tuple[Domain, ...] = load_domains()


def all_methods() -> list[Method]:
    """Every method of every domain, in sidebar order."""
    return [method for domain in DOMAINS for method in domain.methods]


def find_method(name: str) -> Method | None:
    """The method with this exact name, if it exists."""
    return next((method for method in all_methods() if method.name == name), None)


def find_domain(name: str) -> Domain | None:
    """The domain with this name, if it exists."""
    return next((domain for domain in DOMAINS if domain.name == name), None)


TOKEN_PLACEHOLDER = "<session token>"
SECRET_PLACEHOLDER = "<hidden>"


def mask_request(request: dict[str, Any]) -> dict[str, Any]:
    """Copy of a request that is safe to show or copy: the token and secret parameters are replaced."""
    masked = dict(request)
    if "auth" in masked:
        masked["auth"] = TOKEN_PLACEHOLDER
    method = find_method(str(masked.get("method", "")))
    params = masked.get("params")
    if method is not None and isinstance(params, dict):
        secrets = {param.name for param in method.params if param.type is ParamType.SECRET}
        masked["params"] = {name: SECRET_PLACEHOLDER if name in secrets else value for name, value in params.items()}
    return masked
