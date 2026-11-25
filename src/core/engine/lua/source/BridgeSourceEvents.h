#pragma once

namespace LuaBridge {

inline constexpr const char* SOURCE_EVENTS = R"lua(function bridge.isGameState()
    return type(IsLoggedIn) == "function" and IsLoggedIn() and true or false
end

local errorWindow = 10
local errorMemory = 200
local errorSeen = {}
local errorCount = 0
local wrapping = false

local function reportError(message)
    local text = tostring(message)
    local now = GetTime and GetTime() or 0
    local last = errorSeen[text]
    if last and now - last < errorWindow then
        return
    end
    if not last then
        errorCount = errorCount + 1
        if errorCount > errorMemory then
            errorSeen = {}
            errorCount = 1
        end
    end
    errorSeen[text] = now
    local addon = string.match(text, "AddOns[\\/]([^\\/]+)[\\/]")
    local stack = debugstack and debugstack(3) or nil
    push("error", text, addon, stack)
end

local function wrapErrorHandler(handler)
    if type(handler) ~= "function" or handler == bridge.errorWrapper then
        return
    end
    local wrapper = function(message, ...)
        pcall(reportError, message)
        return handler(message, ...)
    end
    bridge.errorWrapper = wrapper
    wrapping = true
    seterrorhandler(wrapper)
    wrapping = false
end

local internalHooks = {
    "AcceptGroup", "DeclineGroup",
    "ConfirmSummon", "CancelSummon",
    "BeginTrade", "CancelTrade",
    "AcceptDuel", "CancelDuel",
}

if type(hooksecurefunc) == "function" then
    for _, name in ipairs(internalHooks) do
        if type(_G[name]) == "function" then
            hooksecurefunc(name, function()
                if name == "AcceptGroup" and type(GetLFGRoles) == "function" then
                    local _, tank, healer, damage = GetLFGRoles()
                    push("internal", name, nil, tank and true or false, healer and true or false, damage and true or false)
                else
                    push("internal", name, nil)
                end
            end)
        end
    end
end

bridge.events = {}
bridge.blocked = {}
bridge.hideBlocked = true

local blockedEvents = { ADDON_ACTION_FORBIDDEN = true, ADDON_ACTION_BLOCKED = true }
local eventFrame = type(CreateFrame) == "function" and CreateFrame("Frame") or nil

if eventFrame then
    for name in pairs(blockedEvents) do
        eventFrame:RegisterEvent(name)
    end
    eventFrame:SetScript("OnEvent", function(_, event, ...)
        if bridge.events[event] then
            push("event", event, nil, ...)
        end
        if blockedEvents[event] then
            local source, func = ...
            if source ~= nil and source == bridge.taint then
                bridge.blocked[#bridge.blocked + 1] = { event = event, func = func }
                if bridge.hideBlocked and event == "ADDON_ACTION_FORBIDDEN" and type(StaticPopup_Hide) == "function" then
                    StaticPopup_Hide("ADDON_ACTION_FORBIDDEN")
                end
            end
        end
    end)
end

function bridge.setEvents(names)
    if not eventFrame then
        return false
    end
    local wanted = {}
    for _, name in ipairs(names) do
        wanted[name] = true
    end
    for name in pairs(bridge.events) do
        if not wanted[name] and not blockedEvents[name] then
            pcall(eventFrame.UnregisterEvent, eventFrame, name)
        end
    end
    for name in pairs(wanted) do
        if not bridge.events[name] and not blockedEvents[name] then
            pcall(eventFrame.RegisterEvent, eventFrame, name)
        end
    end
    bridge.events = wanted
    return true
end

)lua";

}
