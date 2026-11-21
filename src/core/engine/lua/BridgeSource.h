#pragma once

namespace LuaBridge {

inline constexpr const char* SOURCE = R"lua(
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

)lua" R"lua(
local exposed = {}

local function expose(owner, name, handler)
    if type(name) ~= "string" or #name > 64 or not string.match(name, "^[%a_][%w_]*$")
        or type(handler) ~= "function" then
        error("Usage: SDK.Expose(name, function) with a Lua identifier of up to 64 characters", 3)
    end
    exposed[owner] = exposed[owner] or {}
    exposed[owner][name] = handler
    return true
end

function bridge.callExposed(owner, name, ...)
    local functions = exposed[owner]
    local handler = functions and functions[name]
    if type(handler) ~= "function" then
        error(tostring(owner) .. " does not expose " .. tostring(name), 0)
    end
    return handler(...)
end

local storage = {}
local storageDirty = {}
local storageListeners = {}
local storageKeyLimit = 128
local storageMaxKeys = 1000
local storageMaxDepth = 12
local storageMaxEntries = 2000

local function plainValue(value, depth, seen, counter)
    local kind = type(value)
    if kind == "string" or kind == "boolean" then
        return true
    end
    if kind == "number" then
        if value ~= value or value == math.huge or value == -math.huge then
            return false, "numbers must be finite"
        end
        return true
    end
    if kind ~= "table" then
        return false, "only strings, numbers, booleans and tables can be saved"
    end
    if depth > storageMaxDepth then
        return false, "tables are nested too deeply"
    end
    if seen[value] then
        return false, "a table cannot contain itself"
    end
    seen[value] = true
    for key, item in pairs(value) do
        counter.entries = counter.entries + 1
        if counter.entries > storageMaxEntries then
            return false, "the value has more than " .. storageMaxEntries .. " entries"
        end
        local keyKind = type(key)
        if keyKind ~= "string" and keyKind ~= "number" then
            return false, "table keys must be strings or numbers"
        end
        local ok, problem = plainValue(item, depth + 1, seen, counter)
        if not ok then
            return false, problem
        end
    end
    seen[value] = nil
    return true
end

local function copyValue(value)
    if type(value) ~= "table" then
        return value
    end
    local copy = {}
    for key, item in pairs(value) do
        copy[key] = copyValue(item)
    end
    return copy
end

local function storageOf(owner)
    local data = storage[owner]
    if not data then
        data = {}
        storage[owner] = data
    end
    return data
end

local function checkKey(key, usage)
    if type(key) ~= "string" or key == "" or #key > storageKeyLimit then
        error(usage .. " with a key of 1 to " .. storageKeyLimit .. " characters", 3)
    end
end

local function storageSave(owner, key, value)
    checkKey(key, "Usage: SDK.Save(key, value)")
    local data = storageOf(owner)
    if value == nil then
        if data[key] ~= nil then
            data[key] = nil
            storageDirty[owner] = true
        end
        return true
    end
    local ok, problem = plainValue(value, 0, {}, { entries = 0 })
    if not ok then
        error("SDK.Save: " .. problem, 3)
    end
    if data[key] == nil then
        local count = 0
        for _ in pairs(data) do
            count = count + 1
        end
        if count >= storageMaxKeys then
            error("SDK.Save: an addon can keep at most " .. storageMaxKeys .. " keys", 3)
        end
    end
    data[key] = copyValue(value)
    storageDirty[owner] = true
    return true
end

local function storageLoad(owner, key, default)
    checkKey(key, "Usage: SDK.Load(key[, default])")
    local value = storageOf(owner)[key]
    if value == nil then
        return default
    end
    return copyValue(value)
end

local function storageKeys(owner)
    local keys = {}
    for key in pairs(storageOf(owner)) do
        keys[#keys + 1] = key
    end
    table.sort(keys)
    return keys
end

local function storageListen(owner, callback)
    if type(callback) ~= "function" then
        error("Usage: SDK.OnDataChanged(function(key, value))", 3)
    end
    storageListeners[owner] = storageListeners[owner] or {}
    table.insert(storageListeners[owner], callback)
    return true
end

function bridge.storageInit(owner, data)
    if type(data) == "table" and storage[owner] == nil then
        storage[owner] = data
    end
end

function bridge.storageTakeDirty()
    local owners = {}
    for owner in pairs(storageDirty) do
        owners[#owners + 1] = owner
    end
    storageDirty = {}
    return owners
end

function bridge.storageOf(owner)
    return storage[owner] or {}
end

function bridge.storageSet(owner, key, value)
    if type(key) ~= "string" or key == "" or #key > storageKeyLimit then
        error("key must be a string of 1 to " .. storageKeyLimit .. " characters", 0)
    end
    if value ~= nil then
        local ok, problem = plainValue(value, 0, {}, { entries = 0 })
        if not ok then
            error(problem, 0)
        end
    end
    storageOf(owner)[key] = copyValue(value)
    for _, callback in ipairs(storageListeners[owner] or {}) do
        pcall(callback, key, copyValue(value))
    end
    return true
end

local function addStorageMethods(methods, owner, guard)
    function methods.Save(key, value)
        guard()
        return storageSave(owner, key, value)
    end

    function methods.Load(key, default)
        guard()
        return storageLoad(owner, key, default)
    end

    function methods.Delete(key)
        guard()
        return storageSave(owner, key, nil)
    end

    function methods.Keys()
        guard()
        return storageKeys(owner)
    end

    function methods.OnDataChanged(callback)
        guard()
        return storageListen(owner, callback)
    end
end

)lua" R"lua(
function bridge.errorHandler(message)
    local stack
    if debugstack then
        stack = debugstack(2)
    end
    return { message = tostring(message), stack = stack }
end

function bridge.get(path)
    return resolve(path)
end

function bridge.set(path, value)
    local owner, key = parent(path)
    owner[key] = value
end

function bridge.call(path, ...)
    local objectPath, name = method(path)
    if objectPath then
        local object = resolve(objectPath)
        if object == nil then
            error(objectPath .. " is nil", 0)
        end
        local fn = object[name]
        if type(fn) ~= "function" then
            error(path .. " is not a function", 0)
        end
        return fn(object, ...)
    end
    local fn = resolve(path)
    if type(fn) ~= "function" then
        error(path .. " is not a function", 0)
    end
    return fn(...)
end

function bridge.callRef(id, name, ...)
    local object = bridge.refs[id]
    if object == nil then
        error("ref " .. tostring(id) .. " does not exist", 0)
    end
    if name then
        local fn = object[name]
        if type(fn) ~= "function" then
            error("ref " .. tostring(id) .. " has no method " .. name, 0)
        end
        return fn(object, ...)
    end
    if type(object) ~= "function" then
        error("ref " .. tostring(id) .. " is not a function", 0)
    end
    return object(...)
end

function bridge.ref(path)
    local value = resolve(path)
    if value == nil then
        error(path .. " is nil", 0)
    end
    if bridge.refCount >= refLimit then
        error("too many refs, release unused ones with lua.releaseRef", 0)
    end
    bridge.nextRef = bridge.nextRef + 1
    bridge.refs[bridge.nextRef] = value
    bridge.refCount = bridge.refCount + 1
    return bridge.nextRef, type(value)
end

function bridge.unref(id)
    if bridge.refs[id] == nil then
        return false
    end
    bridge.refs[id] = nil
    bridge.refCount = bridge.refCount - 1
    return true
end

function bridge.hook(path)
    if bridge.hooks[path] == nil then
        if type(hooksecurefunc) ~= "function" then
            error("hooksecurefunc is not available", 0)
        end
        local owner, key = owned(path)
        if type(owner) ~= "table" then
            error(path .. " has no owner table", 0)
        end
        if type(owner[key]) ~= "function" then
            error(path .. " is not a function", 0)
        end
        hooksecurefunc(owner, key, function(...)
            if bridge.hooks[path] then
                push("hook", path, nil, ...)
            end
        end)
    end
    bridge.hooks[path] = true
    return true
end

function bridge.unhook(path)
    if not bridge.hooks[path] then
        return false
    end
    bridge.hooks[path] = false
    return true
end

function bridge.hookList()
    local list = {}
    for path, active in pairs(bridge.hooks) do
        if active then
            list[#list + 1] = path
        end
    end
    table.sort(list)
    return list
end

function bridge.grant(namespace, addon, data)
    if type(namespace) ~= "table" or type(addon) ~= "string" or string.sub(addon, 1, 1) == "@" then
        return false
    end
    local flag = GetAddOnMetadata and GetAddOnMetadata(addon, "X-WoWControlSDK")
    if not flag or flag == "" or flag == "0" then
        return false
    end
    bridge.storageInit(addon, data)

    local methods = { addon = addon }

    function methods.Emit(name, ...)
        if type(name) ~= "string" or name == "" or #name > 64 then
            error("Usage: SDK.Emit(name, ...) with a name of 1 to 64 characters", 2)
        end
        push("emit", name, addon, ...)
    end

    function methods.Print(text, r, g, b)
        return chatPrint(text, r, g, b)
    end

    function methods.RegisterCommand(names, handler)
        return registerCommand(addon, nil, names, handler)
    end

    function methods.UnregisterCommand(name)
        return unregisterCommand(addon, name)
    end

    function methods.Expose(name, handler)
        return expose(addon, name, handler)
    end

    addStorageMethods(methods, addon, function() end)

    rawset(namespace, "SDK", readOnly(methods))
    return true
end

bridge.scripts = {}

local frameScripts = {
    "OnEvent", "OnUpdate", "OnShow", "OnHide", "OnClick", "OnDoubleClick", "OnEnter", "OnLeave",
    "OnMouseDown", "OnMouseUp", "OnMouseWheel", "OnDragStart", "OnDragStop", "OnReceiveDrag",
    "OnKeyDown", "OnKeyUp", "OnChar", "OnTextChanged", "OnEnterPressed", "OnEscapePressed",
    "OnEditFocusGained", "OnEditFocusLost", "OnValueChanged", "OnSizeChanged", "OnAttributeChanged",
}
local stripDepth = 8

local function scriptObject(name, record)
    local methods = { addon = name, script = true }

    function methods.Emit(eventName, ...)
        if not record.active then
            error("the script " .. name .. " was unloaded", 2)
        end
        if type(eventName) ~= "string" or eventName == "" or #eventName > 64 then
            error("Usage: SDK.Emit(name, ...) with a name of 1 to 64 characters", 2)
        end
        push("emit", eventName, name, ...)
    end

    function methods.OnUnload(callback)
        if type(callback) ~= "function" then
            error("Usage: SDK.OnUnload(function)", 2)
        end
        record.unload[#record.unload + 1] = callback
    end

    function methods.HookScript(frame, handler, callback)
        if type(frame) ~= "table" or type(handler) ~= "string" or type(callback) ~= "function" then
            error("Usage: SDK.HookScript(frame, handler, function)", 2)
        end
        frame:HookScript(handler, function(...)
            if record.active then
                return callback(...)
            end
        end)
        record.hooks = record.hooks + 1
    end

    function methods.IsActive()
        return record.active
    end

    function methods.Print(text, r, g, b)
        if not record.active then
            error("the script " .. name .. " was unloaded", 2)
        end
        return chatPrint(text, r, g, b)
    end

    function methods.RegisterCommand(names, handler)
        if not record.active then
            error("the script " .. name .. " was unloaded", 2)
        end
        return registerCommand(name, record, names, handler)
    end

    function methods.UnregisterCommand(command)
        return unregisterCommand(name, command)
    end

    function methods.Expose(functionName, handler)
        if not record.active then
            error("the script " .. name .. " was unloaded", 2)
        end
        return expose(name, functionName, handler)
    end

    return readOnly(methods)
end

local function scriptEnvironment(record)
    local env = {}

    env.SlashCmdList = setmetatable({}, {
        __index = SlashCmdList,
        __newindex = function(_, key, value)
            record.slash[key] = true
            SlashCmdList[key] = value
        end,
    })

    env.CreateFrame = function(...)
        local frame = CreateFrame(...)
        record.frames[#record.frames + 1] = frame
        return frame
    end

    env.hooksecurefunc = function(target, method, callback)
        if type(target) ~= "table" then
            target, method, callback = nil, target, method
        end
        if type(callback) ~= "function" then
            error("Usage: hooksecurefunc([table,] name, function)", 2)
        end
        local guarded = function(...)
            if record.active then
                return callback(...)
            end
        end
        if target then
            hooksecurefunc(target, method, guarded)
        else
            hooksecurefunc(method, guarded)
        end
        record.hooks = record.hooks + 1
    end

    rawset(env, "_G", env)
    return setmetatable(env, {
        __index = _G,
        __newindex = function(_, key, value)
            if record.globals[key] == nil then
                record.globals[key] = { value = rawget(_G, key) }
                record.globalCount = record.globalCount + 1
            end
            _G[key] = value
        end,
    })
end

local function strip(frame, depth)
    if type(frame) ~= "table" or type(frame.GetObjectType) ~= "function" then
        return
    end
    if frame.UnregisterAllEvents then
        pcall(frame.UnregisterAllEvents, frame)
    end
    if frame.HasScript and frame.SetScript then
        for _, handler in ipairs(frameScripts) do
            local ok, has = pcall(frame.HasScript, frame, handler)
            if ok and has then
                pcall(frame.SetScript, frame, handler, nil)
            end
        end
    end
    if frame.Hide then
        pcall(frame.Hide, frame)
    end
    if depth < stripDepth and frame.GetChildren then
        for _, child in ipairs({ frame:GetChildren() }) do
            strip(child, depth + 1)
        end
    end
end

function bridge.scriptStats(name)
    local record = bridge.scripts[name]
    if not record then
        return nil
    end
    local slashCommands = 0
    for _ in pairs(record.slash) do
        slashCommands = slashCommands + 1
    end
    for _ in pairs(record.commands) do
        slashCommands = slashCommands + 1
    end
    return {
        frames = #record.frames,
        hooks = record.hooks,
        globals = record.globalCount,
        slashCommands = slashCommands,
    }
end

function bridge.runScript(chunk, name)
    if type(chunk) ~= "function" or type(name) ~= "string" then
        error("invalid script", 0)
    end
    if bridge.scripts[name] then
        bridge.unloadScript(name)
    end
    local record = {
        name = name, active = true, frames = {}, hooks = 0, globals = {}, globalCount = 0,
        slash = {}, commands = {}, unload = {},
    }
    bridge.scripts[name] = record
    setfenv(chunk, scriptEnvironment(record))
    chunk(name, { SDK = scriptObject(name, record) })
    return bridge.scriptStats(name)
end

function bridge.unloadScript(name)
    local stats = bridge.scriptStats(name)
    local record = bridge.scripts[name]
    if not record then
        return nil
    end
    bridge.scripts[name] = nil
    exposed[name] = nil
    record.active = false

    for index = #record.unload, 1, -1 do
        pcall(record.unload[index])
    end

    for _, frame in ipairs(record.frames) do
        strip(frame, 0)
        local ok, frameName = pcall(frame.GetName, frame)
        if ok and frameName and rawget(_G, frameName) == frame then
            _G[frameName] = nil
        end
    end

    local removed = {}
    for key in pairs(record.slash) do
        local callback = SlashCmdList[key]
        if callback ~= nil then
            removed[callback] = true
        end
        SlashCmdList[key] = nil
    end
    if type(hash_SlashCmdList) == "table" then
        for command, callback in pairs(hash_SlashCmdList) do
            if removed[callback] then
                hash_SlashCmdList[command] = nil
            end
        end
    end
    for key in pairs(record.commands) do
        commands[key] = nil
        if type(hash_SlashCmdList) == "table" then
            hash_SlashCmdList[key] = nil
        end
    end

    for key, saved in pairs(record.globals) do
        _G[key] = saved.value
    end
    return stats
end

function bridge.isGameState()
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

if type(DefaultServerLogin) == "function" then
    push("internal", "GLUE_LOADED", nil)
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
