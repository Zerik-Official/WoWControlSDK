#pragma once

namespace LuaBridge {

inline constexpr const char* SOURCE_AUTH = R"lua(
local function flag(value)
    return value ~= nil and value ~= false and value ~= 0
end

local function shown(frame)
    return type(frame) == "table" and type(frame.IsShown) == "function" and frame:IsShown() == 1
end

function bridge.realmList()
    local realms = {}
    if type(GetNumRealms) ~= "function" or type(GetRealmInfo) ~= "function" then
        return realms
    end
    local categories = type(GetRealmCategories) == "function" and { GetRealmCategories() } or {}
    for category = 1, math.max(#categories, 1) do
        for index = 1, GetNumRealms(category) or 0 do
            local name, characters, invalid, down, current, pvp, rp, load, locked, major, minor, revision, build =
                GetRealmInfo(category, index)
            if name then
                realms[#realms + 1] = {
                    name = name,
                    category = categories[category],
                    characters = characters or 0,
                    pvp = flag(pvp),
                    rp = flag(rp),
                    population = load,
                    locked = flag(locked),
                    down = flag(down),
                    invalid = flag(invalid),
                    current = flag(current),
                    version = major and (major .. "." .. minor .. "." .. revision) or nil,
                    build = build,
                }
            end
        end
    end
    return realms
end

function bridge.authChallenge()
    if shown(TokenEnterDialog) then
        return "token"
    elseif shown(VirtualKeypadFrame) then
        return "pin"
    elseif shown(SecurityMatrixLoginFrame) then
        return "matrix"
    end
    return nil
end

function bridge.submitAuth(code)
    if type(code) ~= "string" or not string.match(code, "^%d+$") then
        error("code must contain only digits", 0)
    end
    if shown(TokenEnterDialog) then
        if #code < 4 or #code > 16 then
            error("the code must have 4 to 16 digits", 0)
        end
        TokenEntered(code)
        TokenEnterDialog:Hide()
        return "token"
    end
    if shown(VirtualKeypadFrame) then
        local maxLength = MAX_PIN_LENGTH or 10
        if #code < 4 or #code > maxLength then
            error("the PIN must have 4 to " .. maxLength .. " digits", 0)
        end
        local buttons = {}
        for index = 1, 10 do
            local button = _G["VirtualKeypadButton" .. index]
            local label = button and button:GetText()
            if label then
                buttons[label] = tostring(button:GetID())
            end
        end
        local presses = {}
        for digit in string.gmatch(code, "%d") do
            if not buttons[digit] then
                error("the keypad has no button for " .. digit, 0)
            end
            presses[#presses + 1] = buttons[digit]
        end
        PINEntered(unpack(presses))
        VirtualKeypadFrame.PIN = ""
        VirtualKeypadFrame:Hide()
        return "pin"
    end
    if shown(SecurityMatrixLoginFrame) then
        error("the security matrix cannot be answered with a code", 0)
    end
    error("no authentication code is requested", 0)
end

function bridge.cancelAuth()
    local kind = bridge.authChallenge()
    if not kind then
        error("no authentication code is requested", 0)
    end
    for _, frame in ipairs({ TokenEnterDialog, VirtualKeypadFrame, SecurityMatrixLoginFrame }) do
        if shown(frame) then
            frame:Hide()
        end
    end
    CancelLogin()
    return kind
end

local function watchGlueFrame(frame, onShow, onHide)
    if type(frame) ~= "table" or type(frame.HookScript) ~= "function" then
        return
    end
    frame:HookScript("OnShow", onShow)
    frame:HookScript("OnHide", onHide)
end

local function watchGlueFrames()
    watchGlueFrame(TokenEnterDialog, function()
        push("internal", "AUTH_CHALLENGE", nil, "token")
    end, function()
        push("internal", "AUTH_CHALLENGE_CLOSED", nil, "token")
    end)
    watchGlueFrame(VirtualKeypadFrame, function()
        push("internal", "AUTH_CHALLENGE", nil, "pin", 4, MAX_PIN_LENGTH or 10)
    end, function()
        push("internal", "AUTH_CHALLENGE_CLOSED", nil, "pin")
    end)
    watchGlueFrame(SecurityMatrixLoginFrame, function()
        push("internal", "AUTH_CHALLENGE", nil, "matrix")
    end, function()
        push("internal", "AUTH_CHALLENGE_CLOSED", nil, "matrix")
    end)
    watchGlueFrame(RealmList, function()
        push("internal", "REALM_LIST_OPENED", nil, bridge.realmList())
    end, function()
        push("internal", "REALM_LIST_CLOSED", nil)
    end)
end

if type(DefaultServerLogin) == "function" then
    push("internal", "GLUE_LOADED", nil)
    watchGlueFrames()
    if type(CreateFrame) == "function" then
        local glueWatcher = CreateFrame("Frame")
        glueWatcher:RegisterEvent("CHARACTER_LIST_UPDATE")
        glueWatcher:SetScript("OnEvent", function(_, event)
            push("internal", event, nil)
        end)
    end
end

if type(seterrorhandler) == "function" and type(geterrorhandler) == "function" then
    wrapErrorHandler(geterrorhandler())
    if type(hooksecurefunc) == "function" then
        hooksecurefunc("seterrorhandler", function(handler)
            if not wrapping then
                wrapErrorHandler(handler)
            end
        end)
    end
end

return bridge
)lua";

}
