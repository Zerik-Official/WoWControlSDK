local addonName, ns = ...

-- Every file of the addon receives the same private table, so ns.SDK is
-- available here too.
if not ns.SDK then
    return
end

-- Public functions the launcher calls with lua.call, for example
-- {"method": "lua.call", "params": {"function": "WoWControlSDKExample.Ping", "args": ["hi"]}}.
-- Treat every argument as untrusted data: check its type and size, and never
-- run it as code with loadstring or RunScript.
local Api = {}
WoWControlSDKExample = Api

local MAX_TEXT = 255

local function isText(value)
    return type(value) == "string" and value ~= "" and #value <= MAX_TEXT
end

local panel = CreateFrame("Frame", "WoWControlSDKExampleFrame", UIParent)
panel:SetSize(320, 60)
panel:SetPoint("TOP", 0, -120)
panel:SetBackdrop({
    bgFile = "Interface\\Tooltips\\UI-Tooltip-Background",
    edgeFile = "Interface\\Tooltips\\UI-Tooltip-Border",
    tile = true, tileSize = 16, edgeSize = 16,
    insets = { left = 4, right = 4, top = 4, bottom = 4 },
})
panel:SetBackdropColor(0, 0, 0, 0.8)
panel.text = panel:CreateFontString(nil, "OVERLAY", "GameFontHighlight")
panel.text:SetPoint("CENTER")
panel:Hide()

-- Returns values directly: lua.call sends every return value back as JSON.
function Api.Ping(text)
    return "pong", text
end

function Api.GetStatus()
    return {
        addon = addonName,
        version = GetAddOnMetadata(addonName, "Version"),
        player = UnitName("player"),
        level = UnitLevel("player"),
        zone = GetRealZoneText(),
        options = ns.Options(),
    }
end

-- Shows text in the example panel and confirms it with an event.
function Api.ShowMessage(text)
    if not isText(text) then
        return false, "text must be a string of 1 to 255 characters"
    end
    panel.text:SetText(text)
    panel:Show()
    ns.Emit("example.messageShown", text)
    return true
end

function Api.HideMessage()
    panel:Hide()
    return true
end

-- Only known options with the expected type are accepted.
function Api.SetOption(key, value)
    local options = ns.Options()
    if options[key] == nil then
        return false, "unknown option"
    end
    if type(value) ~= type(options[key]) then
        return false, "expected a " .. type(options[key])
    end
    if type(value) == "string" and not isText(value) then
        return false, "text must be a string of 1 to 255 characters"
    end
    options[key] = value
    ns.Emit("example.optionChanged", key, value)
    return true
end

-- Request and response: the launcher sends an id, the addon answers with an
-- example.response event carrying the same id. Useful for work that finishes
-- later, such as waiting for a game event.
local actions = {}

function actions.status()
    return Api.GetStatus()
end

function actions.greeting()
    return ns.Options().greeting
end

function Api.Request(id, action)
    if type(id) ~= "number" or type(action) ~= "string" then
        return false, "usage: Request(id, action)"
    end
    local handler = actions[action]
    if not handler then
        ns.Emit("example.response", { id = id, ok = false, error = "unknown action" })
        return false
    end
    ns.Emit("example.response", { id = id, ok = true, data = handler() })
    return true
end
