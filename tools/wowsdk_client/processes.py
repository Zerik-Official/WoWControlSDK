"""Find running games and injectors, tell which ones have the SDK, and close them."""

# Standard library
from __future__ import annotations

import ctypes
import ctypes.wintypes as wintypes
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path

# Third-party
import pywintypes
import win32api
import win32con
import win32file
import win32gui
import win32pipe
import win32process

# Project imports
from .transport import COMMAND_PIPE

SDK_DLL_NAME = "wowcontrolsdk.dll"
INJECTOR_NAME = "wowinjector.exe"
DEFAULT_GAME_NAME = "wow.exe"

TH32CS_SNAPPROCESS = 0x00000002
TH32CS_SNAPMODULE = 0x00000008
TH32CS_SNAPMODULE32 = 0x00000010
PROCESS_QUERY_LIMITED_INFORMATION = 0x1000
INVALID_HANDLE = ctypes.c_void_p(-1).value
MAX_PATH = 260
MAX_MODULE_NAME = 256


class ProcessError(Exception):
    """A process could not be closed."""


class PROCESSENTRY32W(ctypes.Structure):
    """Process entry returned by the Toolhelp snapshot."""

    _fields_ = [
        ("dwSize", wintypes.DWORD),
        ("cntUsage", wintypes.DWORD),
        ("th32ProcessID", wintypes.DWORD),
        ("th32DefaultHeapID", ctypes.c_size_t),
        ("th32ModuleID", wintypes.DWORD),
        ("cntThreads", wintypes.DWORD),
        ("th32ParentProcessID", wintypes.DWORD),
        ("pcPriClassBase", ctypes.c_long),
        ("dwFlags", wintypes.DWORD),
        ("szExeFile", ctypes.c_wchar * MAX_PATH),
    ]


class MODULEENTRY32W(ctypes.Structure):
    """Module entry returned by the Toolhelp snapshot."""

    _fields_ = [
        ("dwSize", wintypes.DWORD),
        ("th32ModuleID", wintypes.DWORD),
        ("th32ProcessID", wintypes.DWORD),
        ("GlblcntUsage", wintypes.DWORD),
        ("ProccntUsage", wintypes.DWORD),
        ("modBaseAddr", ctypes.c_void_p),
        ("modBaseSize", wintypes.DWORD),
        ("hModule", wintypes.HMODULE),
        ("szModule", ctypes.c_wchar * (MAX_MODULE_NAME)),
        ("szExePath", ctypes.c_wchar * MAX_PATH),
    ]


_kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
_kernel32.CreateToolhelp32Snapshot.restype = wintypes.HANDLE
_kernel32.CreateToolhelp32Snapshot.argtypes = (wintypes.DWORD, wintypes.DWORD)
_kernel32.Process32FirstW.argtypes = (wintypes.HANDLE, ctypes.POINTER(PROCESSENTRY32W))
_kernel32.Process32NextW.argtypes = (wintypes.HANDLE, ctypes.POINTER(PROCESSENTRY32W))
_kernel32.Module32FirstW.argtypes = (wintypes.HANDLE, ctypes.POINTER(MODULEENTRY32W))
_kernel32.Module32NextW.argtypes = (wintypes.HANDLE, ctypes.POINTER(MODULEENTRY32W))
_kernel32.CloseHandle.argtypes = (wintypes.HANDLE,)
_kernel32.QueryFullProcessImageNameW.argtypes = (wintypes.HANDLE, wintypes.DWORD, wintypes.LPWSTR,
                                                 ctypes.POINTER(wintypes.DWORD))


@dataclass(frozen=True)
class TrackedProcess:
    """A game or injector process and what the client knows about it."""

    pid: int
    name: str
    kind: str
    path: str
    started: datetime | None
    sdk_loaded: bool
    owns_pipe: bool
    window: str


def _snapshot_processes() -> list[tuple[int, str]]:
    snapshot = _kernel32.CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)
    if not snapshot or snapshot == INVALID_HANDLE:
        return []
    entries: list[tuple[int, str]] = []
    entry = PROCESSENTRY32W()
    entry.dwSize = ctypes.sizeof(PROCESSENTRY32W)
    try:
        found = _kernel32.Process32FirstW(snapshot, ctypes.byref(entry))
        while found:
            entries.append((int(entry.th32ProcessID), entry.szExeFile))
            found = _kernel32.Process32NextW(snapshot, ctypes.byref(entry))
    finally:
        _kernel32.CloseHandle(snapshot)
    return entries


def _has_module(pid: int, module_name: str) -> bool:
    snapshot = _kernel32.CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid)
    if not snapshot or snapshot == INVALID_HANDLE:
        return False
    entry = MODULEENTRY32W()
    entry.dwSize = ctypes.sizeof(MODULEENTRY32W)
    try:
        found = _kernel32.Module32FirstW(snapshot, ctypes.byref(entry))
        while found:
            if entry.szModule.lower() == module_name:
                return True
            found = _kernel32.Module32NextW(snapshot, ctypes.byref(entry))
    finally:
        _kernel32.CloseHandle(snapshot)
    return False


def _process_details(pid: int) -> tuple[str, datetime | None]:
    try:
        handle = win32api.OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, False, pid)
    except pywintypes.error:
        return "", None
    try:
        size = wintypes.DWORD(MAX_PATH * 4)
        buffer = ctypes.create_unicode_buffer(size.value)
        path = buffer.value if _kernel32.QueryFullProcessImageNameW(int(handle), 0, buffer, ctypes.byref(size)) else ""
        try:
            created = win32process.GetProcessTimes(handle)["CreationTime"]
            started = datetime.fromtimestamp(created.timestamp())
        except (pywintypes.error, KeyError, ValueError, OSError):
            started = None
        return path, started
    finally:
        win32api.CloseHandle(handle)


def _window_states() -> dict[int, bool]:
    states: dict[int, bool] = {}

    def collect(hwnd: int, _: object) -> bool:
        if win32gui.GetParent(hwnd):
            return True
        _, pid = win32process.GetWindowThreadProcessId(hwnd)
        states[pid] = states.get(pid, False) or bool(win32gui.IsWindowVisible(hwnd))
        return True

    try:
        win32gui.EnumWindows(collect, None)
    except pywintypes.error:
        pass
    return states


def command_pipe_owner() -> int | None:
    """Process id of the game that answers the command pipe, or ``None`` when no game does."""
    try:
        handle = win32file.CreateFile(COMMAND_PIPE, win32file.GENERIC_READ | win32file.GENERIC_WRITE, 0, None,
                                      win32file.OPEN_EXISTING, 0, None)
    except pywintypes.error:
        return None
    try:
        return int(win32pipe.GetNamedPipeServerProcessId(handle))
    except pywintypes.error:
        return None
    finally:
        win32file.CloseHandle(handle)


def list_processes(game_path: str = "") -> list[TrackedProcess]:
    """Running games and injectors, games first."""
    game_names = {DEFAULT_GAME_NAME}
    if game_path:
        game_names.add(Path(game_path).name.lower())

    candidates = [(pid, name) for pid, name in _snapshot_processes()
                  if name.lower() in game_names or name.lower() == INJECTOR_NAME]
    if not candidates:
        return []

    owner = command_pipe_owner()
    windows = _window_states()
    tracked: list[TrackedProcess] = []
    for pid, name in candidates:
        is_game = name.lower() != INJECTOR_NAME
        path, started = _process_details(pid)
        if pid in windows:
            window = "visible" if windows[pid] else "hidden"
        else:
            window = "none"
        tracked.append(TrackedProcess(
            pid=pid,
            name=name,
            kind="game" if is_game else "injector",
            path=path,
            started=started,
            sdk_loaded=is_game and _has_module(pid, SDK_DLL_NAME),
            owns_pipe=owner == pid,
            window=window,
        ))
    tracked.sort(key=lambda item: (item.kind != "game", item.started or datetime.min))
    return tracked


def terminate(pid: int) -> None:
    """Close a process immediately."""
    try:
        handle = win32api.OpenProcess(win32con.PROCESS_TERMINATE, False, pid)
    except pywintypes.error as exc:
        raise ProcessError(exc.strerror) from exc
    try:
        win32api.TerminateProcess(handle, 1)
    except pywintypes.error as exc:
        raise ProcessError(exc.strerror) from exc
    finally:
        win32api.CloseHandle(handle)
