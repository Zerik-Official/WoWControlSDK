import json, os, subprocess, sys, time
try:
    import win32file, pywintypes
except ImportError:
    print("Missing pywin32. Install: pip install pywin32")
    sys.exit(1)

RPC_PIPE   = r"\\.\pipe\WowGameCommand"
EVENT_PIPE = r"\\.\pipe\WowGameEvent"

WOW_PATH  = r"D:\Juegos\World Of Warcraft\Wow.exe"
DLL_PATH  = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "bin", "release", "WoWControlSDK.dll"))
INJECTOR  = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "bin", "release", "WowInjector.exe"))

_rpc_id = 0

def next_id():
    global _rpc_id
    _rpc_id += 1
    return _rpc_id

def send(method, params={}):
    req = {"jsonrpc": "2.0", "method": method, "params": params, "id": next_id()}
    try:
        h = win32file.CreateFile(RPC_PIPE, win32file.GENERIC_READ | win32file.GENERIC_WRITE, 0, None, win32file.OPEN_EXISTING, 0, None)
        win32file.WriteFile(h, json.dumps(req).encode("utf-8"))
        _, data = win32file.ReadFile(h, 65536)
        win32file.CloseHandle(h)
        return json.loads(data.decode("utf-8").rstrip("\x00"))
    except pywintypes.error as e:
        return {"error": str(e)}

def result_of(resp):
    return resp.get("result", resp)

def print_json(resp):
    print(json.dumps(resp, indent=2, ensure_ascii=False))

def pipe_connected():
    try:
        h = win32file.CreateFile(RPC_PIPE, win32file.GENERIC_READ | win32file.GENERIC_WRITE, 0, None, win32file.OPEN_EXISTING, 0, None)
        win32file.CloseHandle(h)
        return True
    except pywintypes.error:
        return False

# ─── Actions ─────────────────────────────────────────────────

def action_launch_wow():
    for p in [INJECTOR, DLL_PATH, WOW_PATH]:
        if not os.path.exists(p):
            print(f"[!] Not found: {p}")
            return
    cmd = [
        INJECTOR, "--wow", WOW_PATH, "--dll", DLL_PATH, "--kill",
        "--wow-args", "-console",
        "--pipe-cmd", json.dumps({"jsonrpc":"2.0","method":"dll.toggleConsole","params":{},"id":1}),
        "--pipe-cmd", json.dumps({"jsonrpc":"2.0","method":"dll.setLogConfig","params":{"showInConsole":True},"id":2}),
    ]
    print("[>] Launching injector...")
    subprocess.Popen(cmd, creationflags=subprocess.CREATE_NEW_CONSOLE)

def action_ping():
    print_json(send("client.ping"))

def action_screen():
    print_json(send("client.getScreen"))

def action_debug():
    print_json(send("client.getDebugState"))

def action_toggle_console():
    print_json(send("dll.toggleConsole"))

def action_log_config():
    v = input("Show logs in console? (y/n) [y]: ").strip().lower()
    show = v != "n"
    raw = input("Max log entries [256]: ").strip()
    entries = int(raw) if raw.isdigit() else 256
    print_json(send("dll.setLogConfig", {"showInConsole": show, "maxEntries": entries}))

def action_dll_info():
    print_json(send("dll.getInfo"))

def action_get_logs():
    raw = input("How many logs? [50]: ").strip()
    count = int(raw) if raw.isdigit() else 50
    print_json(send("dll.getLogs", {"offset": 0, "count": count}))

def action_login():
    user = input("Username: ").strip()
    pwd  = input("Password: ").strip()
    if not user or not pwd:
        print("[!] Username and password required"); return
    realm_list = input("Realmlist (optional): ").strip()
    realm_name = input("Realm name (optional): ").strip()
    params = {"username": user, "password": pwd}
    if realm_list: params["realmList"] = realm_list
    if realm_name: params["realmName"] = realm_name
    print("[>] Waiting for login result...")
    print_json(send("client.login", params))

def action_get_chars():
    print_json(send("client.getCharacters"))

def action_refresh_chars():
    print_json(send("client.refreshCharacters"))

def action_enter_world():
    resp = send("client.getCharacters")
    chars = result_of(resp).get("characters", [])
    if not chars:
        print("[!] No characters (are you on charselect?)"); return
    for c in chars:
        print(f"  {c['index']}) {c['name']} \u2014 lvl {c['level']} race:{c['race']} class:{c['class']}")
    sel = input("Index or name: ").strip()
    if not sel: return
    try:    params = {"index": int(sel)}
    except ValueError: params = {"name": sel}
    print("[>] Entering world...")
    print_json(send("client.enterWorld", params))

def action_accept_eula():
    print_json(send("client.acceptEULA"))

def action_accept_tos():
    print_json(send("client.acceptTOS"))

def action_accept_termination():
    print_json(send("client.acceptTermination"))

def action_accept_scanning():
    print_json(send("client.acceptScanning"))

def action_accept_contest():
    print_json(send("client.acceptContest"))

def action_accept_all():
    print_json(send("client.acceptAll"))

def action_logout():
    print_json(send("client.logout"))

def action_quit():
    if input("Quit WoW? (y/n): ").strip().lower() == "y":
        print_json(send("client.quit"))

def action_get_realmlist():
    print_json(send("client.getRealmlist"))

def action_set_realmlist():
    val = input("Realmlist (e.g. 127.0.0.1): ").strip()
    if val:
        print_json(send("client.setRealmlist", {"realmList": val}))

def action_player_state():
    print_json(send("player.getState"))

def action_player_health():
    print_json(send("player.getHealth"))

def action_player_position():
    print_json(send("player.getPosition"))

def action_player_target():
    print_json(send("player.getTarget"))

def action_unit_get():
    token = input("Token (player/target/focus/mouseover/partyN/raidN): ").strip() or "player"
    print_json(send("unit.get", {"token": token}))

def action_unit_state():
    token = input("Token: ").strip() or "player"
    print_json(send("unit.state", {"token": token}))

def action_unit_position():
    token = input("Token: ").strip() or "player"
    print_json(send("unit.position", {"token": token}))

def action_world_state():
    print_json(send("world.getState"))

def action_group():
    print_json(send("group.get"))

def action_group_combat():
    print_json(send("group.combatSummary"))

def action_object_inspect():
    token = input("Token: ").strip() or "player"
    print("  a) object.exists  b) object.getType  c) object.isUnit  d) object.isPlayer")
    opt = input("Choice: ").strip()
    method = {"a":"object.exists","b":"object.getType","c":"object.isUnit","d":"object.isPlayer"}.get(opt)
    if method:
        print_json(send(method, {"token": token}))

def action_events_list():
    resp = send("events.list")
    subs = result_of(resp).get("events", [])
    if subs:
        print(f"Subscribed ({len(subs)}):")
        for e in subs: print(f"  {e}")
    else:
        print("No active subscriptions.")

def action_events_subscribe():
    print("Examples: PLAYER_LOGIN, PLAYER_ENTERING_WORLD, UNIT_HEALTH, PLAYER_REGEN_DISABLED, COMBAT_LOG_EVENT_UNFILTERED")
    raw = input("Events (comma-separated): ").strip()
    if not raw: return
    events = [e.strip() for e in raw.split(",") if e.strip()]
    print_json(send("events.subscribe", {"events": events}))

def action_events_defaults():
    events = ["PLAYER_LOGIN","PLAYER_ENTERING_WORLD","PLAYER_REGEN_DISABLED","UNIT_HEALTH"]
    resp = send("events.subscribe", {"events": events})
    print(f"[>] Subscribed to: {', '.join(events)}")
    print_json(resp)

def action_events_unsubscribe():
    raw = input("Events to unsubscribe (empty = all): ").strip()
    if not raw:
        print_json(send("events.clear"))
    else:
        events = [e.strip() for e in raw.split(",") if e.strip()]
        print_json(send("events.unsubscribe", {"events": events}))

def action_events_history():
    raw = input("How many? [50]: ").strip()
    count = int(raw) if raw.isdigit() else 50
    print_json(send("events.getHistory", {"offset": 0, "count": count}))

def action_events_clear_history():
    print_json(send("events.clearHistory"))

def action_events_set_buffer():
    raw = input("Max buffer entries [256]: ").strip()
    entries = int(raw) if raw.isdigit() else 256
    print_json(send("events.setMaxBuffer", {"maxBuffer": entries}))

def action_events_ready():
    print_json(send("events.isReady"))

def action_listen_events():
    subs = result_of(send("events.list")).get("events", [])
    if not subs:
        print("[!] No subscriptions. Subscribe first."); return
    print(f"[>] Subscriptions: {', '.join(subs)}")
    print("[>] Connecting to event pipe. Ctrl+C to stop.")
    try:
        h = win32file.CreateFile(EVENT_PIPE, win32file.GENERIC_READ, 0, None, win32file.OPEN_EXISTING, 0, None)
        print("[>] Connected.\n")
        buf = ""
        while True:
            try:
                _, data = win32file.ReadFile(h, 65536)
                buf += data.decode("utf-8", errors="replace")
                while "\n" in buf:
                    line, buf = buf.split("\n", 1)
                    line = line.strip()
                    if not line: continue
                    try: print(json.dumps(json.loads(line), ensure_ascii=False))
                    except json.JSONDecodeError: print(line)
            except pywintypes.error as e:
                code = e.args[0]
                if code == 233:
                    time.sleep(0.05); continue
                if code == 109:
                    print("[>] Pipe disconnected.")
                break
        win32file.CloseHandle(h)
    except pywintypes.error as e:
        print(f"[!] Could not connect to event pipe: {e}")
    except KeyboardInterrupt:
        print("\n[>] Listener stopped.")

# ─── Lua ────────────────────────────────────────────────────

def action_lua_execute():
    code = input("Lua code: ").strip()
    if code: print_json(send("lua.execute", {"code": code}))

def action_lua_evaluate():
    code = input("Lua code: ").strip()
    if code: print_json(send("lua.evaluate", {"code": code}))

def action_lua_get_global():
    name = input("Global name: ").strip()
    if name: print_json(send("lua.getGlobal", {"name": name}))

def action_lua_set_global():
    name = input("Global name: ").strip()
    if not name: return
    raw = input("Value (string/number/bool): ").strip()
    if not raw: return
    try:
        v = json.loads(raw)
        if isinstance(v, (int, float)):
            val = v
        elif isinstance(v, bool):
            val = v
        elif isinstance(v, str):
            val = v
        else:
            print("[!] Unsupported type"); return
    except json.JSONDecodeError:
        val = raw
    print_json(send("lua.setGlobal", {"name": name, "value": val}))

def action_lua_create_namespace():
    name = input("Namespace name (e.g. SDK): ").strip()
    if name: print_json(send("lua.createNamespace", {"name": name}))

# ─── Menu ─────────────────────────────────────────────────────

MENU = [
    ("Game", None),
    ("Launch WoW",           action_launch_wow),
    ("Ping",                 action_ping),
    ("Screen",               action_screen),
    ("Debug state",          action_debug),

    ("DLL", None),
    ("Toggle console",       action_toggle_console),
    ("Log config",           action_log_config),
    ("DLL info",             action_dll_info),
    ("Captured logs",        action_get_logs),

    ("Auth", None),
    ("Login",                action_login),
    ("Accept EULA",          action_accept_eula),
    ("Accept TOS",           action_accept_tos),
    ("Accept Termination",   action_accept_termination),
    ("Accept Scanning",      action_accept_scanning),
    ("Accept Contest",       action_accept_contest),
    ("Accept ALL",           action_accept_all),
    ("Get characters",       action_get_chars),
    ("Refresh characters",   action_refresh_chars),
    ("Enter world",          action_enter_world),
    ("Logout to charselect", action_logout),
    ("Quit game",            action_quit),
    ("Get realmlist",        action_get_realmlist),
    ("Set realmlist",        action_set_realmlist),

    ("Player", None),
    ("Player state",         action_player_state),
    ("Health",               action_player_health),
    ("Position",             action_player_position),
    ("Target",               action_player_target),

    ("Unit", None),
    ("unit.get (full)",      action_unit_get),
    ("unit.state (flags)",   action_unit_state),
    ("unit.position",        action_unit_position),

    ("Object", None),
    ("object.* inspect",     action_object_inspect),

    ("World / Group", None),
    ("world.getState",       action_world_state),
    ("group.get",            action_group),
    ("group.combatSummary",  action_group_combat),

    ("Events", None),
    ("List subscriptions",   action_events_list),
    ("Subscribe",            action_events_subscribe),
    ("Subscribe defaults",   action_events_defaults),
    ("Unsubscribe / Clear",  action_events_unsubscribe),
    ("View history",         action_events_history),
    ("Clear history",        action_events_clear_history),
    ("Set max buffer",       action_events_set_buffer),
    ("Check hook ready",     action_events_ready),
    ("Listen live",          action_listen_events),

    ("Lua", None),
    ("lua.execute (fire-and-forget)",    action_lua_execute),
    ("lua.evaluate (with return)",       action_lua_evaluate),
    ("lua.getGlobal",                    action_lua_get_global),
    ("lua.setGlobal",                    action_lua_set_global),
    ("lua.createNamespace",              action_lua_create_namespace),

    ("", None),
    ("Exit",                 None),
]

def main():
    print("=" * 52)
    print("  WoWControlSDK \u2014 Interactive CLI")
    print("=" * 52)
    print("[+] RPC pipe connected\n" if pipe_connected() else "[-] RPC pipe unavailable\n")

    while True:
        print()
        for i, (label, action) in enumerate(MENU):
            if action is None and label:
                print(f"  \u2500\u2500 {label} \u2500\u2500")
            elif action is None:
                continue
            else:
                print(f"  {i:>2}) {label}")

        choice = input("\nOption: ").strip()
        if not choice.isdigit(): continue
        idx = int(choice)
        if idx < 0 or idx >= len(MENU): continue
        label, func = MENU[idx]
        if func is None:
            if "Exit" in label:
                print("Goodbye."); break
            continue
        print()
        func()

if __name__ == "__main__":
    main()
