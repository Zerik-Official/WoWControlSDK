"""Method browser: a rail of domain icons and the method cards of the selected domain."""

# Standard library
from __future__ import annotations

from typing import Any, Callable

# Third-party
import customtkinter as ctk

# Project imports
from ..catalog import DOMAINS, Domain, Method, find_domain
from ..i18n import tr
from . import theme
from .icons import icon
from .tooltip import tooltip

RAIL_BUTTON_SIZE = 36
RAIL_ICON_SIZE = 18
CARD_HEIGHT = 10
SEARCH_DELAY_MS = 150
CARD_WRAP = 210


class MethodCard(ctk.CTkFrame):
    """A clickable card with the method name and its description."""

    def __init__(self, master: Any, method: Method, show_domain: bool,
                 on_click: Callable[[Method], None]) -> None:
        super().__init__(master, height=CARD_HEIGHT, fg_color="transparent", corner_radius=8)
        self.method = method
        self._selected = False
        self.grid_columnconfigure(1, weight=1)

        self._bar = ctk.CTkFrame(self, width=3, height=CARD_HEIGHT, corner_radius=2, fg_color="transparent")
        self._bar.grid(row=0, column=0, rowspan=2, sticky="ns", padx=(2, 8), pady=6)

        title = method.name if show_domain else method.action
        self._title = ctk.CTkLabel(self, text=title, font=theme.title(13), anchor="w", text_color=theme.TEXT)
        self._title.grid(row=0, column=1, sticky="ew", pady=(6, 0), padx=(0, 8))
        self._description = ctk.CTkLabel(self, text=method.description, font=theme.small(11),
                                         text_color=theme.MUTED, anchor="w", justify="left",
                                         wraplength=CARD_WRAP)
        self._description.grid(row=1, column=1, sticky="ew", pady=(0, 6), padx=(0, 8))

        for widget in (self, self._bar, self._title, self._description):
            widget.bind("<Button-1>", lambda _event: on_click(method))
            widget.bind("<Enter>", lambda _event: self._hover(True))
            widget.bind("<Leave>", lambda _event: self._hover(False))

    def set_selected(self, selected: bool) -> None:
        """Highlight the card of the method shown in the form."""
        self._selected = selected
        self._bar.configure(fg_color=theme.ACCENT if selected else "transparent")
        self.configure(fg_color=theme.SURFACE_ALT if selected else "transparent")

    def _hover(self, inside: bool) -> None:
        if not self._selected:
            self.configure(fg_color=theme.SURFACE_ALT if inside else "transparent")


class Sidebar(ctk.CTkFrame):
    """Lets the user pick a domain on the rail and a method from its cards, or search every method."""

    def __init__(self, master: Any, on_select: Callable[[Method], None], **kwargs: Any) -> None:
        super().__init__(master, width=theme.SIDEBAR_WIDTH, corner_radius=0, fg_color=theme.SURFACE, **kwargs)
        self.grid_propagate(False)
        self.grid_rowconfigure(1, weight=1)
        self.grid_columnconfigure(1, weight=1)
        self._on_select = on_select
        self._domain: Domain = DOMAINS[0]
        self._selected: Method | None = None
        self._rail_buttons: dict[str, ctk.CTkButton] = {}
        self._cards: list[MethodCard] = []
        self._search_job: str | None = None

        self._build_search()
        self._build_rail()
        self._build_list()
        self._show_domain(self._domain)

    def _build_search(self) -> None:
        box = ctk.CTkFrame(self, fg_color=theme.SURFACE_ALT, corner_radius=8)
        box.grid(row=0, column=0, columnspan=2, sticky="ew", padx=12, pady=(12, 8))
        box.grid_columnconfigure(1, weight=1)
        ctk.CTkLabel(box, text="", image=icon("search", theme.MUTED, 16), width=28).grid(row=0, column=0,
                                                                                       padx=(6, 0))
        self._search = ctk.CTkEntry(box, placeholder_text=tr("sidebar.search"), border_width=0,
                                    fg_color="transparent")
        self._search.grid(row=0, column=1, sticky="ew", pady=2)
        self._search.bind("<KeyRelease>", lambda _event: self._schedule_search())
        self._clear = ctk.CTkButton(box, text="", image=icon("x", theme.MUTED, 14), width=26, height=26,
                                    fg_color="transparent", hover_color=theme.SURFACE, command=self._clear_search)
        tooltip(self._clear, tr("sidebar.clear_search"))

    def _build_rail(self) -> None:
        rail = ctk.CTkScrollableFrame(self, width=RAIL_BUTTON_SIZE + 8, fg_color="transparent")
        rail.grid(row=1, column=0, sticky="ns", padx=(8, 0), pady=(0, 8))
        scrollbar = getattr(rail, "_scrollbar", None)
        if scrollbar is not None:
            scrollbar.grid_remove()
        for row, domain in enumerate(DOMAINS):
            button = ctk.CTkButton(
                rail, text="", width=RAIL_BUTTON_SIZE, height=RAIL_BUTTON_SIZE, corner_radius=10,
                fg_color="transparent", hover_color=theme.SURFACE_ALT,
                command=lambda selected=domain: self._pick_domain(selected),
            )
            button.grid(row=row, column=0, pady=2)
            self._rail_buttons[domain.name] = button
            count = len(domain.methods)
            methods = tr("sidebar.count_one") if count == 1 else tr("sidebar.count", count=count)
            tooltip(button, f"{domain.title}\n{methods}")

    def _build_list(self) -> None:
        panel = ctk.CTkFrame(self, fg_color="transparent")
        panel.grid(row=1, column=1, sticky="nsew", padx=(6, 8), pady=(0, 8))
        panel.grid_rowconfigure(1, weight=1)
        panel.grid_columnconfigure(0, weight=1)

        header = ctk.CTkFrame(panel, fg_color="transparent")
        header.grid(row=0, column=0, sticky="ew", padx=4, pady=(2, 6))
        header.grid_columnconfigure(1, weight=1)
        self._header_icon = ctk.CTkLabel(header, text="", width=22)
        self._header_icon.grid(row=0, column=0, rowspan=2, padx=(0, 8))
        self._header_title = ctk.CTkLabel(header, text="", font=theme.title(15), anchor="w")
        self._header_title.grid(row=0, column=1, sticky="ew")
        self._header_count = ctk.CTkLabel(header, text="", font=theme.small(11), text_color=theme.MUTED,
                                          anchor="w")
        self._header_count.grid(row=1, column=1, sticky="ew")

        self._list = ctk.CTkScrollableFrame(panel, fg_color="transparent")
        self._list.grid(row=1, column=0, sticky="nsew")
        self._list.grid_columnconfigure(0, weight=1)

    def _paint_rail(self, active: str | None) -> None:
        for domain in DOMAINS:
            selected = domain.name == active
            self._rail_buttons[domain.name].configure(
                fg_color=theme.ACCENT if selected else "transparent",
                hover_color=theme.ACCENT_HOVER if selected else theme.SURFACE_ALT,
                image=icon(domain.icon, theme.ON_ACCENT if selected else theme.TEXT, RAIL_ICON_SIZE),
            )

    def _empty_search(self) -> None:
        self._search.delete(0, "end")
        self._clear.grid_remove()
        self.focus_set()
        restore_placeholder = getattr(self._search, "_activate_placeholder", None)
        if callable(restore_placeholder):
            restore_placeholder()

    def _pick_domain(self, domain: Domain) -> None:
        if self._search.get():
            self._empty_search()
        self._show_domain(domain)

    def _show_domain(self, domain: Domain) -> None:
        self._domain = domain
        self._paint_rail(domain.name)
        self._header_icon.configure(image=icon(domain.icon, theme.ACCENT, 22))
        self._header_title.configure(text=domain.title)
        count = len(domain.methods)
        self._header_count.configure(text=tr("sidebar.count_one") if count == 1 else tr("sidebar.count", count=count))
        self._fill(list(domain.methods), show_domain=False)

    def _fill(self, methods: list[Method], show_domain: bool) -> None:
        for child in self._list.winfo_children():
            child.destroy()
        self._cards = []

        if not methods:
            ctk.CTkLabel(self._list, text=tr("sidebar.no_results"), font=theme.small(12), text_color=theme.MUTED,
                         anchor="w").grid(row=0, column=0, sticky="ew", padx=6, pady=8)
            return

        for row, method in enumerate(methods):
            card = MethodCard(self._list, method, show_domain, self.select)
            card.grid(row=row, column=0, sticky="ew", pady=1)
            card.set_selected(self._selected is not None and method.name == self._selected.name)
            self._cards.append(card)
        canvas = getattr(self._list, "_parent_canvas", None)
        if canvas is not None:
            canvas.yview_moveto(0)

    def _schedule_search(self) -> None:
        if self._search_job is not None:
            self.after_cancel(self._search_job)
        self._search_job = self.after(SEARCH_DELAY_MS, self._run_search)

    def _run_search(self) -> None:
        self._search_job = None
        query = self._search.get().strip().lower()
        if not query:
            self._clear.grid_remove()
            self._show_domain(self._domain)
            return

        self._clear.grid(row=0, column=2, padx=(0, 4))
        matches = [method for domain in DOMAINS for method in domain.methods
                   if query in method.name.lower() or query in method.description.lower()]
        self._paint_rail(None)
        self._header_icon.configure(image=icon("search", theme.ACCENT, 22))
        self._header_title.configure(text=tr("sidebar.results_title"))
        self._header_count.configure(text=tr("sidebar.results_one") if len(matches) == 1
                                     else tr("sidebar.results", count=len(matches)))
        self._fill(matches, show_domain=True)

    def _clear_search(self) -> None:
        self._empty_search()
        self._run_search()

    def select(self, method: Method) -> None:
        """Highlight a method, switching to its domain when needed, and notify the owner."""
        self._selected = method
        if not self._search.get().strip() and method.domain != self._domain.name:
            self._show_domain(find_domain(method.domain) or self._domain)
        for card in self._cards:
            card.set_selected(card.method.name == method.name)
        self._on_select(method)

    def focus_search(self) -> None:
        """Move the keyboard focus to the search box and select its text."""
        self._search.focus_set()
        self._search.select_range(0, "end")

    def search(self, text: str) -> None:
        """Fill the search box and show its results."""
        self._empty_search()
        if text:
            self._search.insert(0, text)
        self._run_search()
