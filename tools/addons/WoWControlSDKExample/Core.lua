local addonName, ns = ...

-- The SDK object only exists in this private table, and only when the
-- .toc declares "## X-WoWControlSDK: 1". Keep it in a local: it is never
-- placed in _G, so code injected later cannot reach it.
local SDK = ns.SDK

local PREFIX = "|cff33ccff[WoWControl]|r "
local DEFAULTS = {
    forwardLevelUp = true,
    greeting = "Hello from WoWControlSDK",
}

function ns.Print(text)
    DEFAULT_CHAT_FRAME:AddMessage(PREFIX .. tostring(text))
end

if not SDK then
    ns.Print("SDK not available. Inject WoWControlSDK and keep X-WoWControlSDK in the .toc.")
    return
end

-- Emit sends a named event to the launcher as lua.custom. Arguments may be
-- strings, numbers, booleans, nil and tables; functions become a marker.
-- Emit raises an error for invalid names, so wrap it when the name is dynamic.
function ns.Emit(name, ...)
    local ok, err = pcall(SDK.Emit, name, ...)
    if not ok then
        ns.Print("Emit failed: " .. tostring(err))
    end
    return ok
end

function ns.Options()
    return WoWControlSDKExampleDB.options
end

local frame = CreateFrame("Frame")
frame:RegisterEvent("ADDON_LOADED")
frame:RegisterEvent("PLAYER_LOGIN")
frame:RegisterEvent("PLAYER_LEVEL_UP")

frame:SetScript("OnEvent", function(_, event, arg1)
    if event == "ADDON_LOADED" and arg1 == addonName then
        WoWControlSDKExampleDB = WoWControlSDKExampleDB or {}
        WoWControlSDKExampleDB.options = WoWControlSDKExampleDB.options or {}
        for key, value in pairs(DEFAULTS) do
            if WoWControlSDKExampleDB.options[key] == nil then
                WoWControlSDKExampleDB.options[key] = value
            end
        end
    elseif event == "PLAYER_LOGIN" then
        local _, class = UnitClass("player")
        ns.Emit("example.ready", {
            addon = SDK.addon,
            version = GetAddOnMetadata(addonName, "Version"),
            player = UnitName("player"),
            realm = GetRealmName(),
            level = UnitLevel("player"),
            class = class,
        })
        ns.Print("Connected as " .. SDK.addon .. ". Type /wcs for commands.")
    elseif event == "PLAYER_LEVEL_UP" and ns.Options().forwardLevelUp then
        ns.Emit("example.levelUp", { level = arg1 })
    end
end)

local commands = {}

function commands.status()
    ns.Print("SDK addon: " .. SDK.addon)
    ns.Print("Global SDK: " .. tostring(rawget(_G, "SDK")))
    local writable = pcall(function() SDK.Emit = nil end)
    ns.Print("SDK is read-only: " .. tostring(not writable))
end

function commands.ping(text)
    ns.Emit("example.ping", text ~= "" and text or "ping", time())
    ns.Print("Sent example.ping")
end

function commands.types()
    ns.Emit("example.types", "text", 42, 3.5, true, false, nil, { 1, 2, 3 }, {
        nested = { deep = { value = 1 } },
        list = { "a", "b" },
    }, print)
    ns.Print("Sent example.types with every supported value type")
end

function commands.flood()
    for i = 1, 300 do
        SDK.Emit("example.flood", i)
    end
    ns.Print("Sent 300 events; the SDK keeps 256 per frame and reports the rest as lua.dropped")
end

function commands.help()
    ns.Print("/wcs status - check the SDK object")
    ns.Print("/wcs ping [text] - send example.ping")
    ns.Print("/wcs types - send every value type")
    ns.Print("/wcs flood - test the per-frame event limit")
end

SLASH_WOWCONTROLSDKEXAMPLE1 = "/wcs"
SlashCmdList.WOWCONTROLSDKEXAMPLE = function(message)
    local command, rest = string.match(message or "", "^(%S*)%s*(.-)$")
    local handler = commands[string.lower(command or "")] or commands.help
    handler(rest or "")
end
