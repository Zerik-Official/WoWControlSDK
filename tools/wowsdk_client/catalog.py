"""Catalog of every SDK method the client can call.

Each method is described once here. The interface builds its forms from these
descriptions, so supporting a new SDK method only needs a new entry.
"""

# Standard library
from __future__ import annotations

from dataclasses import dataclass, field
from enum import Enum


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
    description: str
    required: bool = False
    default: str | int | bool | None = None
    choices: tuple[str, ...] = ()


@dataclass(frozen=True)
class Method:
    """One SDK method."""

    name: str
    description: str
    params: tuple[Param, ...] = ()
    confirm: str | None = None

    @property
    def domain(self) -> str:
        """Domain part of the method name, for example ``group``."""
        return self.name.split(".", 1)[0]


@dataclass(frozen=True)
class Domain:
    """A group of methods shown together in the sidebar."""

    name: str
    title: str
    methods: tuple[Method, ...] = field(default_factory=tuple)


def _token(description: str = "Unit token: player, target, focus, mouseover, partyN, raidN or a GUID") -> Param:
    """Unit token parameter shared by the unit and object methods."""
    return Param("token", ParamType.STRING, description, default="player")


def _index() -> Param:
    """Queue index parameter shared by the battlefield actions."""
    return Param("index", ParamType.INTEGER, "Queue slot, 1 or 2", required=True, default=1)


DOMAINS: tuple[Domain, ...] = (
    Domain("client", "Client", (
        Method("client.ping", "Check that the command pipe answers."),
        Method("client.getScreen", "Current glue screen and login state."),
        Method("client.getDebugState", "Low-level login and authentication diagnostics."),
        Method("client.login", "Log in, optionally setting the realm list and choosing a realm.", (
            Param("username", ParamType.STRING, "Account username", required=True),
            Param("password", ParamType.SECRET, "Account password", required=True),
            Param("realmList", ParamType.STRING, "Realm list address to set before logging in"),
            Param("realmName", ParamType.STRING, "Realm to select after logging in"),
        )),
        Method("client.getCharacters", "Characters on the character select screen."),
        Method("client.refreshCharacters", "Ask the server for the character list again."),
        Method("client.enterWorld", "Enter the world with a character.", (
            Param("name", ParamType.STRING, "Character name, takes precedence over index"),
            Param("index", ParamType.INTEGER, "Character slot index", default=0),
            Param("wait", ParamType.BOOLEAN, "Wait until the world is loaded, up to 30 seconds", default=True),
        )),
        Method("client.logout", "Log out to the character select screen."),
        Method("client.quit", "Close the game.", confirm="Close World of Warcraft?"),
        Method("client.getRealmlist", "Read the realm list CVar."),
        Method("client.setRealmlist", "Set the realm list CVar.", (
            Param("realmList", ParamType.STRING, "New realm list address", required=True),
        )),
        Method("client.acceptEULA", "Accept the EULA dialog."),
        Method("client.acceptTOS", "Accept the Terms of Service dialog."),
        Method("client.acceptTermination", "Accept the termination policy dialog."),
        Method("client.acceptScanning", "Accept the scanning dialog."),
        Method("client.acceptContest", "Accept the contest dialog."),
        Method("client.acceptAll", "Accept every agreement dialog."),
    )),
    Domain("player", "Player", (
        Method("player.getState", "Full cached player state."),
        Method("player.getHealth", "Cached health and power."),
        Method("player.getPosition", "Cached world position and rotation."),
        Method("player.getTarget", "Cached target GUID."),
    )),
    Domain("world", "World", (
        Method("world.getState", "Map, zone, map area, instance and difficulty."),
    )),
    Domain("unit", "Units", (
        Method("unit.get", "Full unit snapshot.", (_token(),)),
        Method("unit.state", "Unit state flags.", (_token(),)),
        Method("unit.position", "Unit world position.", (_token(),)),
        Method("unit.mapPosition", "Normalized map position of a party or raid member.", (
            _token("Unit token: partyN or raidN"),
        )),
    )),
    Domain("object", "Objects", (
        Method("object.exists", "Whether the entity exists.", (_token(),)),
        Method("object.getType", "Object type of the entity.", (_token(),)),
        Method("object.isUnit", "Whether the entity is a unit.", (_token(),)),
        Method("object.isPlayer", "Whether the entity is a player.", (_token(),)),
    )),
    Domain("group", "Group", (
        Method("group.get", "Full party or raid roster, including members out of range."),
        Method("group.combatSummary", "Combat summary of the members in range."),
        Method("group.getInvite", "Pending group invitation."),
        Method("group.acceptInvite", "Accept the pending invitation with the selected roles.", (
            Param("tank", ParamType.BOOLEAN, "Offer the tank role", default=False),
            Param("healer", ParamType.BOOLEAN, "Offer the healer role", default=False),
            Param("damage", ParamType.BOOLEAN, "Offer the damage role", default=False),
        )),
        Method("group.declineInvite", "Decline the pending invitation."),
    )),
    Domain("friends", "Friends", (
        Method("friends.list", "Friend list with level, class, zone and status."),
    )),
    Domain("summon", "Summons", (
        Method("summon.get", "Pending summon."),
        Method("summon.accept", "Accept the pending summon."),
        Method("summon.decline", "Decline the pending summon."),
    )),
    Domain("lfg", "Dungeon Finder", (
        Method("lfg.getProposal", "Active Dungeon Finder proposal."),
        Method("lfg.getDungeon", "One row of LfgDungeons.dbc.", (
            Param("id", ParamType.INTEGER, "LfgDungeons.dbc ID", required=True),
        )),
        Method("lfg.acceptProposal", "Enter the dungeon."),
        Method("lfg.rejectProposal", "Leave the queue."),
    )),
    Domain("trade", "Trade", (
        Method("trade.get", "Pending request and both sides of the trade window."),
        Method("trade.acceptRequest", "Accept the pending trade request."),
        Method("trade.declineRequest", "Decline the pending trade request."),
        Method("trade.accept", "Press Trade in the trade window."),
        Method("trade.unaccept", "Withdraw your acceptance."),
        Method("trade.cancel", "Close the trade window without trading."),
    )),
    Domain("duel", "Duels", (
        Method("duel.get", "Current duel request or duel."),
        Method("duel.accept", "Accept the duel."),
        Method("duel.decline", "Decline the request or forfeit the duel."),
    )),
    Domain("battlefield", "Battlegrounds", (
        Method("battlefield.getQueues", "Both battleground and arena queues."),
        Method("battlefield.acceptPort", "Enter the battleground or arena.", (_index(),)),
        Method("battlefield.leaveQueue", "Leave a queue or decline its invitation.", (_index(),)),
    )),
    Domain("chat", "Chat", (
        Method("chat.send", "Send a message on a chat channel.", (
            Param("channel", ParamType.CHOICE, "Chat channel", required=True, default="WHISPER",
                  choices=("WHISPER", "SAY", "YELL", "EMOTE", "PARTY", "RAID", "RAID_WARNING", "BATTLEGROUND",
                           "GUILD", "OFFICER", "CHANNEL", "BN_WHISPER")),
            Param("target", ParamType.STRING,
                  "Character name for WHISPER, channel number or name for CHANNEL, presence ID for BN_WHISPER"),
            Param("text", ParamType.STRING, "Message, up to 255 bytes", required=True),
            Param("language", ParamType.INTEGER, "Languages.dbc ID; empty uses the character's default language"),
        )),
        Method("chat.reply", "Reply to the last whisper, or to a specific one.", (
            Param("text", ParamType.STRING, "Message", required=True),
            Param("lineId", ParamType.INTEGER, "Reply to the whisper with this line ID"),
            Param("guid", ParamType.STRING, "Reply to the last whisper from this sender GUID"),
        )),
        Method("chat.getRecentWhispers", "Whispers chat.reply can answer."),
    )),
    Domain("events", "Events", (
        Method("events.subscribe", "Subscribe to WoW UI events.", (
            Param("events", ParamType.STRING_LIST, "Event names separated by commas", required=True,
                  default="PLAYER_LOGIN, PLAYER_ENTERING_WORLD, PLAYER_REGEN_DISABLED, UNIT_HEALTH"),
        )),
        Method("events.unsubscribe", "Unsubscribe from WoW UI events.", (
            Param("events", ParamType.STRING_LIST, "Event names separated by commas", required=True),
        )),
        Method("events.list", "Subscribed WoW UI events."),
        Method("events.clear", "Remove every subscription."),
        Method("events.getHistory", "Read the event history buffer.", (
            Param("offset", ParamType.INTEGER, "Position to start from", default=0),
            Param("count", ParamType.INTEGER, "Number of entries", default=50),
        )),
        Method("events.clearHistory", "Clear the event history buffer."),
        Method("events.setMaxBuffer", "Change the history buffer size.", (
            Param("maxBuffer", ParamType.INTEGER, "Maximum number of entries", default=256),
        )),
        Method("events.isReady", "Whether the event hooks are ready."),
    )),
    Domain("lua", "Lua", (
        Method("lua.execute", "Run Lua code without a return value.", (
            Param("code", ParamType.TEXT, "Lua source code", required=True),
            Param("chunkName", ParamType.STRING, "Name shown in errors"),
            Param("secure", ParamType.BOOLEAN, "Run without taint (dangerous)", default=False),
        )),
        Method("lua.evaluate", "Run Lua code and return every value.", (
            Param("code", ParamType.TEXT, "Lua expression or code", required=True, default="return GetRealZoneText()"),
            Param("chunkName", ParamType.STRING, "Name shown in errors"),
            Param("maxDepth", ParamType.INTEGER, "Table depth to convert", default=4),
            Param("maxEntries", ParamType.INTEGER, "Values to convert", default=500),
            Param("secure", ParamType.BOOLEAN, "Run without taint (dangerous)", default=False),
        )),
        Method("lua.call", "Call a Lua function by path or ref with JSON arguments.", (
            Param("function", ParamType.STRING, "Path such as GetItemInfo or ChatFrame1:GetName", default="GetItemInfo"),
            Param("args", ParamType.JSON, "Arguments as a JSON array", default="[6948]"),
            Param("ref", ParamType.INTEGER, "Ref to call instead of a path"),
            Param("method", ParamType.STRING, "Method to call on the ref"),
            Param("maxDepth", ParamType.INTEGER, "Table depth to convert", default=4),
            Param("maxEntries", ParamType.INTEGER, "Values to convert", default=500),
            Param("secure", ParamType.BOOLEAN, "Run without taint (dangerous)", default=False),
        )),
        Method("lua.batch", "Run several Lua operations in one game frame.", (
            Param("operations", ParamType.JSON, "Array of {method, params}", required=True,
                  default='[{"method": "lua.call", "params": {"function": "UnitName", "args": ["player"]}}, '
                          '{"method": "lua.getGlobal", "params": {"name": "GetCVar"}}]'),
            Param("stopOnError", ParamType.BOOLEAN, "Stop at the first failed operation", default=False),
        )),
        Method("lua.getGlobal", "Read a value by path, tables included.", (
            Param("name", ParamType.STRING, "Path such as UnitName or MyAddon.config", required=True),
            Param("maxDepth", ParamType.INTEGER, "Table depth to convert", default=4),
            Param("maxEntries", ParamType.INTEGER, "Values to convert", default=500),
        )),
        Method("lua.setGlobal", "Set a value by path, tables included.", (
            Param("name", ParamType.STRING, "Path such as MyAddon.config.volume", required=True),
            Param("value", ParamType.JSON, "Value as JSON, for example 42, true, \"text\" or {\"a\": 1}"),
        )),
        Method("lua.createNamespace", "Create an empty table at a path.", (
            Param("name", ParamType.STRING, "Table path", required=True),
        )),
        Method("lua.getRef", "Keep a reference to a Lua value.", (
            Param("path", ParamType.STRING, "Path such as PlayerFrame", required=True),
        )),
        Method("lua.releaseRef", "Release a reference.", (
            Param("ref", ParamType.INTEGER, "Ref id", required=True),
        )),
        Method("lua.hook", "Stream calls to a Lua function as lua.hooked events.", (
            Param("function", ParamType.STRING, "Path such as AcceptQuest or StaticPopup1Button1:Click", required=True),
        )),
        Method("lua.unhook", "Stop streaming calls to a hooked function.", (
            Param("function", ParamType.STRING, "Hooked path", required=True),
        )),
        Method("lua.getHooks", "List active hooks."),
        Method("lua.setAllowedAddons", "Limit SDK access to these addons, or null for every addon that opts in.", (
            Param("addons", ParamType.JSON, "JSON array of addon names or null", required=True, default="null"),
        )),
        Method("lua.getAddons", "Addons that received the SDK and the allow list."),
        Method("lua.setBlockedNotice", "Show or hide the game notice when the SDK is blocked.", (
            Param("show", ParamType.BOOLEAN, "Show the game notice", default=False),
        )),
    )),
    Domain("window", "Window", (
        Method("window.getInfo", "Window handle, state, bounds and title."),
        Method("window.getHandle", "Window handle."),
        Method("window.show", "Show the window."),
        Method("window.hide", "Hide the window."),
        Method("window.setStartHidden", "Keep the game window hidden until window.show.", (
            Param("hidden", ParamType.BOOLEAN, "Keep the window hidden", required=True, default=True),
        )),
        Method("window.minimize", "Minimize the window."),
        Method("window.restore", "Restore the window."),
        Method("window.maximize", "Maximize the window."),
        Method("window.focus", "Bring the window to the foreground."),
        Method("window.getPosition", "Window position."),
        Method("window.setPosition", "Move the window.", (
            Param("x", ParamType.INTEGER, "X coordinate", required=True, default=0),
            Param("y", ParamType.INTEGER, "Y coordinate", required=True, default=0),
        )),
        Method("window.getSize", "Window size."),
        Method("window.setSize", "Resize the window.", (
            Param("width", ParamType.INTEGER, "Width in pixels", required=True, default=1280),
            Param("height", ParamType.INTEGER, "Height in pixels", required=True, default=720),
        )),
        Method("window.setBounds", "Move and resize the window.", (
            Param("x", ParamType.INTEGER, "X coordinate", required=True, default=0),
            Param("y", ParamType.INTEGER, "Y coordinate", required=True, default=0),
            Param("width", ParamType.INTEGER, "Width in pixels", required=True, default=1280),
            Param("height", ParamType.INTEGER, "Height in pixels", required=True, default=720),
        )),
        Method("window.setTitle", "Change the window title.", (
            Param("title", ParamType.STRING, "New title", required=True),
        )),
        Method("window.setTopMost", "Keep the window above the others.", (
            Param("topMost", ParamType.BOOLEAN, "Always on top", required=True, default=True),
        )),
        Method("window.setOpacity", "Change the window opacity.", (
            Param("percent", ParamType.INTEGER, "Opacity from 0 to 100", default=100),
        )),
        Method("window.flashTaskbar", "Flash the taskbar button.", (
            Param("count", ParamType.INTEGER, "Number of flashes, 0 until focused", default=0),
            Param("timeoutMs", ParamType.INTEGER, "Interval in milliseconds, 0 for the default", default=0),
        )),
        Method("window.setIcon", "Change the window icon or restore the original one.", (
            Param("path", ParamType.STRING, "Absolute path to an .ico file"),
            Param("type", ParamType.CHOICE, "Icon size to change, empty for both", default="", choices=("", "small", "big")),
            Param("restore", ParamType.BOOLEAN, "Restore the original icon instead", default=False),
        )),
    )),
    Domain("dll", "DLL", (
        Method("dll.getInfo", "DLL version, build and state."),
        Method("dll.toggleConsole", "Show or hide the in-game console."),
        Method("dll.getLogs", "Captured game logs.", (
            Param("offset", ParamType.INTEGER, "Position to start from", default=0),
            Param("count", ParamType.INTEGER, "Number of entries", default=50),
        )),
        Method("dll.setLogConfig", "Configure log capture.", (
            Param("maxEntries", ParamType.INTEGER, "Maximum buffered entries", default=256),
            Param("showInConsole", ParamType.BOOLEAN, "Echo logs to the in-game console", default=False),
        )),
    )),
    Domain("auth", "Authentication", (
        Method("auth.status", "Authentication state of the SDK."),
    )),
)


def all_methods() -> list[Method]:
    """Every method of every domain, in sidebar order."""
    return [method for domain in DOMAINS for method in domain.methods]


def find_method(name: str) -> Method | None:
    """The method with this exact name, if it exists."""
    return next((method for method in all_methods() if method.name == name), None)
