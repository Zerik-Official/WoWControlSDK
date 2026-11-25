#pragma once

namespace LuaBridge {

inline constexpr const char* SOURCE_CORE = R"lua(
local bridge = {}
local queueLimit = 256
local eventQueueLimit = 4096
local refLimit = 256

bridge.queue = {}
bridge.refs = {}
bridge.refCount = 0
bridge.nextRef = 0
bridge.hooks = {}

local function push(kind, name, addon, ...)
    local queue = bridge.queue
    if #queue >= (kind == "event" and eventQueueLimit or queueLimit) then
        bridge.dropped = (bridge.dropped or 0) + 1
        return
    end
    queue[#queue + 1] = { kind = kind, name = name, addon = addon, n = select("#", ...), ... }
end

local function resolve(path)
    local value = _G
    local walked
    for segment in string.gmatch(path, "[^%.]+") do
        if type(value) ~= "table" then
            error((walked or "_G") .. " is not a table", 0)
        end
        value = value[tonumber(segment) or segment]
        walked = walked and (walked .. "." .. segment) or segment
    end
    return value
end

local function parent(path)
    local ownerPath, key = string.match(path, "^(.+)%.([^%.]+)$")
    if not ownerPath then
        return _G, path
    end
    local owner = resolve(ownerPath)
    if type(owner) ~= "table" then
        error(ownerPath .. " is not a table", 0)
    end
    return owner, tonumber(key) or key
end

local function method(path)
    return string.match(path, "^(.+):([%a_][%w_]*)$")
end

local function owned(path)
    local objectPath, name = method(path)
    if not objectPath then
        return parent(path)
    end
    local object = resolve(objectPath)
    if object == nil then
        error(objectPath .. " is nil", 0)
    end
    return object, name
end

local function readOnly(methods)
    return setmetatable({}, {
        __index = methods,
        __newindex = function()
            error("SDK is read-only", 2)
        end,
        __metatable = false,
    })
end

local printLimit = 4096
local commandLimit = 8
local rawAddMessage
do
    local ok, probe = pcall(CreateFrame, "ScrollingMessageFrame")
    local methods = ok and probe and getmetatable(probe)
    methods = type(methods) == "table" and methods.__index
    if type(methods) == "table" then
        rawAddMessage = methods.AddMessage
    end
    if ok and probe then
        probe:Hide()
    end
end

local function chatFrame()
    for _, name in ipairs({ "DEFAULT_CHAT_FRAME", "ChatFrame1" }) do
        local frame = rawget(_G, name)
        if type(frame) == "table" and type(frame.GetObjectType) == "function" then
            local ok, kind = pcall(frame.GetObjectType, frame)
            if ok and kind == "ScrollingMessageFrame" then
                return frame
            end
        end
    end
end

local function color(value)
    if type(value) ~= "number" then
        return 1
    end
    return math.max(0, math.min(1, value))
end

local function chatPrint(text, r, g, b)
    local frame = chatFrame()
    if not frame or not rawAddMessage then
        return false
    end
    text = tostring(text)
    if #text > printLimit then
        text = string.sub(text, 1, printLimit)
    end
    rawAddMessage(frame, text, color(r), color(g), color(b))
    return true
end

local commands = {}

local function commandDispatcher(key)
    return function(message, editBox)
        local entry = commands[key]
        if not entry or (entry.record and not entry.record.active) then
            return
        end
        if type(editBox) ~= "table" or type(editBox.GetText) ~= "function" then
            return
        end
        local ok, text = pcall(editBox.GetText, editBox)
        if not ok or type(text) ~= "string" then
            return
        end
        local typed = string.match(text, "^(/[^%s]+)")
        if not typed or string.upper(typed) ~= key then
            return
        end
        local rest = typed == text and "" or string.sub(text, #typed + 2)
        if strtrim(rest) ~= message then
            return
        end
        entry.handler(message)
    end
end

local function classicCommandExists(key)
    if type(SlashCmdList) ~= "table" then
        return false
    end
    for index in pairs(SlashCmdList) do
        local i = 1
        local alias = rawget(_G, "SLASH_" .. tostring(index) .. i)
        while type(alias) == "string" do
            if string.upper(alias) == key then
                return true
            end
            i = i + 1
            alias = rawget(_G, "SLASH_" .. tostring(index) .. i)
        end
    end
    return false
end

local function registerCommand(owner, record, names, handler)
    if type(names) == "string" then
        names = { names }
    end
    if type(names) ~= "table" or #names == 0 or #names > commandLimit or type(handler) ~= "function" then
        error("Usage: SDK.RegisterCommand(\"/name\" or { \"/name\", ... }, function(message))", 3)
    end
    if type(hash_SlashCmdList) ~= "table" then
        error("slash commands are not available here", 3)
    end
    local keys = {}
    for _, name in ipairs(names) do
        if type(name) ~= "string" or not string.match(name, "^/[%w_]+$") or #name > 32 then
            error("command names start with / and use letters, digits or _ (up to 32 characters)", 3)
        end
        local key = string.upper(name)
        local entry = commands[key]
        local secure = type(hash_SecureCmdList) == "table" and hash_SecureCmdList[key]
        if secure or (entry and entry.owner ~= owner)
            or (not entry and (rawget(hash_SlashCmdList, key) ~= nil or classicCommandExists(key))) then
            error(name .. " is already used by the game or another addon", 3)
        end
        keys[#keys + 1] = key
    end
    for _, key in ipairs(keys) do
        commands[key] = { owner = owner, handler = handler, record = record }
        hash_SlashCmdList[key] = commandDispatcher(key)
        if record then
            record.commands[key] = true
        end
    end
    return true
end

local function unregisterCommand(owner, name)
    if type(name) ~= "string" then
        error("Usage: SDK.UnregisterCommand(\"/name\")", 3)
    end
    local key = string.upper(name)
    local entry = commands[key]
    if not entry or entry.owner ~= owner then
        return false
    end
    commands[key] = nil
    if type(hash_SlashCmdList) == "table" then
        hash_SlashCmdList[key] = nil
    end
    if entry.record then
        entry.record.commands[key] = nil
    end
    return true
end

)lua";

}
