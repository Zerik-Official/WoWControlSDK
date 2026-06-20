#!/usr/bin/env python3
"""WoWControlSDK RPC Test Suite

Tests JSON-RPC 2.0 protocol over named pipe (WowGameCommand).

Usage:
    pytest tests/ -v                         # run all
    pytest tests/ -k "player"               # filter by keyword

Requires:
    - WoW running with WoWControlSDK.dll injected
    - pytest (pip install pytest)
"""

import json
import pytest

try:
    import win32file
    import pywintypes
except ImportError:
    pytestmark = pytest.skip("pywin32 required: pip install pywin32", allow_module_level=True)

PIPE_NAME = r"\\.\pipe\WowGameCommand"


def send(req: dict) -> dict:
    try:
        h = win32file.CreateFile(
            PIPE_NAME,
            win32file.GENERIC_READ | win32file.GENERIC_WRITE,
            0, None, win32file.OPEN_EXISTING, 0, None,
        )
    except pywintypes.error as e:
        pytest.skip(f"Cannot connect to pipe: {e}")
        return {}

    try:
        win32file.WriteFile(h, json.dumps(req).encode("utf-8"))
        _, data = win32file.ReadFile(h, 8192)
        return json.loads(data.decode("utf-8").rstrip("\x00"))
    finally:
        win32file.CloseHandle(h)


def assert_result(resp: dict, id=1):
    assert resp.get("jsonrpc") == "2.0", f"bad jsonrpc: {resp}"
    assert "error" not in resp, f"unexpected error: {resp.get('error')}"
    assert resp.get("id") == id, f"id mismatch: {resp}"
    return resp["result"]


def assert_error(resp: dict, code: int):
    assert resp.get("jsonrpc") == "2.0"
    assert "result" not in resp
    assert resp["error"]["code"] == code
    return resp["error"]


class TestProtocol:
    def test_ping(self):
        resp = send({"jsonrpc": "2.0", "method": "client.ping", "params": {}, "id": 1})
        r = assert_result(resp)
        assert r["status"] == "ok"

    def test_ping_notification(self):
        resp = send({"jsonrpc": "2.0", "method": "client.ping", "params": {}})
        assert isinstance(resp, dict)
        assert resp.get("result", {}).get("status") == "ok"

    def test_parse_error(self):
        h = win32file.CreateFile(
            PIPE_NAME, win32file.GENERIC_READ | win32file.GENERIC_WRITE,
            0, None, win32file.OPEN_EXISTING, 0, None,
        )
        win32file.WriteFile(h, b"not json")
        _, data = win32file.ReadFile(h, 4096)
        win32file.CloseHandle(h)
        resp = json.loads(data.decode("utf-8").rstrip("\x00"))
        assert_error(resp, -32700)

    def test_invalid_request(self):
        resp = send({"method": "client.ping", "params": {}, "id": 1})
        assert_error(resp, -32600)

    def test_method_not_found(self):
        resp = send({"jsonrpc": "2.0", "method": "does.not.exist", "params": {}, "id": 42})
        assert_error(resp, -32601)


class TestPlayer:
    def test_health(self):
        resp = send({"jsonrpc": "2.0", "method": "player.getHealth", "params": {}, "id": 1})
        r = assert_result(resp)
        assert "health" in r
        assert "maxHealth" in r
        assert isinstance(r["health"], int)
        assert isinstance(r["maxHealth"], int)

    def test_position(self):
        resp = send({"jsonrpc": "2.0", "method": "player.getPosition", "params": {}, "id": 1})
        r = assert_result(resp)
        pos = r["position"]
        assert "x" in pos and "y" in pos and "z" in pos
        assert all(isinstance(v, (int, float)) for v in (pos["x"], pos["y"], pos["z"]))
        assert "rotation" in r

    def test_target(self):
        resp = send({"jsonrpc": "2.0", "method": "player.getTarget", "params": {}, "id": 1})
        r = assert_result(resp)
        assert "guid" in r
        assert "valid" in r
        assert isinstance(r["valid"], bool)

    def test_state(self):
        resp = send({"jsonrpc": "2.0", "method": "player.getState", "params": {}, "id": 1})
        r = assert_result(resp)
        assert "health" in r
        assert "maxHealth" in r
        assert "level" in r
        assert "class" in r
        assert "race" in r
        assert "position" in r
        assert "inCombat" in r
        assert isinstance(r["inCombat"], bool)
        assert "valid" in r
        assert r["valid"] is True


class TestWorld:
    def test_state(self):
        resp = send({"jsonrpc": "2.0", "method": "world.getState", "params": {}, "id": 1})
        r = assert_result(resp)
        assert "mapId" in r
        assert "zoneId" in r
        assert "inWorld" in r
        assert "loading" in r
        assert "zoneName" in r
        assert isinstance(r["mapId"], int)
        assert isinstance(r["zoneName"], str)


if __name__ == "__main__":
    import sys
    import subprocess
    sys.exit(subprocess.call([sys.executable, "-m", "pytest", __file__, "-v", *sys.argv[1:]]))
