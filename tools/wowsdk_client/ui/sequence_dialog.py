"""Window that orders, times and runs the steps of a sequence."""

# Standard library
from __future__ import annotations

import queue
import time
import tkinter as tk
from pathlib import Path
from tkinter import filedialog, messagebox
from typing import Any, Protocol

# Third-party
import customtkinter as ctk

# Project imports
from ..catalog import find_domain, mask_request
from ..i18n import tr
from ..sequence import MAX_DELAY_S, RunUpdate, SequenceRunner, Step, load, save
from ..transport import RpcResult
from . import theme
from .choice_dialog import choose
from .chrome import WindowChrome
from .icons import icon
from .json_view import JsonView
from .tooltip import tooltip

POLL_MS = 80
ROW_WRAP = 300
SIZE = (800, 600)
MIN_SIZE = (640, 420)
FILE_TYPES = [("JSON", "*.json")]
GHOST_ALPHA = 0.88
AUTOSCROLL_MARGIN = 36


class SequenceHost(Protocol):
    """What the dialog needs from the main window."""

    sequence: list[Step]

    def sequence_changed(self, rebuild: bool = True) -> None:
        """The steps were added, removed, reordered or edited; ``rebuild`` redraws the dialog."""

    def call_method(self, method: str, params: dict[str, Any]) -> RpcResult:
        """Send one call and wait for its result; runs on a worker thread."""

    def show_sequence_result(self, method: str, result: RpcResult) -> None:
        """Show a finished step in the response view and the history."""

    def edit_step(self, index: int) -> None:
        """Load a step into the method form to change it."""


def _format_delay(seconds: float) -> str:
    return f"{seconds:g}"


class StepRow(ctk.CTkFrame):
    """One block of the list: grip, method, parameters, delay, status and actions."""

    def __init__(self, dialog: SequenceDialog, index: int, step: Step) -> None:
        super().__init__(dialog.list, fg_color=theme.SURFACE_ALT, corner_radius=8, border_width=2,
                         border_color=theme.SURFACE_ALT)
        self.index = index
        self.step = step
        self.grid_columnconfigure(2, weight=1)

        self.grip = ctk.CTkLabel(self, text="", image=icon("grip", theme.MUTED, 18), width=26, cursor="fleur")
        self.grip.grid(row=0, column=0, rowspan=2, padx=(6, 0), pady=6)
        tooltip(self.grip, tr("sequence.drag"))
        self.grip.bind("<ButtonPress-1>", lambda event: dialog.drag_start(self, event))
        self.grip.bind("<B1-Motion>", dialog.drag_motion)
        self.grip.bind("<ButtonRelease-1>", dialog.drag_end)

        self.number = ctk.CTkLabel(self, text=f"{index + 1}", font=theme.title(14), width=26, text_color=theme.MUTED)
        self.number.grid(row=0, column=1, rowspan=2, padx=(2, 6))

        method = step.catalog_method()
        domain = find_domain(method.domain) if method else None
        self.title = ctk.CTkLabel(self, text=f"  {step.method}", font=theme.title(13), anchor="w", compound="left",
                                  image=icon(domain.icon, theme.ACCENT, 16) if domain else None, cursor="hand2")
        self.title.grid(row=0, column=2, sticky="ew", pady=(6, 0))
        self.title.bind("<Button-1>", lambda _event: dialog.show_row_result(self.index))
        missing = step.missing_params()
        summary = tr("sequence.missing", names=", ".join(missing)) if missing else (
            step.summary() or tr("sequence.no_params"))
        ctk.CTkLabel(self, text=summary, font=theme.mono(11), anchor="w", justify="left", wraplength=ROW_WRAP,
                     text_color=theme.DANGER if missing else theme.MUTED).grid(row=1, column=2, sticky="ew",
                                                                                 pady=(0, 6))

        delay_box = ctk.CTkFrame(self, fg_color="transparent")
        delay_box.grid(row=0, column=3, rowspan=2, padx=6)
        timer = ctk.CTkLabel(delay_box, text="", image=icon("timer", theme.MUTED, 16), width=20)
        timer.grid(row=0, column=0)
        self.delay = ctk.CTkEntry(delay_box, width=58, justify="right")
        self.delay.insert(0, _format_delay(step.delay_s))
        self.delay.grid(row=0, column=1, padx=(2, 2))
        ctk.CTkLabel(delay_box, text="s", text_color=theme.MUTED).grid(row=0, column=2)
        for widget in (timer, self.delay):
            tooltip(widget, tr("sequence.delay_hint"))
        self.delay.bind("<FocusOut>", lambda _event: dialog.set_delay(self.index, self.delay.get()))
        self.delay.bind("<Return>", lambda _event: dialog.set_delay(self.index, self.delay.get()))

        self.status = ctk.CTkLabel(self, text="", font=theme.small(12), width=118, anchor="e")
        self.status.grid(row=0, column=4, rowspan=2, padx=(0, 4))

        self.retry = ctk.CTkButton(self, text="", image=icon("rotate-ccw", theme.ACCENT, 16), width=30, height=30,
                                   fg_color="transparent", hover_color=theme.SURFACE,
                                   command=lambda: dialog.retry(self.index))
        self.retry.grid(row=0, column=5, rowspan=2)
        self.retry.grid_remove()
        tooltip(self.retry, tr("sequence.retry"))
        edit = ctk.CTkButton(self, text="", image=icon("pencil", theme.TEXT, 16), width=30, height=30,
                             fg_color="transparent", hover_color=theme.SURFACE,
                             command=lambda: dialog.edit(self.index))
        edit.grid(row=0, column=6, rowspan=2)
        tooltip(edit, tr("sequence.edit"))
        remove = ctk.CTkButton(self, text="", image=icon("trash", theme.DANGER, 16), width=30, height=30,
                               fg_color="transparent", hover_color=theme.SURFACE,
                               command=lambda: dialog.remove(self.index))
        remove.grid(row=0, column=7, rowspan=2, padx=(0, 6))
        tooltip(remove, tr("sequence.remove"))

    def set_status(self, text: str, color: Any, can_retry: bool) -> None:
        """Show the progress of this step and whether it can be retried."""
        self.status.configure(text=text, text_color=color)
        if can_retry:
            self.retry.grid()
        else:
            self.retry.grid_remove()

    def set_position(self, position: int) -> None:
        """Place the block at a position of the list and number it."""
        self.grid(row=position, column=0, sticky="ew", pady=3)
        self.number.configure(text=f"{position + 1}")

    def set_placeholder(self, active: bool) -> None:
        """Show the block as the gap where the dragged step will land."""
        self.configure(fg_color="transparent" if active else theme.SURFACE_ALT,
                       border_color=theme.ACCENT if active else theme.SURFACE_ALT)


class SequenceDialog(ctk.CTkToplevel):
    """Lists the steps as blocks that can be dragged, timed, edited and run in order, with a console."""

    def __init__(self, master: Any, host: SequenceHost) -> None:
        super().__init__(master)
        self.transient(master)
        self._host = host
        self._rows: list[StepRow] = []
        self._results: dict[int, RpcResult] = {}
        self._statuses: dict[int, tuple[str, Any]] = {}
        self._done: set[int] = set()
        self._selected: list[int] = []
        self._runner: SequenceRunner | None = None
        self._dragged: StepRow | None = None
        self._order: list[StepRow] = []
        self._ghost: tk.Toplevel | None = None
        self._ghost_offset = (0, 0)
        self._run_started = 0.0

        self.chrome = WindowChrome(self, tr("sequence.title"), resizable=True, minimizable=False, taskbar=False,
                                   on_close=self._close, min_size=MIN_SIZE)
        body = self.chrome.body
        body.grid_columnconfigure(0, weight=1)
        body.grid_rowconfigure(1, weight=1)

        header = ctk.CTkFrame(body, fg_color="transparent")
        header.grid(row=0, column=0, sticky="ew", padx=16, pady=(12, 0))
        header.grid_columnconfigure(0, weight=1)
        self._title = ctk.CTkLabel(header, text="", font=theme.title(17), anchor="w", compound="left",
                                   image=icon("list-ordered", theme.ACCENT, 20))
        self._title.grid(row=0, column=0, sticky="w")
        ctk.CTkLabel(header, text=tr("sequence.hint"), font=theme.small(12), text_color=theme.MUTED, anchor="w",
                     justify="left", wraplength=SIZE[0] - 60).grid(row=1, column=0, sticky="w", pady=(2, 0))

        self.tabs = ctk.CTkTabview(body, segmented_button_selected_color=theme.ACCENT,
                                   segmented_button_selected_hover_color=theme.ACCENT_HOVER)
        self.tabs.grid(row=1, column=0, sticky="nsew", padx=12)
        steps_tab = self.tabs.add(tr("sequence.tab.steps"))
        console_tab = self.tabs.add(tr("sequence.tab.console"))
        for tab in (steps_tab, console_tab):
            tab.grid_columnconfigure(0, weight=1)
            tab.grid_rowconfigure(0, weight=1)

        self.list = ctk.CTkScrollableFrame(steps_tab, fg_color="transparent")
        self.list.grid(row=0, column=0, sticky="nsew")
        self.list.grid_columnconfigure(0, weight=1)

        self.console = JsonView(console_tab)
        self.console.grid(row=0, column=0, sticky="nsew")
        console_bar = ctk.CTkFrame(console_tab, fg_color="transparent")
        console_bar.grid(row=1, column=0, sticky="e", pady=(6, 0))
        for column, (name, key, command) in enumerate((("copy", "sequence.console_copy", self._copy_console),
                                                       ("trash", "sequence.console_clear", self.console.clear))):
            button = ctk.CTkButton(console_bar, text="", image=icon(name, theme.TEXT, 16), width=34, height=30,
                                   fg_color=theme.SURFACE_ALT, hover_color=theme.SURFACE, command=command)
            button.grid(row=0, column=column, padx=(6, 0))
            tooltip(button, tr(key))

        footer = ctk.CTkFrame(body, fg_color="transparent")
        footer.grid(row=2, column=0, sticky="ew", padx=16, pady=(8, 14))
        footer.grid_columnconfigure(3, weight=1)
        self._run = ctk.CTkButton(footer, text=tr("sequence.run"), width=130, image=icon("play", theme.ON_ACCENT, 16),
                                  compound="left", text_color=theme.ON_ACCENT, fg_color=theme.ACCENT,
                                  hover_color=theme.ACCENT_HOVER, text_color_disabled=theme.ON_ACCENT_DISABLED,
                                  command=self.run)
        self._run.grid(row=0, column=0)
        self._stop = ctk.CTkButton(footer, text=tr("sequence.stop"), width=100, image=icon("square", theme.TEXT, 14),
                                   compound="left", fg_color=theme.SURFACE_ALT, hover_color=theme.SURFACE,
                                   text_color=theme.TEXT, text_color_disabled=theme.MUTED, state="disabled",
                                   command=self.stop)
        self._stop.grid(row=0, column=1, padx=(8, 0))
        self._stop_on_error = ctk.CTkSwitch(footer, text=tr("sequence.stop_on_error"), progress_color=theme.ACCENT)
        self._stop_on_error.select()
        self._stop_on_error.grid(row=0, column=2, padx=(12, 0))
        self._state = ctk.CTkLabel(footer, text="", font=theme.small(12), text_color=theme.MUTED, anchor="w")
        self._state.grid(row=0, column=3, sticky="ew", padx=(12, 0))
        for column, (name, key, command) in enumerate((
            ("folder-open", "sequence.open", self._open),
            ("save", "sequence.save", self._save),
            ("trash", "sequence.clear", self._clear),
        ), start=4):
            button = ctk.CTkButton(footer, text="", image=icon(name, theme.TEXT, 16), width=34, height=32,
                                   fg_color=theme.SURFACE_ALT, hover_color=theme.SURFACE, command=command)
            button.grid(row=0, column=column, padx=(6, 0))
            tooltip(button, tr(key))

        self.bind("<Escape>", lambda _event: self._cancel_drag())
        self.refresh()
        self.chrome.center_on(master, SIZE)
        self.after(50, self.lift)

    @property
    def running(self) -> bool:
        """Whether a run is in progress."""
        return self._runner is not None and self._runner.running

    def refresh(self) -> None:
        """Rebuild the blocks from the host's steps."""
        self._cancel_drag()
        for child in self.list.winfo_children():
            child.destroy()
        self._rows = []
        steps = self._host.sequence
        count = len(steps)
        self._title.configure(text=f"  {tr('sequence.title')}  ·  " + (tr("sequence.count_one") if count == 1
                                                                        else tr("sequence.count", count=count)))
        if not steps:
            ctk.CTkLabel(self.list, text=tr("sequence.empty"), font=theme.small(13), text_color=theme.MUTED,
                         justify="left", wraplength=SIZE[0] - 80).grid(row=0, column=0, sticky="w", padx=8, pady=12)
        for index, step in enumerate(steps):
            row = StepRow(self, index, step)
            row.set_position(index)
            self._rows.append(row)
            self._paint(index)
        self._run.configure(state="normal" if steps and not self.running else "disabled")

    def _failed(self, index: int) -> bool:
        result = self._results.get(index)
        return index in self._done and (result is None or not result.ok)

    def _paint(self, index: int) -> None:
        if 0 <= index < len(self._rows):
            text, color = self._statuses.get(index, ("", theme.MUTED))
            can_retry = not self.running and (self._failed(index) or text == tr("sequence.not_run"))
            self._rows[index].set_status(text, color, can_retry)

    def _set_status(self, index: int, text: str, color: Any) -> None:
        self._statuses[index] = (text, color)
        self._paint(index)

    def _structure_changed(self, moved: dict[int, int] | None = None) -> None:
        """Save the new steps; ``moved`` maps old positions to new ones so their state follows them."""
        moved = moved or {}
        self._results = {moved[index]: result for index, result in self._results.items() if index in moved}
        self._statuses = {moved[index]: status for index, status in self._statuses.items() if index in moved}
        self._done = {moved[index] for index in self._done if index in moved}
        self._host.sequence_changed()

    def set_delay(self, index: int, text: str) -> None:
        """Store the delay typed in a block."""
        if index >= len(self._host.sequence):
            return
        step = self._host.sequence[index]
        try:
            value = float(text.replace(",", ".").strip() or 0)
        except ValueError:
            value = step.delay_s
        value = min(max(value, 0.0), MAX_DELAY_S)
        if value != step.delay_s:
            step.delay_s = value
            self._host.sequence_changed(rebuild=False)
        if index < len(self._rows):
            row = self._rows[index]
            row.delay.delete(0, "end")
            row.delay.insert(0, _format_delay(value))

    def edit(self, index: int) -> None:
        """Load a block into the method form."""
        if not self.running:
            self._host.edit_step(index)

    def remove(self, index: int) -> None:
        """Delete a block."""
        if self.running or index >= len(self._host.sequence):
            return
        count = len(self._host.sequence)
        del self._host.sequence[index]
        self._structure_changed({old: old if old < index else old - 1 for old in range(count) if old != index})

    def drag_start(self, row: StepRow, event: Any) -> None:
        """Lift a block: a copy follows the pointer and the block stays as a gap in the list."""
        if self.running:
            return
        self.focus_set()
        self._dragged = row
        self._order = list(self._rows)
        self._ghost_offset = (event.x_root - row.winfo_rootx(), event.y_root - row.winfo_rooty())
        ghost = tk.Toplevel(self)
        ghost.wm_overrideredirect(True)
        ghost.attributes("-topmost", True)
        ghost.attributes("-alpha", GHOST_ALPHA)
        frame = ctk.CTkFrame(ghost, fg_color=theme.SURFACE_ALT, corner_radius=0, border_width=2,
                             border_color=theme.ACCENT, width=row.winfo_width(), height=row.winfo_height())
        frame.pack(fill="both", expand=True)
        frame.pack_propagate(False)
        method = row.step.catalog_method()
        domain = find_domain(method.domain) if method else None
        ctk.CTkLabel(frame, text=f"  {row.step.method}", font=theme.title(13), compound="left", anchor="w",
                     image=icon(domain.icon, theme.ACCENT, 16) if domain else None).pack(side="left", padx=40)
        ghost.wm_geometry(f"+{row.winfo_rootx()}+{row.winfo_rooty()}")
        self._ghost = ghost
        row.set_placeholder(True)

    def drag_motion(self, event: Any) -> None:
        """Move the copy with the pointer and open the gap where the block would land."""
        if self._dragged is None or self._ghost is None:
            return
        self._ghost.wm_geometry(f"+{event.x_root - self._ghost_offset[0]}+{event.y_root - self._ghost_offset[1]}")
        self._autoscroll(event.y_root)

        others = [row for row in self._order if row is not self._dragged]
        target = len(others)
        for position, row in enumerate(others):
            if event.y_root < row.winfo_rooty() + row.winfo_height() / 2:
                target = position
                break
        if others[:target] + [self._dragged] + others[target:] != self._order:
            self._order = others[:target] + [self._dragged] + others[target:]
            for position, row in enumerate(self._order):
                row.set_position(position)

    def _autoscroll(self, y_root: int) -> None:
        canvas = getattr(self.list, "_parent_canvas", None)
        if canvas is None:
            return
        top = canvas.winfo_rooty()
        bottom = top + canvas.winfo_height()
        if y_root < top + AUTOSCROLL_MARGIN:
            canvas.yview_scroll(-1, "units")
        elif y_root > bottom - AUTOSCROLL_MARGIN:
            canvas.yview_scroll(1, "units")

    def drag_end(self, _event: Any) -> None:
        """Drop the block in the gap and save the new order."""
        dragged, order = self._dragged, self._order
        self._close_ghost()
        if dragged is None:
            return
        dragged.set_placeholder(False)
        new_order = [row.index for row in order]
        if new_order == list(range(len(order))):
            return
        steps = self._host.sequence
        steps[:] = [steps[index] for index in new_order]
        self._structure_changed({old: new for new, old in enumerate(new_order)})

    def _close_ghost(self) -> None:
        if self._ghost is not None:
            self._ghost.destroy()
            self._ghost = None
        self._dragged = None

    def _cancel_drag(self) -> None:
        if self._dragged is None:
            return
        dragged = self._dragged
        self._close_ghost()
        dragged.set_placeholder(False)
        for position, row in enumerate(self._rows):
            row.set_position(position)

    def show_row_result(self, index: int) -> None:
        """Show the response of a finished block in the main window."""
        result = self._results.get(index)
        if result is not None and index < len(self._host.sequence):
            self._host.show_sequence_result(self._host.sequence[index].method, result)

    def run(self) -> None:
        """Run the steps; when an earlier run left results, ask which steps to run again."""
        if self.running or not self._host.sequence:
            return
        count = len(self._host.sequence)
        completed = [index for index in range(count) if index in self._results and self._results[index].ok]
        failed = [index for index in range(count) if index not in completed]
        indices = list(range(count))
        if self._done and failed:
            options = [("all", tr("sequence.choice.all", count=count))]
            if completed:
                options.append(("completed", tr("sequence.choice.completed", count=len(completed))))
            options.append(("failed", tr("sequence.choice.failed", count=len(failed))))
            choice = choose(self, tr("sequence.title"), tr("sequence.choice.message"), options)
            if choice is None:
                return
            indices = completed if choice == "completed" else failed if choice == "failed" else indices
        self._start(indices, use_delays=True)

    def retry(self, index: int) -> None:
        """Run one step again at once, without its delay."""
        if not self.running and index < len(self._host.sequence):
            self._start([index], use_delays=False)

    def _start(self, indices: list[int], use_delays: bool) -> None:
        self.focus_set()
        self._selected = indices
        for index in indices:
            self._results.pop(index, None)
            self._done.discard(index)
            self._statuses[index] = (tr("sequence.pending"), theme.MUTED)
        self._runner = SequenceRunner(self._host.sequence, self._host.call_method, bool(self._stop_on_error.get()),
                                      indices=indices, use_delays=use_delays)
        for index in range(len(self._rows)):
            self._paint(index)
        self._run_started = time.perf_counter()
        steps = ", ".join(str(index + 1) for index in indices)
        self.console.append_block(f"━━ {time.strftime('%H:%M:%S')}  {tr('sequence.console_run', steps=steps)}")
        self._runner.start()
        self._run.configure(state="disabled")
        self._stop.configure(state="normal")
        self._state.configure(text=tr("sequence.running"), text_color=theme.MUTED)
        self.after(POLL_MS, self._poll)

    def stop(self) -> None:
        """Stop before the next block."""
        if self._runner is not None and self._runner.running:
            self._runner.stop()
            self._state.configure(text=tr("sequence.stopping"), text_color=theme.MUTED)

    def _poll(self) -> None:
        runner = self._runner
        if runner is None or not self.winfo_exists():
            return
        while True:
            try:
                update = runner.updates.get_nowait()
            except queue.Empty:
                break
            self._handle(update)
        if runner.running or not runner.updates.empty():
            self.after(POLL_MS, self._poll)
        else:
            self._finish()

    def _log_step(self, index: int, result: RpcResult | None, text: str | None = None) -> None:
        method = self._host.sequence[index].method if index < len(self._host.sequence) else "?"
        stamp = time.strftime("%H:%M:%S")
        if result is None:
            self.console.append_block(f"✗ {index + 1}. {method}  ·  {tr('sequence.skipped')}  ·  {stamp}",
                                      text=text or "", tag="danger")
            return
        status = tr("app.result_ok") if result.ok else tr("app.result_error")
        mark = "✓" if result.ok else "✗"
        header = f"{mark} {index + 1}. {method}  ·  {status}  ·  {result.elapsed_ms:.0f} ms  ·  {stamp}"
        request = mask_request(result.request)
        response: Any = result.response if result.error is None else result.error
        self.console.append_block(header, {"request": request, "response": response},
                                  tag="success" if result.ok else "danger")

    def _handle(self, update: RunUpdate) -> None:
        if update.kind == "waiting":
            self._set_status(update.index, tr("sequence.waiting", seconds=f"{update.seconds:.1f}"), theme.ACCENT)
        elif update.kind == "running":
            self._set_status(update.index, tr("sequence.in_progress"), theme.ACCENT)
        elif update.kind == "invalid":
            self._done.add(update.index)
            self._set_status(update.index, tr("sequence.skipped"), theme.DANGER)
            self._state.configure(text=update.text, text_color=theme.DANGER)
            self._log_step(update.index, None, update.text)
        elif update.kind == "result" and update.result is not None:
            result = update.result
            self._results[update.index] = result
            self._done.add(update.index)
            text = (f"{tr('app.result_ok')} · {result.elapsed_ms:.0f} ms" if result.ok
                    else tr("app.result_error"))
            self._set_status(update.index, text, theme.SUCCESS if result.ok else theme.DANGER)
            self._log_step(update.index, result)
            if update.index < len(self._host.sequence):
                self._host.show_sequence_result(self._host.sequence[update.index].method, result)
        elif update.kind == "stopped":
            for index in self._selected:
                if index not in self._done:
                    self._set_status(index, tr("sequence.not_run"), theme.MUTED)
            self._state.configure(text=tr("sequence.stopped"), text_color=theme.DANGER)
        elif update.kind == "finished":
            failed = sum(1 for index in self._selected if self._failed(index))
            if failed:
                self._state.configure(text=tr("sequence.finished_errors", count=failed), text_color=theme.DANGER)
            else:
                self._state.configure(text=tr("sequence.finished"), text_color=theme.SUCCESS)

    def _finish(self) -> None:
        self._runner = None
        elapsed = time.perf_counter() - self._run_started
        ok = sum(1 for index in self._selected if index in self._results and self._results[index].ok)
        summary = tr("sequence.console_summary", ok=ok, total=len(self._selected), seconds=f"{elapsed:.1f}")
        self.console.append_block(f"━━ {summary}")
        self._stop.configure(state="disabled")
        self._run.configure(state="normal" if self._host.sequence else "disabled")
        for index in range(len(self._rows)):
            self._paint(index)
        if len(self._selected) > 1:
            self.tabs.set(tr("sequence.tab.console"))

    def _copy_console(self) -> None:
        self.clipboard_clear()
        self.clipboard_append(self.console.text())

    def _open(self) -> None:
        if self.running:
            return
        path = filedialog.askopenfilename(parent=self, filetypes=FILE_TYPES)
        if not path:
            return
        try:
            steps = load(Path(path))
        except ValueError as exc:
            messagebox.showerror(tr("sequence.title"), tr("sequence.open_failed", error=exc), parent=self)
            return
        self._host.sequence[:] = steps
        self._structure_changed()
        self._state.configure(text=tr("sequence.opened", count=len(steps)), text_color=theme.MUTED)

    def _save(self) -> None:
        if not self._host.sequence:
            return
        path = filedialog.asksaveasfilename(parent=self, defaultextension=".json", filetypes=FILE_TYPES,
                                            initialfile="sequence.json")
        if not path:
            return
        try:
            save(self._host.sequence, Path(path))
        except OSError as exc:
            messagebox.showerror(tr("sequence.title"), tr("sequence.save_failed", error=exc), parent=self)
            return
        self._state.configure(text=tr("sequence.saved"), text_color=theme.MUTED)

    def _clear(self) -> None:
        if self.running or not self._host.sequence:
            return
        if not messagebox.askyesno(tr("sequence.title"), tr("sequence.confirm_clear"), parent=self):
            return
        self._host.sequence.clear()
        self._structure_changed()

    def _close(self) -> None:
        if self._runner is not None:
            self._runner.stop()
        self._close_ghost()
        self.destroy()
