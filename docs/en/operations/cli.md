# Desktop client

WoWControl Client is a desktop application for calling every SDK method and watching its events. It starts the game with the injector and a session token, builds a form for each method, shows the JSON response, keeps a history of calls and lists the running games. It is useful for manual testing, for inspecting responses and as a reference for a custom integration.

The interface is available in English and Spanish.

![WoWControl Client showing the world.getState response](../assets/client/client-response.png)

## Download

Every release includes `WoWControlClient-<version>.zip` next to the SDK package. Extract it anywhere and run `WoWControlClient.exe`. The folder already contains `binaries/WoWControlSDK.dll` and `binaries/WowInjector.exe`, so only the path of `Wow.exe` has to be set.

The client is built as a folder (`WoWControlClient.exe` plus `_internal/`) instead of a single executable, so it starts quickly and is not unpacked to a temporary folder on every run. Keep the whole folder together.

## Run from source

| Tool | Version | Purpose |
| --- | --- | --- |
| Python | 3.10 or later | Runs the client |
| customtkinter | 6.0.0 | Interface |
| pywin32 | 312 | Named pipes and processes |
| Pillow | 12.2.0 | Icons |

```bash
pip install -r requirements.txt
cd tools
python -m wowsdk_client
```

From source the client uses `bin/release/WoWControlSDK.dll` and `bin/release/WowInjector.exe` of the repository when they exist.

## Build

```bash
pip install -r requirements-build.txt
pyinstaller --noconfirm --clean --windowed --onedir --name WoWControlClient --paths tools --collect-data customtkinter --collect-data wowsdk_client --distpath dist --workpath build/pyinstaller --specpath build/pyinstaller tools/run_client.py
```

The client is written to `dist/WoWControlClient/`. `--windowed` hides the console window, `--onedir` keeps the folder layout and `--collect-data wowsdk_client` includes the method catalog, the translations and the icons. The CI workflow runs the same command on every pull request and uploads the folder as an artifact.

## Window

| Area | Content |
| --- | --- |
| Title bar | The client's own title bar: SDK status (whether the command pipe exists), session token state, version and commit of the configured DLL, **Sequence**, **Settings**, **Launch WoW** and the window buttons. Drag it to move the window and double-click it to maximize; the edges resize the window |
| Method browser | A rail with one icon per domain and the methods of the selected domain as cards with their description. Resting the mouse on an icon shows the name of its domain. The search box looks in every domain |
| Form | The selected method with one input per parameter: text, password, number, switch, list, Lua code or JSON. Required parameters are marked with `*`. A method shows again the values it was last sent with |
| Response | The JSON response with colors, the call status and the time it took, with **Repeat** and a button to copy the response |
| History | The last 100 calls; selecting one shows its request and response, and its repeat button loads it into the form |
| Events | Live view of the event pipe with a name filter, pause and clear |
| Processes | Every `Wow.exe` and `WowInjector.exe` that is running, with the option to close them |
| Activity | Injector output, event pipe status and client messages |

Press **Send** or `Ctrl+Enter` to send the request. Calls run on a worker thread, so slow methods such as `client.login` or `client.enterWorld` do not freeze the window. The copy button next to **Send** copies the JSON request with the token and secret parameters, such as passwords, replaced by placeholders. **History** and the sequence console hide them the same way. `client.quit` asks for confirmation.

| Shortcut | Action |
| --- | --- |
| `Ctrl+Enter` | Send the request |
| `Ctrl+Shift+Enter` | Add the request to the sequence |
| `Ctrl+R` | Repeat the call shown in **Response** |
| `Ctrl+F` | Search methods |

### Repeating a call

**Repeat**, in **Response**, and the repeat button of each **History** row select the method of that call and fill the form with the values it was sent with. Nothing is sent until **Send** is pressed, so the values can be changed first.

### Sequences

A sequence chains several calls that run one after another, for example `client.login` and then `client.enterWorld`. **Add to sequence** adds the request of the form as the last step, and **Sequence** in the title bar opens the sequence window.

![Sequence window after a run, with the console showing each step](../assets/client/client-sequence.png)

| Element | Description |
| --- | --- |
| Handle | Drag a block by its handle to change the order. A copy of the block follows the pointer, the gap moves between the other blocks and the list scrolls near its edges; `Esc` cancels |
| Delay | Seconds to wait before the step starts, for example `3` to send `client.enterWorld` three seconds after the login answered |
| Edit | Loads the step into the form; **Add to sequence** becomes **Update step** until the step is saved or the edit is cancelled |
| Status | Pending, the countdown of the delay, running, OK with its time, error, skipped or not run. Selecting a finished step shows its response |
| Retry | Appears on a step that failed, was skipped or did not run, and runs that step again at once, without its delay |
| **Run all** and **Stop** | Run the steps in order, or stop before the next one; a call already sent still finishes |
| Stop on error | When on, the first failed step ends the run |
| Open and save | Read or write the sequence as a JSON file |
| **Console** tab | Every step of every run with its time, status, request and response as colored JSON, with buttons to copy and clear it. It opens by itself when a run of several steps ends |

When an earlier run left steps that failed or did not run, **Run all** asks whether to run every step, only the completed ones or only the failed ones. When every step completed, it runs them all again without asking.

Each finished step also appears in **Response** and **History**. Closing the window stops a run in progress.

The sequence is saved in `%APPDATA%\WoWControlClient\sequence.json` and restored when the client starts. Secret parameters, such as the password of `client.login`, are kept in memory only: they are never written to disk, so after reopening the client or opening a file the step shows them as missing until it is edited.

### dll.getInfo without the game

When the SDK is not running, `dll.getInfo` is answered by `WowInjector.exe --call dll.getInfo`, which loads the configured DLL without starting the game (see [Reading the DLL without the game](../methods/dll.md#reading-the-dll-without-the-game)). The response shows `without the game` next to its time. The same call fills the DLL version in the title bar when the client starts and when the DLL or injector path changes; resting the mouse on it shows the full commit, its date and the path. It needs the injector of SDK 2.3 or later.

### Finding a method

Select a domain on the rail to list its methods, or type in the search box to look for a word in every method name and description. The search uses the language of the interface.

![Search results for "accept" across several domains](../assets/client/client-search.png)

## Settings

| Setting | Description |
| --- | --- |
| Wow.exe | Path of the game executable |
| SDK DLL and Injector | Paths of `WoWControlSDK.dll` and `WowInjector.exe`. Empty uses the bundled or built ones |
| Use a session token | Launch with `--token-stdin` and a new token for each session (recommended). When off, the client launches with `--no-auth` |
| Close running Wow.exe before launching | Passes `--kill` to the injector |
| Open the in-game SDK console | Sends `dll.toggleConsole` once the SDK is ready |
| Show captured logs in the console | Sends `dll.setLogConfig` with `showInConsole`. Messages captured before the console opened, such as the archives loaded at startup, are shown too |
| Start the game hidden | Passes `--hidden` to the injector; send `window.show` to reveal the window |
| Use manual map (experimental) | Passes `--manual-map` to map the DLL in memory without `LoadLibrary`. It can fail during SDK startup on some game builds |
| Connect to the event pipe after launching | Opens **Events** automatically once the injector finished. On by default |
| Wow.exe arguments | Passed with `--wow-args`; `-console` by default |
| Token | Token used for requests and the event pipe. Paste one to control a game started elsewhere |
| Appearance | Dark, light or system theme |
| Language | English, Spanish or the Windows language. The change applies after restarting the client |

![Settings dialog](../assets/client/client-settings.png)

Settings are stored in `%APPDATA%\WoWControlClient\settings.json`. The session token is kept in memory only and is never written to disk.

## Launching the game

**Launch WoW** generates a 64-character hexadecimal token, starts the injector without a console window and writes the token to its standard input, so the token never appears on a command line. The injector output, including the game process id, appears in **Activity**. Every request then carries the token in its `auth` field.

![Activity tab with the injector output after launching the game](../assets/client/client-activity.png)

When **Close running Wow.exe before launching** is off and a game with the SDK or an injector is already running, the client warns that commands could reach the wrong game and offers to close them first.

### When the launch fails

If the injector ends with an error, the client shows its exit code, the most likely cause with what to do next, and the last error line. The injector also closes the game it started, so a failed launch never leaves a suspended `Wow.exe` behind.

| Injector message | Likely cause |
| --- | --- |
| `the SDK did not open its pipe` | The DLL was injected but failed while starting, for example because it does not match the client build |
| `the game closed while the SDK was starting` | The game crashed during startup; look for a new file in its `Errors` folder |
| `LoadLibraryA returned NULL` | Wrong DLL path, a 64-bit build or a missing Visual C++ x86 runtime |
| `CreateProcess` | Wrong `Wow.exe` path or blocked by security software |
| `auth.init rejected` | Another game with the SDK already had a session; close it in **Processes** |

## Processes

The **Processes** tab lists every running `Wow.exe` (or the executable set in the settings) and `WowInjector.exe`, and refreshes every 3 seconds while it is open.

![Processes tab with a game with the SDK, a game left suspended and an injector](../assets/client/client-processes.png)

| Label | Meaning |
| --- | --- |
| SDK loaded | `WoWControlSDK.dll` is loaded in the game |
| Answers the pipe | This game owns `\\.\pipe\WowGameCommand`, so the client's requests reach it |
| Visible window / Hidden window | The game has a window, shown or hidden |
| No window | The game is starting or was left suspended by an earlier failed launch |

**Close** ends one process after a confirmation, and **Close all games** ends every game and injector in the list. Closing a game loses anything the game had not saved.

## Live events

**Events → Connect** opens `\\.\pipe\WowGameEvent`, sends the authentication handshake and lists each event with its time, name and arguments. The filter matches event names, for example `trade.` or `UNIT_`. With **Connect to the event pipe after launching** on, the client connects by itself once the game started with the SDK.

![Events tab with social, trade, addon and queue events](../assets/client/client-events.png)

The event pipe accepts one client at a time. If another program, such as a launcher, is connected, the client reports that the pipe is busy.

!!! tip
    SDK events such as `client.screenChanged`, `loading.*`, `group.inviteReceived`, `lua.custom` or `trade.requested` are always streamed. WoW UI events only appear after `events.subscribe`.

## Adding methods

The client builds every form from the JSON files in `tools/wowsdk_client/data/catalog/`. Each domain has its own file with its icon, its methods and their parameters, and every text is written in English and Spanish:

```json
{
  "name": "client.getNetStats",
  "description": {
    "en": "Latency and bandwidth, refreshed every 500 ms.",
    "es": "Latencia y ancho de banda, actualizados cada 500 ms."
  }
}
```

A parameter adds `name`, `type` (`string`, `text`, `secret`, `integer`, `boolean`, `string_list`, `choice` or `json`), `description` and, when needed, `required`, `default` and `choices`. `index.json` sets the order of the domains on the rail. No interface code is needed for a new method.

The interface texts live in `tools/wowsdk_client/data/locales/en.json` and `es.json`.

| Module | Responsibility |
| --- | --- |
| `catalog.py` | Loads the method catalog |
| `i18n.py` | Interface language and texts |
| `transport.py` | One JSON-RPC request per command pipe connection, with the `auth` field |
| `events.py` | Event pipe reader on a worker thread |
| `launcher.py` | Injector command line, token generation, output forwarding and failure hints |
| `processes.py` | Running games and injectors, the SDK state of each one and closing them |
| `settings.py` | Settings file and bundled binary lookup |
| `sequence.py` | Sequence steps, the file without secrets and the runner on a worker thread |
| `offline.py` | Methods answered without the game through `WowInjector.exe --call` and the DLL version |
| `ui/` | Window, method browser, method form, JSON view, events and processes panels, sequence window, settings dialog and tooltips |

The icons come from the Lucide set of [react-icons](https://react-icons.github.io/react-icons/). They are stored as white PNG files in `tools/wowsdk_client/data/icons/` and the client tints them for each theme.
