"""
WoWControlSDK RPC Test Client

Tests the JSON-RPC 2.0 protocol over the WowGameCommand named pipe.

Usage:
    python tests/test_rpc_client.py              # run all tests
    python tests/test_rpc_client.py client.ping   # run single test

Requires:
    - WoW running with WoWControlSDK.dll injected
    - pywin32 (pip install pywin32)
"""

import json
import sys
import struct
import traceback

try:
    import win32pipe
    import win32file
    import pywintypes
except ImportError:
    print("ERROR: pywin32 required. Install with: pip install pywin32")
    sys.exit(1)

PIPE_NAME = r"\\.\pipe\WowGameCommand"
TIMEOUT_MS = 3000


def send_request(request: dict) -> dict | str:
    try:
        handle = win32file.CreateFile(
            PIPE_NAME,
            win32file.GENERIC_READ | win32file.GENERIC_WRITE,
            0,
            None,
            win32file.OPEN_EXISTING,
            0,
            None,
        )
    except pywintypes.error as e:
        if e.winerror == 2:
            return f"Pipe not found: {PIPE_NAME}\n  Is WoW running with WoWControlSDK.dll injected?"
        return f"Cannot connect: {e}"

    try:
        raw = json.dumps(request).encode("utf-8")
        win32file.WriteFile(handle, raw)

        result, data = win32file.ReadFile(handle, 4096)
        response_raw = data.decode("utf-8").rstrip("\x00")
        return json.loads(response_raw)
    except pywintypes.error as e:
        return f"Pipe error: {e}"
    except json.JSONDecodeError as e:
        return f"Bad JSON response: {e} - raw: {response_raw!r}"
    finally:
        win32file.CloseHandle(handle)


def assert_eq(label: str, got, expected):
    status = "PASS" if got == expected else "FAIL"
    print(f"  [{status}] {label}")
    if status == "FAIL":
        print(f"    expected: {expected!r}")
        print(f"    got:      {got!r}")


def assert_key(label: str, obj: dict, key: str, expected_type=None):
    if key not in obj:
        print(f"  [FAIL] {label}: missing key '{key}'")
        print(f"    obj: {obj}")
        return
    val = obj[key]
    if expected_type and not isinstance(val, expected_type):
        print(f"  [FAIL] {label}: '{key}' type mismatch")
        print(f"    expected: {expected_type.__name__}, got: {type(val).__name__}")
        return
    print(f"  [PASS] {label}: '{key}' = {val!r}")


def test_ping():
    print("\n=== client.ping ===")
    resp = send_request({
        "jsonrpc": "2.0",
        "method": "client.ping",
        "params": {},
        "id": 1,
    })
    if isinstance(resp, str):
        print(f"  [FAIL] {resp}")
        return

    assert_key("jsonrpc version", resp, "jsonrpc")
    assert_eq("jsonrpc == 2.0", resp.get("jsonrpc"), "2.0")
    assert_eq("result.status == ok", resp.get("result", {}).get("status"), "ok")
    assert_eq("id matches", resp.get("id"), 1)
    assert_key("has result", resp, "result", dict)


def test_ping_no_id():
    print("\n=== client.ping (no id) ===")
    resp = send_request({
        "jsonrpc": "2.0",
        "method": "client.ping",
        "params": {},
    })
    if isinstance(resp, str):
        print(f"  [FAIL] {resp}")
        return

    assert_key("has result", resp, "result", dict)
    assert_eq("result.status == ok", resp.get("result", {}).get("status"), "ok")
    assert_eq("no id sent", resp.get("id"), None)


def test_parse_error():
    print("\n=== Parse error ===")
    try:
        handle = win32file.CreateFile(
            PIPE_NAME,
            win32file.GENERIC_READ | win32file.GENERIC_WRITE,
            0,
            None,
            win32file.OPEN_EXISTING,
            0,
            None,
        )
        win32file.WriteFile(handle, b"not json at all")
        _, data = win32file.ReadFile(handle, 4096)
        resp = json.loads(data.decode("utf-8").rstrip("\x00"))
        win32file.CloseHandle(handle)
    except Exception as e:
        print(f"  [FAIL] Connection error: {e}")
        return

    assert_key("has error", resp, "error", dict)
    assert_eq("error.code == -32700", resp.get("error", {}).get("code"), -32700)
    assert_eq("error.message present", bool(resp.get("error", {}).get("message")), True)


def test_invalid_request():
    print("\n=== Invalid Request ===")
    resp = send_request({
        "method": "client.ping",
        "params": {},
        "id": 1,
    })
    if isinstance(resp, str):
        print(f"  [FAIL] {resp}")
        return

    assert_key("has error", resp, "error", dict)
    error_code = resp.get("error", {}).get("code")
    assert_eq("error.code == -32600", error_code, -32600)


def test_method_not_found():
    print("\n=== Method not found ===")
    resp = send_request({
        "jsonrpc": "2.0",
        "method": "nonexistent.method",
        "params": {},
        "id": 42,
    })
    if isinstance(resp, str):
        print(f"  [FAIL] {resp}")
        return

    assert_key("has error", resp, "error", dict)
    assert_eq("error.code == -32601", resp.get("error", {}).get("code"), -32601)
    assert_eq("id preserved", resp.get("id"), 42)


TESTS = {
    "client.ping": test_ping,
    "ping.no-id": test_ping_no_id,
    "parse-error": test_parse_error,
    "invalid-request": test_invalid_request,
    "method-not-found": test_method_not_found,
}


def main():
    print(f"WoWControlSDK RPC Test Client")
    print(f"Pipe: {PIPE_NAME}")
    print(f"Make sure WoW is running with WoWControlSDK.dll injected.")
    print()

    selected = sys.argv[1:] if len(sys.argv) > 1 else list(TESTS.keys())

    passed = 0
    failed = 0
    for name in selected:
        if name not in TESTS:
            print(f"Unknown test: {name}")
            print(f"Available: {', '.join(TESTS.keys())}")
            continue
        try:
            TESTS[name]()
            passed += 1
        except Exception as e:
            print(f"  [EXCEPTION] {name}: {e}")
            traceback.print_exc()
            failed += 1

    print(f"\n{'='*40}")
    print(f"Results: {passed} passed, {failed} failed")
    return 1 if failed > 0 else 0


if __name__ == "__main__":
    sys.exit(main())
