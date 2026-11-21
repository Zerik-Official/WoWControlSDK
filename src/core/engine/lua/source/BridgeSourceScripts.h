#pragma once

namespace LuaBridge {

inline constexpr const char* SOURCE_SCRIPTS = R"lua(bridge.scripts = {}

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

)lua";

}
