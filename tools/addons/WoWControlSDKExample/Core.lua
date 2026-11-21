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

-- SDK.Print writes to the chat with the chat frame's own AddMessage, so
-- addons that replace ChatFrame1.AddMessage to read or change messages do not
-- see it. Without the SDK the addon falls back to the default chat frame.
function ns.Print(text)
    if SDK then
        SDK.Print(PREFIX .. tostring(text))
    else
        DEFAULT_CHAT_FRAME:AddMessage(PREFIX .. tostring(text))
    end
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

-- The options live in the SDK storage instead of SavedVariables, which the
-- game can only keep in a global. SDK.Load returns a copy of the saved value,
-- or the default; SDK.Save writes it to disk at once.
local options = SDK.Load("options", {})
for key, value in pairs(DEFAULTS) do
    if options[key] == nil then
        options[key] = value
    end
end

function ns.Options()
    return options
end

function ns.SaveOptions()
    SDK.Save("options", options)
end

-- The launcher can change the options with lua.setStorage while the game runs.
SDK.OnDataChanged(function(key, value)
    if key == "options" and type(value) == "table" then
        options = value
        ns.Print("Options changed by the application")
    end
end)

local frame = CreateFrame("Frame")
frame:RegisterEvent("PLAYER_LOGIN")
frame:RegisterEvent("PLAYER_LEVEL_UP")

frame:SetScript("OnEvent", function(_, event, arg1)
    if event == "PLAYER_LOGIN" then
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
    local leaked = {}
    for _, name in ipairs({ "WoWControlSDKExample", "WoWControlSDKExampleFrame", "WoWControlSDKExampleDB",
                            "SLASH_WOWCONTROLSDKEXAMPLE1" }) do
        if rawget(_G, name) ~= nil then
            leaked[#leaked + 1] = name
        end
    end
    ns.Print("Globals of this addon: " .. (#leaked == 0 and "none" or table.concat(leaked, ", ")))
    ns.Print("Saved keys: " .. table.concat(SDK.Keys(), ", "))
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

-- SDK.RegisterCommand adds /wcs without SLASH_ globals or a SlashCmdList
-- entry, so other code cannot find the handler and call it directly. It only
-- runs when the chat or a macro sends the text; treat the message as untrusted.
SDK.RegisterCommand("/wcs", function(message)
    local command, rest = string.match(message or "", "^(%S*)%s*(.-)$")
    local handler = commands[string.lower(command or "")] or commands.help
    handler(rest or "")
end)
